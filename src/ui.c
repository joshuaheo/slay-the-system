#define _XOPEN_SOURCE 700
#include <locale.h>
#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <wchar.h>
#include "battle.h"
#include "ui.h"
#include "card.h"
#include "enemy.h"
#include "relic.h"
#include "reward.h"
#include "shop.h"
#include "map.h"
#include "save.h"

extern volatile sig_atomic_t g_quit_requested;

#if 1
static void show_card_pile_screen(const char *title,const Card *cards,int count,int reverse_order);

static int get_display_hand_count(const Player *player, int max_display_hand);
static void normalize_selected_index(int *selected, int hand_count);
static void show_battle_message(int y, int x, const char *message);

static const char *get_enemy_intent_text(const Enemy *enemy);

static void normalize_target_index(int *target_index,Enemy enemies[],int enemy_count);

static void move_target_index(int *target_index,Enemy enemies[],int enemy_count,int direction);

static void draw_temp_battle_screen(GameState *state,Enemy enemies[],int enemy_count,int target_index,int selected,int max_display_hand,int battle_width,int battle_height,int card_slot_width);

static void show_battle_inventory_menu(Player *player);

static BattleResult handle_play_selected_card(GameState *state,Enemy enemies[],int enemy_count,int selected,int target_index,int message_y,int message_x);
static BattleResult handle_end_turn(GameState *state,Enemy enemies[],int enemy_count,int message_y,int message_x,int *turn_number);
static int has_active_shrink_effect_for_ui(Enemy enemies[], int enemy_count);

static void init_boss_enemies(Enemy enemies[], int *enemy_count);
static void init_elite_enemies(Enemy enemies[], int *enemy_count);
static void init_normal_enemies(int floor,Enemy enemies[], int *enemy_count);
static void init_start_normal_enemies(Enemy enemies[], int *enemy_count);
static void init_later_normal_enemies(Enemy enemies[], int *enemy_count);
static void init_battle_enemies(StageType stage,int floor, Enemy enemies[], int *enemy_count);

static int make_card_hand_display_name(const char *src, char *dest, int dest_size);
#endif
//ncurses 시작 함수
void init_ui(void) {
    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
}

//ncurses종료함수
void close_ui(void)
{
    clear();
    refresh();

    echo();
    nocbreak();
    keypad(stdscr, FALSE);
    curs_set(1);

    endwin();
}

//문자열이 터미널 화면에서 차지하는 실제 너비를 계산하는 함수
static int get_display_width(const char *text) {
    wchar_t wtext[256];
    int len;
    int width;

    len = mbstowcs(wtext, text, 255);
    if (len < 0) {
        return (int)strlen(text);
    }

    wtext[len] = L'\0';

    width = wcswidth(wtext, 255);
    if (width < 0) {
        return (int)strlen(text);
    }

    return width;
}

//문자열을 가운데 정렬해서 출력하는함수
static void print_centered(int row, const char *text) {
    int screen_width;
    int text_width;
    int col;

    screen_width = getmaxx(stdscr);
    text_width = get_display_width(text);
    col = (screen_width - text_width) / 2;

    if (col < 0) {
        col = 0;
    }

    mvprintw(row, col, "%s", text);
}

//로고 출력함수
static void print_logo(int start_row) {
    print_centered(start_row,     "  ____  _              _____ _          ____            _                 ");
    print_centered(start_row + 1, " / ___|| | __ _ _   _ |_   _| |__   ___/ ___| _   _ ___| |_ ___ _ __ ___ ");
    print_centered(start_row + 2, " \\___ \\| |/ _` | | | |  | | | '_ \\ / _ \\___ \\| | | / __| __/ _ \\ '_ ` _ \\");
    print_centered(start_row + 3, "  ___) | | (_| | |_| |  | | | | | |  __/___) | |_| \\__ \\ ||  __/ | | | | |");
    print_centered(start_row + 4, " |____/|_|\\__,_|\\__, |  |_| |_| |_|\\___|____/ \\__, |___/\\__\\___|_| |_| |_|");
    print_centered(start_row + 5, "                |___/                         |___/                       ");
}

//시작화면을 출력하고 사용자의 메뉴 선택결과를 반환하는 함수
MenuChoice show_start_screen(void) {
    int selected = 0;
    int ch;
    const char *menu_items[] = {
        "1. 게임시작",
        "2. 종료"
    };
    int menu_count = 2;
    int i;
    int logo_row;
    int menu_row;

    while (1) {
        clear();

        logo_row = 3;
        menu_row = 12;

        print_logo(logo_row);

        for (i = 0; i < menu_count; i++) {
            if (i == selected) {
                attron(A_REVERSE);
                print_centered(menu_row + i * 2, menu_items[i]);
                attroff(A_REVERSE);
            } else {
                print_centered(menu_row + i * 2, menu_items[i]);
            }
        }

        print_centered(menu_row + 6, "방향키와 엔터로 선택하세요.");
        refresh();

        ch = getch();

        if (ch == KEY_UP) {
            selected--;
            if (selected < 0) {
                selected = menu_count - 1;
            }
        } else if (ch == KEY_DOWN) {
            selected++;
            if (selected >= menu_count) {
                selected = 0;
            }
        } else if (ch == '\n' || ch == KEY_ENTER) {
            if (selected == 0) {
                return MENU_START_GAME;
            } else {
                return MENU_EXIT;
            }
        } else if (ch == '1') {
            return MENU_START_GAME;
        } else if (ch == '2') {
            return MENU_EXIT;
        }
    }
}

//로그인 화면 함수
int show_login_screen(char *username, int size) {
    int width;
    int input_col;
    int logo_row;
    int input_row;

    if (username == NULL || size <= 0) {
        return 0;
    }

    username[0] = '\0';

    clear();

    logo_row = 3;
    input_row = 12;

    print_logo(logo_row);

    print_centered(input_row, "로그인");
    print_centered(input_row + 2, "아이디를 입력하세요");

    width = getmaxx(stdscr);
    input_col = (width - 30) / 2;

    if (input_col < 0) {
        input_col = 0;
    }

    mvprintw(input_row + 4, input_col, "> ");
    refresh();

    echo();
    curs_set(1);

    move(input_row + 4, input_col + 2);
    getnstr(username, size - 1);

    noecho();
    curs_set(0);

    return 1;
}

//유효하지않은 아이디일경우 나오는 화면
void show_invalid_username_screen(void) {
    int logo_row;
    int message_row;

    clear();

    logo_row = 3;
    message_row = 12;

    print_logo(logo_row);

    print_centered(message_row, "잘못된 아이디입니다.");
    print_centered(message_row + 2, "아이디는 영문자, 숫자, _, - 만 사용할 수 있습니다.");
    print_centered(message_row + 4, "아무 키나 누르면 다시 입력합니다.");

    refresh();
    getch();
}

//저장 슬롯 화면 
int show_save_slot_screen(const char *username)
{
    int selected = 0;
    int ch;
    int i;
    int row;
    char line[256];
    char time_text[64];
    GameState temp_state;
    int floor;

    if (username == NULL) {
        return 0;
    }

    while (1) {
        clear();

        print_logo(3);
        print_centered(11, "세이브 슬롯 선택");

        snprintf(line, sizeof(line), "User: %s", username);
        print_centered(13, line);

        row = 16;

        for (i = 0; i < MAX_SAVE_SLOTS; i++) {
            int slot = i + 1;

            if (save_file_exists(username, slot)) {
                if (load_game(username, slot, &temp_state)) {
                    floor = temp_state.floor;
                } else {
                    floor = -1;
                }
                if (get_save_modified_time_string(username, slot, time_text, sizeof(time_text))) {
                    if (floor > 0) {
                        snprintf(line, sizeof(line),
                        "게임 %d  [저장됨]  %d층  마지막 저장: %s",
                        slot, floor, time_text);
                    } else {
                        snprintf(line, sizeof(line),
                        "게임 %d  [저장됨]  층 정보 없음  마지막 저장: %s",
                        slot, time_text);
                    }
                } else {
                    if (floor > 0) {
                        snprintf(line, sizeof(line),
                        "게임 %d  [저장됨]  %d층  마지막 저장: 알 수 없음",
                        slot, floor);
                    } else {
                        snprintf(line, sizeof(line),
                        "게임 %d  [저장됨]  층 정보 없음  마지막 저장: 알 수 없음",
                        slot);
                    }
                }
            } else {
                snprintf(line, sizeof(line), "게임 %d  [비어 있음]", slot);
            }

            if (selected == i) {
                attron(A_REVERSE);
                print_centered(row + i * 2, line);
                attroff(A_REVERSE);
            } else {
                print_centered(row + i * 2, line);
            }
        }

        if (selected == MAX_SAVE_SLOTS) {
            attron(A_REVERSE);
            print_centered(row + MAX_SAVE_SLOTS * 2, "뒤로가기");
            attroff(A_REVERSE);
        } else {
            print_centered(row + MAX_SAVE_SLOTS * 2, "뒤로가기");
        }

        print_centered(row + MAX_SAVE_SLOTS * 2 + 3, "방향키와 엔터로 선택하세요.");
        refresh();

        ch = getch();

        if (ch == KEY_UP) {
            selected--;
            if (selected < 0) {
                selected = MAX_SAVE_SLOTS;
            }
        } else if (ch == KEY_DOWN) {
            selected++;
            if (selected > MAX_SAVE_SLOTS) {
                selected = 0;
            }
        } else if (ch == '\n' || ch == KEY_ENTER) {
            if (selected == MAX_SAVE_SLOTS) {
                return 0;
            }
            return selected + 1;
        } else if (ch >= '1' && ch <= '3') {
            return ch - '0';
        } else if (ch == 'q' || ch == 'Q') {
            return 0;
        }
    }
}

//저장된 슬롯 선택 후 행동 화면
SaveSlotAction show_save_slot_action_screen(const char *username, int slot)
{
    int selected = 0;
    int ch;
    int i;
    int row;
    char line[256];
    char time_text[64];
    GameState temp_state;
    int floor;
    const char *items[] = {
        "이어하기",
        "새 게임으로 덮어쓰기",
        "뒤로가기"
    };

    if (username == NULL) {
        return SAVE_ACTION_BACK;
    }

    while (1) {
        clear();

        print_logo(3);

        snprintf(line, sizeof(line), "게임 %d", slot);
        print_centered(11, line);
        if (load_game(username, slot, &temp_state)) {
            floor = temp_state.floor;
        } else {
            floor = -1;
        }
        if (floor > 0) {
            snprintf(line, sizeof(line), "현재 층: %d층", floor);
        } else {
            snprintf(line, sizeof(line), "현재 층: 알 수 없음");
        }
        print_centered(13, line);

        if (get_save_modified_time_string(username, slot, time_text, sizeof(time_text))) {
            snprintf(line, sizeof(line), "마지막 저장: %s", time_text);
        } else {
            snprintf(line, sizeof(line), "마지막 저장: 알 수 없음");
        }
        print_centered(15, line);
        row = 18;

        for (i = 0; i < 3; i++) {
            if (selected == i) {
                attron(A_REVERSE);
                print_centered(row + i * 2, items[i]);
                attroff(A_REVERSE);
            } else {
                print_centered(row + i * 2, items[i]);
            }
        }

        print_centered(row + 8, "방향키와 엔터로 선택하세요.");
        refresh();

        ch = getch();

        if (ch == KEY_UP) {
            selected--;
            if (selected < 0) {
                selected = 2;
            }
        } else if (ch == KEY_DOWN) {
            selected++;
            if (selected > 2) {
                selected = 0;
            }
        } else if (ch == '\n' || ch == KEY_ENTER) {
            if (selected == 0) {
                return SAVE_ACTION_LOAD;
            }
            if (selected == 1) {
                return SAVE_ACTION_NEW;
            }
            return SAVE_ACTION_BACK;
        } else if (ch == '1') {
            return SAVE_ACTION_LOAD;
        } else if (ch == '2') {
            return SAVE_ACTION_NEW;
        } else if (ch == '3' || ch == 'q' || ch == 'Q') {
            return SAVE_ACTION_BACK;
        }
    }
}

//전투 부분 ui 함수

//전투화면의 큰 구분선을 출력하는 함수
static void print_battle_line(int y, int x, int width)
{
    mvhline(y, x, '=', width);
}

//전투화면의 작은 구분선을 출력하는함수
static void print_battle_dash(int y, int x, int width)
{
    mvhline(y, x, '-', width);
}

//전투 결과 메시지 출력함수
static void show_battle_result_message(BattleResult result)
{
    clear();

    if (result == BATTLE_WIN) {
        mvprintw(5, 5, "전투 승리!");
    } else if (result == BATTLE_LOSE) {
        mvprintw(5, 5, "전투 패배...");
    }

    if (result == BATTLE_WIN) {
        mvprintw(7, 5, "아무 키나 누르면 보상 화면으로 이동합니다.");
    } else if (result == BATTLE_LOSE) {
        mvprintw(7, 5, "아무 키나 누르면 세이브를 삭제하고 종료합니다.");
    }

    refresh();
    getch();
    clear();
    refresh();
}

//전투 보상 출력 메시지
static void show_card_added_message(const Card *card)
{
    clear();

    if (card != NULL) {
        mvprintw(5, 5, "%s 카드를 덱에 추가했습니다.", card->name);
    }

    mvprintw(7, 5, "아무 키나 누르면 계속합니다.");
    refresh();
    getch();
}

//유물 요약 출력 함수
static void print_relic_summary(const Player *player)
{
    if (player == NULL || player->relic_count <= 0) {
        printw("없음");
        return;
    }

    printw("%s", player->relics[0].name);

    if (player->relic_count > 1) {
        printw(" 외 %d개", player->relic_count - 1);
    }
}

//임시 전투화면 출력 함수
BattleResult show_temp_battle_screen(GameState *state, StageType stage)
{
    Player *player;
    BattleResult final_result = BATTLE_CONTINUE;
    BattleResult battle_result;

    Enemy enemies[MAX_ENEMIES];
    int enemy_count = 0;
    int target_index = 0;

    const int battle_width = 90;
    const int battle_height = 28;
    const int max_display_hand = 10;
    const int card_slot_width = 18;

    int hand_count;
    int selected = 0;
    int ch;
    int start_y;
    int start_x;
    int turn_number = 1;

    if (state == NULL) {
        return BATTLE_CONTINUE;
    }
    if (g_quit_requested) {
        return BATTLE_QUIT;
    }

    player = &state->player;
    player->exhausted_this_turn = 0;
    player->hp_lost_this_turn = 0;
    init_battle_enemies(stage, state->floor, enemies, &enemy_count);
    apply_relics_on_battle_start(player, enemies, enemy_count);
    apply_relics_on_turn_start(player, enemies, enemy_count, turn_number);

    final_result = check_battle_result(player, enemies, enemy_count);

if (final_result != BATTLE_CONTINUE) {
    show_battle_result_message(final_result);
    return final_result;
}

    while (1) {
        if (g_quit_requested) {        
            final_result = BATTLE_QUIT;
            break;
        }
        start_y = (LINES - battle_height) / 3;
        start_x = (COLS - battle_width) / 2;

        if (start_y < 0) {
            start_y = 0;
        }

        if (start_x < 0) {
            start_x = 0;
        }

        hand_count = get_display_hand_count(player, max_display_hand);
        normalize_selected_index(&selected, hand_count);

        draw_temp_battle_screen(state,enemies,enemy_count,target_index,selected,max_display_hand,battle_width,battle_height,card_slot_width);

        ch = getch();
        if (g_quit_requested) {
            final_result = BATTLE_QUIT;
            break;        
        }

        if (ch == KEY_LEFT || ch == 'a' || ch == 'A') {
            if (hand_count > 0) {
                selected--;

                if (selected < 0) {
                    selected = hand_count - 1;
                }
            }
        }
        else if (ch == KEY_RIGHT || ch == 'd' || ch == 'D') {
            if (hand_count > 0) {
                selected++;

                if (selected >= hand_count) {
                    selected = 0;
                }
            }
        }
        else if (ch == KEY_UP || ch == 'w' || ch == 'W') {
            move_target_index(&target_index, enemies, enemy_count, -1);
        }
        else if (ch == KEY_DOWN || ch == 's' || ch == 'S') {
            move_target_index(&target_index, enemies, enemy_count, 1);
        }
        else if (ch == '\n' || ch == KEY_ENTER) {
            normalize_target_index(&target_index, enemies, enemy_count);
            battle_result = handle_play_selected_card(state,enemies,enemy_count,selected,target_index,start_y + 27,start_x);

            if (battle_result != BATTLE_CONTINUE) {
                final_result = battle_result;
                show_battle_result_message(battle_result);
                break;
            }
            normalize_target_index(&target_index, enemies, enemy_count);

            hand_count = get_display_hand_count(player, max_display_hand);
            normalize_selected_index(&selected, hand_count);
        }
        else if (ch == 'i' || ch == 'I') {
            show_battle_inventory_menu(player);
        }
        else if (ch == 'e' || ch == 'E') {
            battle_result = handle_end_turn(state,enemies,enemy_count,start_y + 27,start_x,&turn_number);

            if (battle_result != BATTLE_CONTINUE) {
                final_result = battle_result;
                show_battle_result_message(battle_result);
                break;
            }
            normalize_target_index(&target_index, enemies, enemy_count);

            hand_count = get_display_hand_count(player, max_display_hand);
            normalize_selected_index(&selected, hand_count);
        }
        else if (ch == 'q' || ch == 'Q') {
            final_result = BATTLE_QUIT;
            break;
        }
    }

    return final_result;
}

//손에 든 카드를 확인하는 함수
static int get_display_hand_count(const Player *player, int max_display_hand)
{
    int hand_count;

    if (player == NULL) {
        return 0;
    }

    hand_count = player->hand_count;

    if (hand_count > max_display_hand) {
        hand_count = max_display_hand;
    }

    return hand_count;
}

//오류 방지 함수
static void normalize_selected_index(int *selected, int hand_count)
{
    if (selected == NULL) {
        return;
    }

    if (*selected < 0) {
        *selected = 0;
    }

    if (hand_count <= 0) {
        *selected = 0;
        return;
    }

    if (*selected >= hand_count) {
        *selected = hand_count - 1;
    }
}

//메시지 출력함수
static void show_battle_message(int y, int x, const char *message)
{
    if (message == NULL) {
        return;
    }

    mvprintw(y, x, "%s", message);
    refresh();
    getch();
}

//보스 설정 함수
static void init_boss_enemies(Enemy enemies[], int *enemy_count)
{
    if (enemies == NULL || enemy_count == NULL) {
        return;
    }

    *enemy_count = 1;
    init_enemy(&enemies[0], ENEMY_VANTOM);
}

//랜덤 엘리트 함수
static void init_elite_enemies(Enemy enemies[], int *enemy_count)
{
    int random_elite;

    if (enemies == NULL || enemy_count == NULL) {
        return;
    }

    *enemy_count = 1;

    random_elite = rand() % 3;

    if (random_elite == 0) {
        init_enemy(&enemies[0], ENEMY_BYGONE_EFFIGY);
    }
    else if (random_elite == 1) {
        init_enemy(&enemies[0], ENEMY_BYRDONIS);
    }
    else {
        init_enemy(&enemies[0], ENEMY_TERROR_EEL);
    }
}

//3층이전까지의 일반적 설정 함수
static void init_start_normal_enemies(Enemy enemies[], int *enemy_count)
{
    int encounter;

    if (enemies == NULL || enemy_count == NULL) {
        return;
    }

    encounter = rand() % 6;

    if (encounter == 0) {
        *enemy_count = 1;
        init_enemy(&enemies[0], ENEMY_JAW_WORM);
    }
    else if (encounter == 1) {
        *enemy_count = 1;
        init_enemy(&enemies[0], ENEMY_SHRINKER_BEETLE);
    }
    else if (encounter == 2) {
        *enemy_count = 1;
        init_enemy(&enemies[0], ENEMY_FUZZY_WURM_CRAWLER);
    }
    else if (encounter == 3) {
        *enemy_count = 1;
        init_enemy(&enemies[0], ENEMY_SLUDGE_SPINNER);
    }
    else if (encounter == 4) {
        *enemy_count = 1;
        init_enemy(&enemies[0], ENEMY_SEAPUNK);
    }
    else {
        *enemy_count = 2;
        init_enemy(&enemies[0], ENEMY_LEAF_SLIME);
        init_enemy(&enemies[1], ENEMY_TWIG_SLIME);
    }
}

//4층이후 일반적 설정 함수
static void init_later_normal_enemies(Enemy enemies[], int *enemy_count)
{
    int encounter;

    if (enemies == NULL || enemy_count == NULL) {
        return;
    }

    encounter = rand() % 14;

    if (encounter == 0) {
        *enemy_count = 1;
        init_enemy(&enemies[0], ENEMY_MAWLER);
    }
    else if (encounter == 1) {
        *enemy_count = 3;
        init_enemy(&enemies[0], ENEMY_INLET);
        init_enemy(&enemies[1], ENEMY_INLET);
        init_enemy(&enemies[2], ENEMY_INLET);
    }
    else if (encounter == 2) {
        *enemy_count = 2;
        init_enemy(&enemies[0], ENEMY_JAW_WORM);
        init_enemy(&enemies[1], ENEMY_JAW_WORM);
    }
    else if (encounter == 3) {
        *enemy_count = 3;
        init_enemy(&enemies[0], ENEMY_LEAF_SLIME);
        init_enemy(&enemies[1], ENEMY_LEAF_SLIME);
        init_enemy(&enemies[2], ENEMY_TWIG_SLIME);
    }
    else if (encounter == 4) {
        *enemy_count = 1;
        init_enemy(&enemies[0], ENEMY_CUBEX_CONSTRUCT);
    }
    else if (encounter == 5) {
        *enemy_count = 2;
        init_enemy(&enemies[0], ENEMY_SHRINKER_BEETLE);
        init_enemy(&enemies[1], ENEMY_FUZZY_WURM_CRAWLER);
    }
    else if (encounter == 6) {
        *enemy_count = 3;
        init_enemy(&enemies[0], ENEMY_INLET);
        init_enemy(&enemies[1], ENEMY_INLET);
        init_enemy(&enemies[2], ENEMY_SLUDGE_SPINNER);
    }
    else if (encounter == 7) {
        *enemy_count = 2;
        init_enemy(&enemies[0], ENEMY_JAW_WORM);
        init_enemy(&enemies[1], ENEMY_FUZZY_WURM_CRAWLER);
    }
    else if (encounter == 8) {
        *enemy_count = 2;
        init_enemy(&enemies[0], ENEMY_SHRINKER_BEETLE);
        init_enemy(&enemies[1], ENEMY_JAW_WORM);
    }
    else if (encounter == 9) {
        *enemy_count = 2;
        init_enemy(&enemies[0], ENEMY_FUZZY_WURM_CRAWLER);
        init_enemy(&enemies[1], ENEMY_FUZZY_WURM_CRAWLER);
    }
    else if (encounter == 10) {
        *enemy_count = 3;
        init_enemy(&enemies[0], ENEMY_INLET);
        init_enemy(&enemies[1], ENEMY_INLET);
        init_enemy(&enemies[2], ENEMY_SEAPUNK);
    }
    else if (encounter == 11) {
        *enemy_count = 2;
        init_enemy(&enemies[0], ENEMY_JAW_WORM);
        init_enemy(&enemies[1], ENEMY_SEAPUNK);
    }
    else if (encounter == 12) {
        *enemy_count = 2;
        init_enemy(&enemies[0], ENEMY_SHRINKER_BEETLE);
        init_enemy(&enemies[1], ENEMY_SLUDGE_SPINNER);
    }
    else {
        *enemy_count = 3;
        init_enemy(&enemies[0], ENEMY_TWIG_SLIME);
        init_enemy(&enemies[1], ENEMY_INLET);
        init_enemy(&enemies[2], ENEMY_INLET);
    }
}

//일반적 설정 함수
static void init_normal_enemies(int floor, Enemy enemies[], int *enemy_count)
{
    if (enemies == NULL || enemy_count == NULL) {
        return;
    }

    if (floor <= 3) {
        init_start_normal_enemies(enemies, enemy_count);
    }
    else {
        init_later_normal_enemies(enemies, enemy_count);
    }
}

//스테이지 별 적 설정 함수
static void init_battle_enemies(StageType stage,int floor, Enemy enemies[], int *enemy_count)
{
    if (enemies == NULL || enemy_count == NULL) {
        return;
    }

    if (stage == STAGE_BOSS) {
        init_boss_enemies(enemies, enemy_count);
    }
    else if (stage == STAGE_ELITE) {
        init_elite_enemies(enemies, enemy_count);
    }
    else {
        init_normal_enemies(floor,enemies, enemy_count);
    }
}

//적 의도 보여주는 함수
static const char *get_enemy_intent_text(const Enemy *enemy)
{
    if (enemy == NULL) {
        return "Unknown";
    }

    switch (enemy->id) {
    case ENEMY_VANTOM:
    if (enemy->pattern_index == 0) {
        return "잉크 투척: 공격 7";
    }
    else if (enemy->pattern_index == 1) {
        return "잉크 창: 공격 6 x 2";
    }
    else if (enemy->pattern_index == 2) {
        return "토막내기: 공격 27 + 부상 3장";
    }
    else {
        return "준비: 힘 2";
    }
    case ENEMY_TERROR_EEL:
    if (enemy->pattern_index == 3) {
        return "기절: 아무것도 하지 않음";
    }
    else if (enemy->pattern_index == 2) {
        return "공포: 취약 99";
    }
    else if (enemy->pattern_index == 0) {
        return "충돌: 공격 16";
    }
    else {
        return "난동부리기: 공격 3 x 3 + 활력 6";
    }
    case ENEMY_BYRDONIS:
    if (enemy->pattern_index == 0) {
        return "물기: 공격 17";
    } else {
        return "쪼기: 공격 3 x 3";
    }
    case ENEMY_BYGONE_EFFIGY:
    if (enemy->pattern_index == 0) {
        return "수면: 아무것도 하지 않음";
    }
    else if (enemy->pattern_index == 1) {
        return "깨어남: 힘 10";
    }
    else {
        return "참격: 공격 13";
    }
    case ENEMY_TWIG_SLIME:
    if (enemy->pattern_index == 0) {
        return "버린 더미에 점액투성이 1장 추가";
    } else {
        return "공격 11";
    }
    case ENEMY_LEAF_SLIME:
    if (enemy->pattern_index == 0) {
        return "버린 더미에 점액투성이 2장 추가";
    } else {
        return "공격 8";
    }
    case ENEMY_CUBEX_CONSTRUCT:
    if (enemy->pattern_index == 0) {
        return "힘 2";
    }
    else if (enemy->pattern_index == 1 || enemy->pattern_index == 2) {
        return "공격 7 + 힘 2";
    }
    else if (enemy->pattern_index == 3) {
        return "공격 5 x 2";
    }
    else {
        return "방어도 15";
    }
    case ENEMY_INLET:
    if (enemy->pattern_index == 0) {
        return "공격 3";
    }
    else if (enemy->pattern_index == 1) {
        return "공격 2 x 3";
    }
    else {
        return "공격 10";
    }
    case ENEMY_MAWLER:
    if (enemy->pattern_index == 0) {
        return "공격 4 x 2";
    }
    else if (enemy->pattern_index == 1) {
        return "공격 14";
    }
    else {
        return "취약 3";
    }
    case ENEMY_SLUDGE_SPINNER:
    if (enemy->pattern_index == 0) {
        return "공격 8 + 약화 1";
    }
    else if (enemy->pattern_index == 1) {
        return "공격 11";
    }
    else {
        return "공격 6 + 힘 3";
    }
    case ENEMY_SHRINKER_BEETLE:
    if (enemy->turn_count == 0) {
        return "압축: 플레이어 피해 -30%";
    }

    if (enemy->pattern_index == 0) {
        return "공격 7";
    }
    return "공격 13";

    case ENEMY_FUZZY_WURM_CRAWLER:
    if (enemy->pattern_index == 0) {
        return "공격 4";
    }
    else if (enemy->pattern_index == 1) {
        return "힘 7";
    }
    else if (enemy->pattern_index == 2) {
        return "공격 4";
    }
    else {
        return "공격 4";
    }
    case ENEMY_SEAPUNK:
    if (enemy->pattern_index == 0) {
        return "공격 11";
    }
    else if (enemy->pattern_index == 1) {
        return "공격 4 x 2";
    }
    else {
        return "방어도 7 + 힘 1";
    }
    case ENEMY_JAW_WORM:
        if (enemy->pattern_index == 0) {
            return "공격 12";
        }
        else if (enemy->pattern_index == 1) {
            return "공격 6 + 방어도 5";
        }
        else {
            return "힘 2";
        }

    case ENEMY_SLIME:
    default:
        return "공격 6";
    }
}

//전투화면 출력함수
static void draw_temp_battle_screen(GameState *state,Enemy enemies[],int enemy_count,int target_index,int selected,int max_display_hand,int battle_width,int battle_height,int card_slot_width)
{
    Player *player;
    const Card *selected_card;
    int hand_count;
int start_y;
int start_x;
int player_y;
int card_y;
int i;

    if (state == NULL || enemies == NULL || enemy_count <= 0) {
        return;
    }

    player = &state->player;

    start_y = (LINES - battle_height) / 3;
    start_x = (COLS - battle_width) / 2;

    if (start_y < 0) {
        start_y = 0;
    }

    if (start_x < 0) {
        start_x = 0;
    }

    hand_count = get_display_hand_count(player, max_display_hand);

clear();

player_y = start_y + 4 + enemy_count * 4 + 1;
card_y = player_y + 4;

    print_battle_line(start_y + 0, start_x, battle_width);
    mvprintw(start_y + 1, start_x,
             "Floor %-3d        Gold %-4d        Relic: ",
             state->floor,
             player->gold);
    print_relic_summary(player);
    print_battle_line(start_y + 2, start_x, battle_width);

    for (i = 0; i < enemy_count; i++) {
    int enemy_y = start_y + 4 + i * 4;

    if (i == target_index && enemies[i].hp > 0) {
        attron(A_REVERSE);
    }

    mvprintw(enemy_y, start_x,"Enemy %d: %s",i + 1,enemies[i].name);

    if (i == target_index && enemies[i].hp > 0) {
        attroff(A_REVERSE);
    }

    if (enemies[i].id == ENEMY_CUBEX_CONSTRUCT) {
    mvprintw(enemy_y + 1, start_x,
             "HP %d/%d   Block %d   Str %d   Weak %d   Vul %d   Art %d",
             enemies[i].hp,
             enemies[i].max_hp,
             enemies[i].block,
             enemies[i].strength,
             enemies[i].weak,
             enemies[i].vulnerable,
             enemies[i].special_state);
}
else if ((enemies[i].id == ENEMY_INLET || enemies[i].id == ENEMY_VANTOM) &&
         enemies[i].special_state > 0) {
    mvprintw(enemy_y + 1, start_x,
             "HP %d/%d   Block %d   Str %d   Weak %d   Vul %d   Slip %d",
             enemies[i].hp,
             enemies[i].max_hp,
             enemies[i].block,
             enemies[i].strength,
             enemies[i].weak,
             enemies[i].vulnerable,
             enemies[i].special_state);
}
else {
    mvprintw(enemy_y + 1, start_x,
             "HP %d/%d   Block %d   Str %d   Weak %d   Vul %d",
             enemies[i].hp,
             enemies[i].max_hp,
             enemies[i].block,
             enemies[i].strength,
             enemies[i].weak,
             enemies[i].vulnerable);
}
    mvprintw(enemy_y + 2, start_x,"Intent: %s",get_enemy_intent_text(&enemies[i]));
}

    print_battle_dash(player_y - 2, start_x, battle_width);

    mvprintw(player_y, start_x,
         "Player: %s",
         player->name);

mvprintw(player_y + 1, start_x,
         "HP %d/%d   Block %d   Energy %d/%d   Str %d   Weak %d   Vul %d",
         player->hp,
         player->max_hp,
         player->block,
         player->energy,
         player->max_energy,
         player->strength,
         player->weak,
         player->vulnerable);

if (has_active_shrink_effect_for_ui(enemies, enemy_count)) {
    mvprintw(player_y + 2, start_x,
             "Status: 압축  - 플레이어가 주는 피해 30%% 감소");
} else {
    mvprintw(player_y + 2, start_x,
             "Status: 정상");
}

print_battle_line(card_y, start_x, battle_width);

mvprintw(card_y + 1, start_x, "Selected Card");

if (hand_count > 0) {
    selected_card = &player->hand[selected];

    mvprintw(card_y + 3, start_x,
             "%s   Cost %d",
             selected_card->name,
             selected_card->cost);

    mvprintw(card_y + 4, start_x,
             "%s",
             selected_card->description);
}
else {
    selected_card = NULL;

    mvprintw(card_y + 3, start_x,
             "현재 손패에 카드가 없습니다.");
}

print_battle_line(card_y + 6, start_x, battle_width);

mvprintw(card_y + 8, start_x, "Hand");

for (i = 0; i < hand_count; i++) {
    int row = card_y + 9 + (i / 5);
    int col = start_x + (i % 5) * card_slot_width;
    char display_name[MAX_NAME_LEN];
    int corrupted;

    corrupted = make_card_hand_display_name(player->hand[i].name,
                                            display_name,
                                            sizeof(display_name));

    if (i == selected) {
        attron(A_REVERSE);
    }

    if (corrupted) {
        mvprintw(row, col,
                 "[%d]%s*(%d)",
                 i + 1,
                 display_name,
                 player->hand[i].cost);
    } else {
        mvprintw(row, col,
                 "[%d]%s(%d)",
                 i + 1,
                 display_name,
                 player->hand[i].cost);
    }

    if (i == selected) {
        attroff(A_REVERSE);
    }
}

mvprintw(card_y + 12, start_x,
         "← → 카드 선택   ↑ ↓ 대상 선택   A/D 카드   W/S 대상   Enter 사용   I 인벤토리   E 턴 종료   Q 종료");

    refresh();
}

//인벤토리 화면 출력 함수
static void show_battle_inventory_menu(Player *player)
{
    int sub;

    if (player == NULL) {
        return;
    }

    clear();

    mvprintw(3, 5, "[ INVENTORY ]");
    mvprintw(5, 5, "1. Draw Deck");
    mvprintw(6, 5, "2. Discard Pile");
    mvprintw(7, 5, "3. Exhaust Pile");
    mvprintw(8, 5, "4. Relics");

    refresh();

    sub = getch();

    if (sub == '1') {
        show_card_pile_screen("DRAW DECK",
                              player->draw_pile,
                              player->draw_count,
                              1);
    }
    else if (sub == '2') {
        show_card_pile_screen("DISCARD PILE",
                              player->discard_pile,
                              player->discard_count,
                              0);
    }
    else if (sub == '3') {
        show_card_pile_screen("EXHAUST PILE",
                              player->exhaust_pile,
                              player->exhaust_count,
                              0);
    }
    else if (sub == '4') {
        show_relic_inventory_screen(player);
    }
}

//카드 사용 처리 함수
static BattleResult handle_play_selected_card(GameState *state,Enemy enemies[],int enemy_count,int selected,int target_index,int message_y,int message_x)
{
    Player *player;
    BattleResult battle_result;

    if (state == NULL || enemies == NULL || enemy_count <= 0) {
        return BATTLE_CONTINUE;
    }

    player = &state->player;

    if (target_index < 0 || target_index >= enemy_count) {
        show_battle_message(message_y,message_x,"공격 대상을 선택할 수 없습니다.");

        return BATTLE_CONTINUE;
    }

    if (player->hand_count <= 0) {
        show_battle_message(message_y,message_x,"사용할 카드가 없습니다.");

        return BATTLE_CONTINUE;
    }

    if (enemies[target_index].hp <= 0) {
        show_battle_message(message_y,message_x,"선택한 적이 이미 쓰러졌습니다.");

        return BATTLE_CONTINUE;
    }

    if (player->hp <= 0) {
        show_battle_message(message_y,message_x,"플레이어가 쓰러져 카드를 사용할 수 없습니다.");

        return BATTLE_CONTINUE;
    }

    if (play_card(player, enemies, enemy_count, selected, target_index)) {
        battle_result = check_battle_result(player, enemies, enemy_count);

        return battle_result;
    }

    show_battle_message(message_y,message_x,"카드를 사용할 수 없습니다. 에너지 또는 대상 상태를 확인하세요.");

    return BATTLE_CONTINUE;
}

//턴종료 처리함수
static BattleResult handle_end_turn(GameState *state,Enemy enemies[],int enemy_count,int message_y,int message_x,int *turn_number)
{
    Player *player;
    BattleResult battle_result;

    if (state == NULL || enemies == NULL || enemy_count <= 0) {
        return BATTLE_CONTINUE;
    }

    player = &state->player;

    if (player->hp <= 0) {
        show_battle_message(message_y,message_x,
                            "플레이어가 쓰러져 턴을 종료할 수 없습니다.");
        return BATTLE_CONTINUE;
    }

    if (turn_number != NULL) {
        apply_relics_on_turn_end(player, enemies, enemy_count, *turn_number);

        battle_result = check_battle_result(player, enemies, enemy_count);

        if (battle_result != BATTLE_CONTINUE) {
            return battle_result;
        }
    }

    decrease_player_turn_statuses(player);

    discard_hand(player);

    reset_bygone_effigy_slow(enemies, enemy_count);

    enemies_take_turn(enemies, enemy_count, player);

    battle_result = check_battle_result(player, enemies, enemy_count);

    if (battle_result != BATTLE_CONTINUE) {
        return battle_result;
    }

    player->block = 0;

    if (has_relic(player, RELIC_ICE_CREAM)) {
        player->energy += player->max_energy;
    } else {
        player->energy = player->max_energy;
    }

    if (turn_number != NULL) {
        (*turn_number)++;
        player->exhausted_this_turn = 0;
        player->hp_lost_this_turn = 0;
        apply_relics_on_turn_start(player, enemies, enemy_count, *turn_number);
        apply_player_turn_start_powers(player, enemies, enemy_count);
    }

    battle_result = check_battle_result(player, enemies, enemy_count);

    if (battle_result != BATTLE_CONTINUE) {
        return battle_result;
    }

    draw_cards(player, 5);

    return BATTLE_CONTINUE;
}

//압축 여부 확인 함수
static int has_active_shrink_effect_for_ui(Enemy enemies[], int enemy_count)
{
    int i;

    if (enemies == NULL || enemy_count <= 0) {
        return 0;
    }

    if (enemy_count > MAX_ENEMIES) {
        enemy_count = MAX_ENEMIES;
    }

    for (i = 0; i < enemy_count; i++) {
        if (enemies[i].id == ENEMY_SHRINKER_BEETLE &&
            enemies[i].hp > 0 &&
            enemies[i].turn_count > 0) {
            return 1;
        }
    }

    return 0;
}

//카드 설명 줄바꿈 함수
static void print_wrapped_text(int y, int x, const char *text, int width, int max_lines)
{
    char buffer[256];
    char line[256];
    char *word;
    int current_y;
    int printed_lines;
    int line_len;

    if (text == NULL || width <= 0 || max_lines <= 0) {
        return;
    }

    strncpy(buffer, text, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    line[0] = '\0';
    current_y = y;
    printed_lines = 0;
    line_len = 0;

    word = strtok(buffer, " ");

    while (word != NULL && printed_lines < max_lines) {
        int word_len;

        word_len = strlen(word);

        if (line_len == 0) {
            strncpy(line, word, sizeof(line) - 1);
            line[sizeof(line) - 1] = '\0';
            line_len = word_len;
        } else if (line_len + 1 + word_len <= width) {
            strncat(line, " ", sizeof(line) - strlen(line) - 1);
            strncat(line, word, sizeof(line) - strlen(line) - 1);
            line_len += word_len + 1;
        } else {
            mvprintw(current_y, x, "%s", line);
            current_y++;
            printed_lines++;

            strncpy(line, word, sizeof(line) - 1);
            line[sizeof(line) - 1] = '\0';
            line_len = word_len;
        }

        word = strtok(NULL, " ");
    }

    if (line_len > 0 && printed_lines < max_lines) {
        mvprintw(current_y, x, "%s", line);
    }
}

//전투 보상 화면 출력 함수
void show_battle_reward_screen(GameState *state, StageType stage)
{
    Card rewards[CARD_REWARD_COUNT];
    Player *player;
    int selected;
    int ch;
    int i;
    int start_y;
    int start_x;
    int card_width;
    int gold_reward;
    int has_relic_reward;
    Relic relic_reward;

    if (state == NULL) {
        return;
    }

    player = &state->player;
    selected = 0;
    card_width = 26;

has_relic_reward = 0;

if (stage == STAGE_ELITE) {
    gold_reward = generate_gold_reward(40, 60);
    has_relic_reward = grant_random_standard_relic(player, &relic_reward);
} else {
    gold_reward = generate_gold_reward(20, 30);
}

player->gold += gold_reward;

generate_card_rewards(rewards, CARD_REWARD_COUNT, stage);

    while (1) {
        clear();

        start_y = 2;
        start_x = 4;

        mvprintw(start_y + 2, start_x, "획득 골드: %d", gold_reward);
mvprintw(start_y + 3, start_x, "현재 골드: %d", player->gold);

if (has_relic_reward) {
    mvprintw(start_y + 4, start_x, "획득 유물: %s", relic_reward.name);
    mvprintw(start_y + 5, start_x, "%s", relic_reward.description);
}

mvprintw(start_y + 7, start_x, "카드 보상을 선택하세요.");
mvprintw(start_y + 8, start_x, "←/→ 또는 A/D: 선택 이동");
mvprintw(start_y + 9, start_x, "Enter 또는 1/2/3: 선택");
mvprintw(start_y + 10, start_x, "S 또는 0: 카드 보상 넘기기");

        for (i = 0; i < CARD_REWARD_COUNT; i++) {
            int card_x;

            card_x = start_x + i * 32;

            if (i == selected) {
    mvprintw(start_y + 13, card_x, ">");
} else {
    mvprintw(start_y + 13, card_x, " ");
}

mvprintw(start_y + 13, card_x + 2, "[%d]", i + 1);
mvprintw(start_y + 14, card_x + 2, "%s", rewards[i].name);
mvprintw(start_y + 15, card_x + 2, "비용: %d", rewards[i].cost);

print_wrapped_text(start_y + 17,
                   card_x + 2,
                   rewards[i].description,
                   card_width,
                   4);
        }

        refresh();

        ch = getch();

        if (ch == KEY_LEFT || ch == 'a' || ch == 'A') {
            selected--;

            if (selected < 0) {
                selected = CARD_REWARD_COUNT - 1;
            }
        } else if (ch == KEY_RIGHT || ch == 'd' || ch == 'D') {
            selected++;

            if (selected >= CARD_REWARD_COUNT) {
                selected = 0;
            }
        } else if (ch == '\n' || ch == KEY_ENTER) {
            add_card_to_deck(player, rewards[selected]);
            show_card_added_message(&rewards[selected]);
            break;
        } else if (ch == '1') {
            add_card_to_deck(player, rewards[0]);
            show_card_added_message(&rewards[0]);
            break;
        } else if (ch == '2') {
            add_card_to_deck(player, rewards[1]);
            show_card_added_message(&rewards[1]);
            break;
        } else if (ch == '3') {
            add_card_to_deck(player, rewards[2]);
            show_card_added_message(&rewards[2]);
            break;
        } else if (ch == 's' || ch == 'S' || ch == '0') {
            break;
        }
    }
}

//임시 스테이지 현황 출력 함수
void show_current_stage_screen(int floor, StageType stage)
{
    clear();

    mvprintw(5, 5, "현재 층: %d층", floor);
    mvprintw(7, 5, "스테이지: %s", get_stage_type_name(stage));
    mvprintw(9, 5, "아무 키나 누르면 진행합니다.");

    refresh();
    getch();

    clear();
    refresh();
}

//죽은 적을 선택하는거 방지 함수
static void normalize_target_index(int *target_index,Enemy enemies[],int enemy_count)
{
    int i;

    if (target_index == NULL || enemies == NULL || enemy_count <= 0) {
        return;
    }

    if (*target_index < 0) {
        *target_index = 0;
    }

    if (*target_index >= enemy_count) {
        *target_index = enemy_count - 1;
    }

    if (enemies[*target_index].hp > 0) {
        return;
    }

    for (i = 0; i < enemy_count; i++) {
        if (enemies[i].hp > 0) {
            *target_index = i;
            return;
        }
    }

    *target_index = 0;
}

//공격 대상을 바꾸는 함수
static void move_target_index(int *target_index,Enemy enemies[],int enemy_count,int direction)
{
    int next;
    int checked;

    if (target_index == NULL || enemies == NULL || enemy_count <= 0) {
        return;
    }

    next = *target_index;
    checked = 0;

    while (checked < enemy_count) {
        next += direction;

        if (next < 0) {
            next = enemy_count - 1;
        }

        if (next >= enemy_count) {
            next = 0;
        }

        if (enemies[next].hp > 0) {
            *target_index = next;
            return;
        }

        checked++;
    }

    normalize_target_index(target_index, enemies, enemy_count);
}

//휴식 부분 화면 출력 함수

//휴식 선택화면 출력 함수
int show_rest_choice_screen(const Player *player)
{
    int selected = 0;
    int ch;

    while (1) {
        clear();

        mvprintw(3, 5, "휴식 장소");
        mvprintw(5, 5, "현재 체력: %d / %d", player->hp, player->max_hp);
        mvprintw(6, 5, "현재 덱 카드 수: %d", player->owned_deck_count);

        if (selected == 0) {
            attron(A_REVERSE);
        }
        mvprintw(9, 5, "1. 휴식하기 - 최대 체력의 30%% 회복");
        if (selected == 0) {
            attroff(A_REVERSE);
        }

        if (selected == 1) {
            attron(A_REVERSE);
        }
        mvprintw(11, 5, "2. 카드 제거하기 - 덱에서 카드 1장 제거");
        if (selected == 1) {
            attroff(A_REVERSE);
        }

        mvprintw(14, 5, "W/S 또는 방향키로 이동, Enter로 선택");
        refresh();

        ch = getch();

        if (ch == KEY_UP || ch == 'w' || ch == 'W') {
            selected--;
            if (selected < 0) {
                selected = 1;
            }
        } else if (ch == KEY_DOWN || ch == 's' || ch == 'S') {
            selected++;
            if (selected > 1) {
                selected = 0;
            }
        } else if (ch == '\n' || ch == KEY_ENTER || ch == 10 || ch == 13) {
            return selected + 1;
        } else if (ch == '1') {
            return 1;
        } else if (ch == '2') {
            return 2;
        }
    }
}

//덱 카드 제거 선택시 나오는 화면 함수
int show_remove_card_screen(const Player *player)
{
    int selected;
    int page;
    int cards_per_page;
    int total_pages;
    int start_index;
    int end_index;
    int i;
    int ch;
    int rows;
    int cols;
    int desc_width;

    if (player == NULL || player->owned_deck_count <= 0) {
        return -1;
    }

    selected = 0;
    page = 0;
    cards_per_page = 10;

    total_pages = (player->owned_deck_count + cards_per_page - 1) / cards_per_page;

    while (1) {
        getmaxyx(stdscr, rows, cols);

        desc_width = cols - 10;
        if (desc_width < 20) {
            desc_width = 20;
        }

        start_index = page * cards_per_page;
        end_index = start_index + cards_per_page;

        if (end_index > player->owned_deck_count) {
            end_index = player->owned_deck_count;
        }

        if (selected < start_index) {
            selected = start_index;
        }

        if (selected >= end_index) {
            selected = end_index - 1;
        }

        clear();

        mvprintw(2, 5, "제거할 카드를 선택하세요.");
        mvprintw(3, 5, "덱 카드 수: %d", player->owned_deck_count);
        mvprintw(4, 5, "페이지: %d / %d", page + 1, total_pages);
        mvprintw(5, 5, "W/S 또는 ↑/↓ 이동, A/D 또는 ←/→ 페이지 이동, Enter 선택, Q 취소");

        for (i = start_index; i < end_index; i++) {
            int line_y;

            line_y = 7 + (i - start_index);

            if (i == selected) {
                attron(A_REVERSE);
            }

            mvprintw(line_y, 5, "%2d. [%d] %s",
                     i + 1,
                     player->owned_deck[i].cost,
                     player->owned_deck[i].name);

            if (i == selected) {
                attroff(A_REVERSE);
            }
        }

        mvprintw(rows - 5, 5, "선택 카드 설명:");
        print_wrapped_text(rows - 4, 5,
                           player->owned_deck[selected].description,
                           desc_width,
                           3);

        refresh();

        ch = getch();

        if (ch == KEY_UP || ch == 'w' || ch == 'W') {
            selected--;

            if (selected < start_index) {
                selected = end_index - 1;
            }
        } else if (ch == KEY_DOWN || ch == 's' || ch == 'S') {
            selected++;

            if (selected >= end_index) {
                selected = start_index;
            }
        } else if (ch == KEY_LEFT || ch == 'a' || ch == 'A') {
            if (page > 0) {
                page--;
                selected = page * cards_per_page;
            }
        } else if (ch == KEY_RIGHT || ch == 'd' || ch == 'D') {
            if (page < total_pages - 1) {
                page++;
                selected = page * cards_per_page;
            }
        } else if (ch == '\n' || ch == KEY_ENTER || ch == 10 || ch == 13) {
            return selected;
        } else if (ch == 'q' || ch == 'Q') {
            return -1;
        }
    }
}

//회복 결과 화면 출력 함수
void show_rest_result_screen(int healed, const Player *player)
{
    clear();

    mvprintw(5, 5, "휴식을 취했습니다.");
    mvprintw(7, 5, "체력을 %d 회복했습니다.", healed);

    if (player != NULL) {
        mvprintw(9, 5, "현재 체력: %d / %d", player->hp, player->max_hp);
    }

    mvprintw(12, 5, "아무 키나 누르면 다음 층으로 이동합니다.");

    refresh();
    getch();

    clear();
    refresh();
}

//카드 제거 화면 출력 함수
void show_card_removed_screen(const Card *card)
{
    clear();

    if (card != NULL) {
        mvprintw(5, 5, "%s 카드를 덱에서 제거했습니다.", card->name);
    } else {
        mvprintw(5, 5, "카드를 제거했습니다.");
    }

    mvprintw(7, 5, "아무 키나 누르면 다음 층으로 이동합니다.");

    refresh();
    getch();

    clear();
    refresh();
}

//카드 제거 불가능시 출력 함수
void show_card_remove_unavailable_screen(void)
{
    clear();

    mvprintw(5, 5, "카드를 제거할 수 없습니다.");
    mvprintw(7, 5, "덱에 카드가 10장 이하이면 제거할 수 없습니다.");
    mvprintw(9, 5, "아무 키나 누르면 휴식 선택으로 돌아갑니다.");

    refresh();
    getch();

    clear();
    refresh();
}

//유물을 얻었을때 나오는 화면
void show_relic_obtained_screen(const char *title, const Relic *relic)
{
    int rows;
    const char *screen_title;
    char relic_name_line[128];

    if (relic == NULL) {
        return;
    }

    rows = getmaxy(stdscr);

    screen_title = title != NULL ? title : "유물 획득";
    snprintf(relic_name_line, sizeof(relic_name_line), "[ %s ]", relic->name);

    clear();

    print_centered(rows / 2 - 5, screen_title);
    print_centered(rows / 2 - 3, "새로운 유물을 얻었습니다!");
    print_centered(rows / 2 - 1, relic_name_line);
    print_centered(rows / 2 + 1, relic->description);
    print_centered(rows / 2 + 4, "아무 키나 누르면 진행합니다.");

    refresh();
    getch();
}

//획득가능한 유물이 없을때 나오는 화면
void show_no_relic_available_screen(void)
{
    int rows;
    int cols;

    getmaxyx(stdscr, rows, cols);

    clear();

    mvprintw(rows / 2 - 2, (cols - 28) / 2, "획득 가능한 유물이 없습니다.");
    mvprintw(rows / 2, (cols - 30) / 2, "아무 키나 누르면 진행합니다.");

    refresh();
    getch();
}

//상점 선택 결과 메시지 출력 함수
static const char *shop_buy_result_message(ShopBuyResult result)
{
    switch (result) {
    case SHOP_BUY_OK:
        return "구매했습니다.";
    case SHOP_BUY_SOLD:
        return "이미 구매한 상품입니다.";
    case SHOP_BUY_NOT_ENOUGH_GOLD:
        return "골드가 부족합니다.";
    case SHOP_BUY_NEED_CARD_SELECT:
        return "제거할 카드를 선택하세요.";
    case SHOP_BUY_DECK_FULL:
        return "덱이 가득 찼습니다.";
    case SHOP_BUY_RELIC_FULL:
        return "유물을 더 이상 가질 수 없습니다.";
    case SHOP_BUY_REMOVE_UNAVAILABLE:
        return "덱이 10장 이하이면 카드를 제거할 수 없습니다.";
    default:
        return "구매할 수 없습니다.";
    }
}

//상점 아이템 상황(품절,할인,가격)출력 함수
static void print_shop_item_line(int y, int x, int selected, int index, const ShopItem *item)
{
    const char *cursor;

    cursor = selected ? ">" : " ";

    if (item == NULL || !item->available) {
        mvprintw(y, x, "%s [%d] 품절", cursor, index + 1);
        return;
    }

    if (item->sold) {
        switch (item->type) {
        case SHOP_ITEM_CARD:
            mvprintw(y, x, "%s [%d] %-16s SOLD", cursor, index + 1, item->card.name);
            break;
        case SHOP_ITEM_RELIC:
            mvprintw(y, x, "%s [%d] %-16s SOLD", cursor, index + 1, item->relic.name);
            break;
        case SHOP_ITEM_REMOVE_CARD:
            mvprintw(y, x, "%s [%d] 카드 제거        SOLD", cursor, index + 1);
            break;
        default:
            mvprintw(y, x, "%s [%d] 품절", cursor, index + 1);
            break;
        }
        return;
    }

    switch (item->type) {
    case SHOP_ITEM_CARD:
        mvprintw(y, x, "%s [%d] 카드  %-16s %3dG", cursor, index + 1, item->card.name, item->price);
        if (item->discounted) {
            printw(" SALE");
        }
        break;

    case SHOP_ITEM_RELIC:
        mvprintw(y, x, "%s [%d] 유물  %-16s %3dG", cursor, index + 1, item->relic.name, item->price);
        if (item->discounted) {
            printw(" SALE");
        }
        break;

    case SHOP_ITEM_REMOVE_CARD:
        mvprintw(y, x, "%s [%d] 서비스 카드 제거       %3dG", cursor, index + 1, item->price);
        break;

    default:
        mvprintw(y, x, "%s [%d] 품절", cursor, index + 1);
        break;
    }
}

//상점 아이템 설명 출력 함수
static void print_shop_item_description(int y, int x, const ShopItem *item)
{
    if (item == NULL || !item->available) {
        mvprintw(y, x, "설명: 구매 가능한 상품이 없습니다.");
        return;
    }

    switch (item->type) {
    case SHOP_ITEM_CARD:
        mvprintw(y, x, "설명: %s", item->card.description);
        mvprintw(y + 1, x, "코스트: %d", item->card.cost);
        break;

    case SHOP_ITEM_RELIC:
        mvprintw(y, x, "설명: %s", item->relic.description);
        break;

    case SHOP_ITEM_REMOVE_CARD:
        mvprintw(y, x, "설명: 덱에서 카드 1장을 제거합니다.");
        mvprintw(y + 1, x, "덱이 %d장 이하이면 제거할 수 없습니다.", SHOP_MIN_DECK_SIZE);
        break;

    default:
        mvprintw(y, x, "설명: 없음");
        break;
    }
}

//상점 화면 출력 함수
int show_shop_screen(GameState *state, Shop *shop)
{
    int selected = 0;
    int ch;
    int i;
    int remove_index;
    ShopBuyResult result;
    const char *help_message = "W/S 또는 ↑/↓ 이동, Enter 구매, Q 상점 나가기 및 다음 층 이동";
    const char *status_message = "";

    if (state == NULL || shop == NULL) {
        return 0;
    }

    keypad(stdscr, TRUE);

    while (1) {
        clear();

        mvprintw(1, 2, "==================== 상점 ====================");
        mvprintw(3, 2, "Floor %-3d    Gold %-4d", state->floor, state->player.gold);

        mvprintw(5, 2, "Cards");
        for (i = 0; i < 5 && i < shop->item_count; i++) {
            print_shop_item_line(6 + i, 4, selected == i, i, &shop->items[i]);
        }

        mvprintw(12, 2, "Relics");
        for (i = 5; i < 7 && i < shop->item_count; i++) {
            print_shop_item_line(13 + (i - 5), 4, selected == i, i, &shop->items[i]);
        }

        mvprintw(16, 2, "Service");
        if (shop->item_count > 7) {
            print_shop_item_line(17, 4, selected == 7, 7, &shop->items[7]);
        }

        mvprintw(20, 2, "----------------------------------------------");
        if (selected >= 0 && selected < shop->item_count) {
            print_shop_item_description(21, 2, &shop->items[selected]);
        }

        mvprintw(24, 2, "----------------------------------------------");

        if (status_message[0] != '\0') {
            mvprintw(25, 2, "상태: %s", status_message);
        } else {            
            mvprintw(25, 2, "상태: 상품을 선택하세요.");
        }

        mvprintw(26, 2, "조작: %s", help_message);

        refresh();

        ch = getch();

        if (ch == 'q' || ch == 'Q') {
            return 1;
        }

        if (ch == KEY_UP || ch == 'w' || ch == 'W') {
            if (selected > 0) {
                selected--;
            }
        } else if (ch == KEY_DOWN || ch == 's' || ch == 'S') {
            if (selected < shop->item_count - 1) {
                selected++;
            }
        } else if (ch == '\n' || ch == KEY_ENTER || ch == 10 || ch == 13) {
            if (selected < 0 || selected >= shop->item_count) {
                status_message = "잘못된 선택입니다.";
                continue;
            }

            if (shop->items[selected].type == SHOP_ITEM_REMOVE_CARD) {
                result = buy_shop_item(&state->player, &shop->items[selected]);

                if (result == SHOP_BUY_NEED_CARD_SELECT) {
                    remove_index = show_remove_card_screen(&state->player);

                    if (remove_index < 0) {
                        status_message = "카드 제거를 취소했습니다.";
                    } else {
                        result = buy_shop_remove_card(&state->player, &shop->items[selected], remove_index);
                        status_message = shop_buy_result_message(result);
                    }
                } else {
                    status_message = shop_buy_result_message(result);
                }
            } else {
                result = buy_shop_item(&state->player, &shop->items[selected]);
                status_message = shop_buy_result_message(result);
            }
        }
    }
}    

//공용 카드 더미 출력 
static void show_card_pile_screen(const char *title,const Card *cards,int count,int reverse_order)
{
    int selected = 0;
    int ch;
    int i;
    int card_index;

    if (title == NULL || cards == NULL) {
        return;
    }

    while (1) {
        clear();

        mvprintw(1, 3, "========== %s ==========", title);

        if (count <= 0) {
            mvprintw(3, 5, "카드 더미가 비어 있습니다.");
            mvprintw(LINES - 2, 3, "Q : 돌아가기");

            refresh();
            ch = getch();

            if (ch == 'q' || ch == 'Q') {
                break;
            }

            continue;
        }

        for (i = 0; i < count; i++) {
            if (reverse_order) {
                card_index = count - 1 - i;
            } else {
                card_index = i;
            }

            if (i == selected) {
                mvprintw(i + 3, 5, "> %s", cards[card_index].name);
            } else {
                mvprintw(i + 3, 5, "  %s", cards[card_index].name);
            }
        }

        mvprintw(LINES - 3, 3, "↑ ↓ : 이동");
        mvprintw(LINES - 2, 3,
                 "ENTER : 상세보기   Q : 돌아가기");

        refresh();

        ch = getch();

        if (ch == KEY_UP) {
            selected--;

            if (selected < 0) {
                selected = count - 1;
            }
        }
        else if (ch == KEY_DOWN) {
            selected++;

            if (selected >= count) {
                selected = 0;
            }
        }
        else if (ch == '\n' || ch == KEY_ENTER) {
            if (reverse_order) {
                card_index = count - 1 - selected;
            } else {
                card_index = selected;
            }

            show_card_detail_screen(&cards[card_index]);
        }
        else if (ch == 'q' || ch == 'Q') {
            break;
        }
    }
}

// 플레이어 유물 출력 화면
void show_relic_inventory_screen(const Player *player)
{
    int selected = 0;
    int ch;
    int i;

    if (player == NULL) {
        return;
    }

    while (1) {

        clear();

        mvprintw(1, 3,
                 "============= RELICS =============");

        if (player->relic_count <= 0) {

            mvprintw(3, 5,
                     "보유한 유물이 없습니다.");

        } else {

            for (i = 0; i < player->relic_count; i++) {

                if (i == selected) {

                    mvprintw(i + 3, 5,
                             "> %s",
                             player->relics[i].name);

                } else {

                    mvprintw(i + 3, 5,
                             "  %s",
                             player->relics[i].name);
                }
            }
        }

        mvprintw(LINES - 3, 3,
                 "↑ ↓ : 이동");

        mvprintw(LINES - 2, 3,
                 "ENTER : 상세보기   Q : 돌아가기");

        refresh();

        ch = getch();

        if (ch == KEY_UP) {

            selected--;

            if (selected < 0) {
                selected = player->relic_count - 1;
            }
        }

        else if (ch == KEY_DOWN) {

            selected++;

            if (selected >= player->relic_count) {
                selected = 0;
            }
        }

        else if (ch == '\n' || ch == KEY_ENTER) {

            if (player->relic_count > 0) {

                show_relic_detail_screen(
                    &player->relics[selected]);
            }
        }

        else if (ch == 'q' || ch == 'Q') {

            break;
        }
    }
}

//카드 상세 정보 출력
void show_card_detail_screen(const Card *card)
{
    clear();

    if (card == NULL) {
        mvprintw(3, 5, "카드 정보가 없습니다.");
        refresh();
        getch();
        return;
    }

    mvprintw(1, 3, "========== CARD INFO ==========");

    mvprintw(3, 5, "[ %s ] Cost : %d", card->name,card->cost);
    print_wrapped_text(5, 5, card->description, 60, 5);

    mvprintw(LINES - 2, 3,
             "[ 아무 키나 누르면 돌아갑니다 ]");

    refresh();
    getch();
}

//유물 상세 정보 출력
void show_relic_detail_screen(const Relic *relic)
{
    clear();

    if (relic == NULL) {
        mvprintw(3, 5, "유물 정보가 없습니다.");
        refresh();
        getch();
        return;
    }

    mvprintw(1, 3, "========= RELIC INFO =========");

    mvprintw(3, 5, "[ %s ] ", relic->name);
    print_wrapped_text(5, 5, relic->description,60,5);

    mvprintw(LINES - 2, 3,
             "[ 아무 키나 누르면 돌아갑니다 ]");

    refresh();
    getch();
}

//이벤트 함수

//이벤트 공생체 화면 함수
int show_symbiote_event_screen(void)
{
    int ch;

    while (1) {
        clear();

        mvprintw(2, 5, "==================== 이벤트: 공생체 ====================");
        mvprintw(5, 5, "길을 나아가던 도중, 형태가 일정하지 않은 검은 덩어리를 발견합니다.");
        mvprintw(6, 5, "그 덩어리는 오래되고 사악한 존재처럼 꿈틀거립니다.");

        mvprintw(9, 5, "[1] 다가간다");
        mvprintw(10, 9, "공격 카드 1장을 선택해 [오염]시킵니다.");
        mvprintw(11, 9, "[오염] 카드는 피해량이 50%% 증가하지만, 사용할 때 체력을 2 잃습니다.");

        mvprintw(14, 5, "[2] 불로 태워 죽인다.");
        mvprintw(15, 9, "카드 1장을 선택해 덱에서 제거합니다. (10장이하인경우 불가합니다)");

        mvprintw(18, 5, "1 또는 2를 눌러 선택하세요.");

        refresh();

        ch = getch();

        if (ch == '1') {
            return 1;
        }

        if (ch == '2') {
            return 2;
        }
    }
}

//공격카드만 선택가능한 화면 함수
int show_attack_card_select_screen(const Player *player)
{
    int attack_indices[MAX_DECK_SIZE];
    int attack_count;
    int selected;
    int page;
    int cards_per_page;
    int total_pages;
    int start_index;
    int end_index;
    int i;
    int ch;
    int rows;
    int cols;
    int desc_width;
    int deck_index;

    if (player == NULL || player->owned_deck_count <= 0) {
        return -1;
    }

    attack_count = 0;

    for (i = 0; i < player->owned_deck_count; i++) {
        if (player->owned_deck[i].type == CARD_ATTACK &&
            player->owned_deck[i].damage > 0) {
            attack_indices[attack_count] = i;
            attack_count++;
        }
    }

    if (attack_count <= 0) {
        return -1;
    }

    selected = 0;
    page = 0;
    cards_per_page = 10;

    total_pages = (attack_count + cards_per_page - 1) / cards_per_page;

    while (1) {
        getmaxyx(stdscr, rows, cols);

        desc_width = cols - 10;
        if (desc_width < 20) {
            desc_width = 20;
        }

        start_index = page * cards_per_page;
        end_index = start_index + cards_per_page;

        if (end_index > attack_count) {
            end_index = attack_count;
        }

        if (selected < start_index) {
            selected = start_index;
        }

        if (selected >= end_index) {
            selected = end_index - 1;
        }

        deck_index = attack_indices[selected];

        clear();

        mvprintw(2, 5, "오염시킬 공격 카드를 선택하세요.");
        mvprintw(3, 5, "[오염] 효과: 피해량 50%% 증가, 사용할 때 체력 2 손실");
        mvprintw(4, 5, "공격 카드 수: %d", attack_count);
        mvprintw(5, 5, "페이지: %d / %d", page + 1, total_pages);
        mvprintw(6, 5, "W/S 또는 ↑/↓ 이동, A/D 또는 ←/→ 페이지 이동, Enter 선택, Q 취소");

        for (i = start_index; i < end_index; i++) {
            int line_y;
            int current_deck_index;
            const Card *card;

            line_y = 8 + (i - start_index);
            current_deck_index = attack_indices[i];
            card = &player->owned_deck[current_deck_index];

            if (i == selected) {
                attron(A_REVERSE);
            }

            mvprintw(line_y, 5, "%2d. [%d] %s  피해 %d  HP손실 %d",
                     current_deck_index + 1,
                     card->cost,
                     card->name,
                     card->damage,
                     card->hp_loss);

            if (i == selected) {
                attroff(A_REVERSE);
            }
        }

        mvprintw(rows - 6, 5, "선택 카드:");
        mvprintw(rows - 5, 5, "%s  피해 %d  HP손실 %d",
                 player->owned_deck[deck_index].name,
                 player->owned_deck[deck_index].damage,
                 player->owned_deck[deck_index].hp_loss);

        mvprintw(rows - 4, 5, "카드 설명:");
        print_wrapped_text(rows - 3, 5,
                           player->owned_deck[deck_index].description,
                           desc_width,
                           2);

        refresh();

        ch = getch();

        if (ch == KEY_UP || ch == 'w' || ch == 'W') {
            selected--;

            if (selected < start_index) {
                selected = end_index - 1;
            }
        }
        else if (ch == KEY_DOWN || ch == 's' || ch == 'S') {
            selected++;

            if (selected >= end_index) {
                selected = start_index;
            }
        }
        else if (ch == KEY_LEFT || ch == 'a' || ch == 'A') {
            if (page > 0) {
                page--;
                selected = page * cards_per_page;
            }
        }
        else if (ch == KEY_RIGHT || ch == 'd' || ch == 'D') {
            if (page < total_pages - 1) {
                page++;
                selected = page * cards_per_page;
            }
        }
        else if (ch == '\n' || ch == KEY_ENTER || ch == 10 || ch == 13) {
            return attack_indices[selected];
        }
        else if (ch == 'q' || ch == 'Q') {
            return -1;
        }
    }
}

//오염 결과 화면 함수
void show_card_corrupted_screen(const Card *card)
{
    clear();

    if (card != NULL) {
        mvprintw(5, 5, "%s 카드가 [오염]되었습니다.", card->name);
        mvprintw(7, 5, "현재 피해량: %d", card->damage);
        mvprintw(8, 5, "사용 시 체력 손실: %d", card->hp_loss);
    }
    else {
        mvprintw(5, 5, "카드가 [오염]되었습니다.");
    }

    mvprintw(11, 5, "아무 키나 누르면 다음 층으로 이동합니다.");

    refresh();
    getch();

    clear();
    refresh();
}

//오염가능한 카드가 없을때 나오는 화면
void show_no_attack_card_screen(void)
{
    clear();

    mvprintw(5, 5, "오염시킬 수 있는 공격 카드가 없습니다.");
    mvprintw(7, 5, "아무 키나 누르면 다음 층으로 이동합니다.");

    refresh();
    getch();

    clear();
    refresh();
}

//오염 디버프 표기 함수
static int make_card_hand_display_name(const char *src, char *dest, int dest_size)
{
    const char *tag;
    int i;
    int corrupted;

    if (dest == NULL || dest_size <= 0) {
        return 0;
    }

    dest[0] = '\0';

    if (src == NULL) {
        return 0;
    }

    tag = strstr(src, " [오염]");
    corrupted = tag != NULL;

    if (tag != NULL) {
        for (i = 0; i < dest_size - 1 && src + i < tag; i++) {
            dest[i] = src[i];
        }
        dest[i] = '\0';
    } else {
        for (i = 0; i < dest_size - 1 && src[i] != '\0'; i++) {
            dest[i] = src[i];
        }
        dest[i] = '\0';
    }

    return corrupted;
}

//변성체의 숲 화면
int show_mutating_forest_event_screen(void)
{
    int ch;

    while (1) {
        clear();

        mvprintw(2, 5, "==================== 이벤트: 변성체의 숲 ====================");
        mvprintw(5, 5, "당신은 결정화된 나무들로 가득한 숲에 들어섭니다.");
        mvprintw(6, 5, "나무들이 격렬하게 떨리기 시작하고, 변성체들이 당신에게 말을 걸어옵니다.");

        mvprintw(9, 5, "[1] 무리 (해당 선택지는 카드가 12장 이상인경우 정상 작동합니다.)");
        mvprintw(10, 9, "모든 골드를 잃습니다. 카드를 최대 2장 제거합니다.");
        mvprintw(11, 9, "카드 제거를 취소해도 잃은 골드는 돌아오지 않습니다.");

        mvprintw(14, 5, "[2] 외톨이");
        mvprintw(15, 9, "최대 체력을 5 얻습니다.");

        mvprintw(18, 5, "1 또는 2를 눌러 선택하세요.");

        refresh();

        ch = getch();

        if (ch == '1') {
            return 1;
        }

        if (ch == '2') {
            return 2;
        }
    }
}

//변성체의 숲 1번 선택지
void show_mutating_forest_removed_screen(const Card removed_cards[], int removed_count, int lost_gold)
{
    clear();

    mvprintw(5, 5, "변성체들이 당신의 주머니와 기억을 뒤흔듭니다.");
    mvprintw(7, 5, "잃은 골드: %d", lost_gold);

    if (removed_count <= 0) {
        mvprintw(9, 5, "제거한 카드가 없습니다.");
    }
    else {
        mvprintw(9, 5, "제거한 카드:");

        if (removed_count >= 1) {
            mvprintw(10, 7, "- %s", removed_cards[0].name);
        }

        if (removed_count >= 2) {
            mvprintw(11, 7, "- %s", removed_cards[1].name);
        }
    }

    mvprintw(14, 5, "아무 키나 누르면 다음 층으로 이동합니다.");

    refresh();
    getch();

    clear();
    refresh();
}

//변성체의 숲 2번 선택지
void show_max_hp_increased_screen(const Player *player, int amount)
{
    clear();

    mvprintw(5, 5, "변성체 하나가 당신에게 조용히 다가옵니다.");
    mvprintw(7, 5, "최대 체력이 %d 증가했습니다.", amount);

    if (player != NULL) {
        mvprintw(9, 5, "현재 체력: %d / %d", player->hp, player->max_hp);
    }

    mvprintw(12, 5, "아무 키나 누르면 다음 층으로 이동합니다.");

    refresh();
    getch();

    clear();
    refresh();
}

//정글 미로 탐험 이벤트 선택지
int show_jungle_maze_event_screen(void)
{
    int ch;

    while (1) {
        clear();

        mvprintw(2, 5, "==================== 이벤트: 정글 미로 탐험 ====================");
        mvprintw(5, 5, "당신은 깊은 공동에서 거대한 미로를 내려다봅니다.");
        mvprintw(6, 5, "초조해 보이는 모험가 무리가 미로 앞에서 당신에게 말을 겁니다.");
        mvprintw(7, 5, "그들은 보물을 나눠 갖는 조건으로 함께 움직이자고 제안합니다.");

        mvprintw(10, 5, "[1] 홀로 탐색한다");
        mvprintw(11, 9, "골드 135~165를 얻습니다. 체력을 10 잃습니다.");
        mvprintw(12, 9, "체력이 부족해도 최소 1은 남습니다.");

        mvprintw(15, 5, "[2] 협력한다");
        mvprintw(16, 9, "골드 35~65를 얻습니다.");

        mvprintw(19, 5, "1 또는 2를 눌러 선택하세요.");

        refresh();

        ch = getch();

        if (ch == '1') {
            return 1;
        }

        if (ch == '2') {
            return 2;
        }
    }
}

//정글 미로 탐험 선택지 결과
void show_jungle_maze_result_screen(int choice, int gold_gain, int hp_loss, const Player *player)
{
    clear();

    if (choice == 1) {
        mvprintw(5, 5, "당신은 홀로 미로 깊숙이 들어갑니다.");
        mvprintw(6, 5, "비정한 함정과 수호자들을 뚫고, 묵직한 전리품을 챙겼습니다.");
    }
    else {
        mvprintw(5, 5, "당신은 모험가들과 협력해 미로를 통과합니다.");
        mvprintw(6, 5, "전리품은 나누어 가졌지만, 몸은 무사합니다.");
    }

    mvprintw(9, 5, "획득 골드: %d", gold_gain);

    if (hp_loss > 0) {
        mvprintw(10, 5, "잃은 체력: %d", hp_loss);
    }
    else {
        mvprintw(10, 5, "잃은 체력: 없음");
    }

    if (player != NULL) {
        mvprintw(12, 5, "현재 골드: %d", player->gold);
        mvprintw(13, 5, "현재 체력: %d / %d", player->hp, player->max_hp);
    }

    mvprintw(16, 5, "아무 키나 누르면 다음 층으로 이동합니다.");

    refresh();
    getch();

    clear();
    refresh();
}

//융합자 이벤트
int show_amalgamator_event_screen(void)
{
    int ch;

    while (1) {
        clear();

        mvprintw(2, 5, "==================== 이벤트: 융합자 ====================");
        mvprintw(5, 5, "깡! 깡!!");
        mvprintw(6, 5, "벽 건너편에서 금속과 금속이 부딪히는 소리가 울려 퍼집니다.");
        mvprintw(8, 5, "당신이 벽에 귀를 기울이자, 칼 같은 틈이 열립니다.");
        mvprintw(9, 5, "여섯 개의 팔을 지닌 기묘한 인물이 작업에 몰두하고 있습니다.");
        mvprintw(11, 5, "그는 당신의 카드를 보며 외칩니다.");
        mvprintw(12, 5, "\"결합이다! 더 나은 형태로 만들어주마!\"");

        mvprintw(15, 5, "[1] 수비를 합친다");
        mvprintw(16, 9, "수비 2장을 제거합니다. 궁극의 수비를 덱에 추가합니다.");

        mvprintw(19, 5, "[2] 타격을 합친다");
        mvprintw(20, 9, "타격 2장을 제거합니다. 궁극의 타격을 덱에 추가합니다.");

        mvprintw(23, 5, "1 또는 2를 눌러 선택하세요.");

        refresh();

        ch = getch();

        if (ch == '1') {
            return 1;
        }

        if (ch == '2') {
            return 2;
        }
    }
}

//융합자 이벤트 결과화면
void show_amalgamator_result_screen(const Card *new_card, const char *removed_name, int removed_count)
{
    clear();

    mvprintw(5, 5, "융합자가 망치를 내려칩니다.");
    mvprintw(6, 5, "불꽃이 튀고, 낡은 카드들이 하나의 새로운 형태로 합쳐집니다.");

    if (removed_name != NULL) {
        mvprintw(9, 5, "제거한 카드: %s %d장", removed_name, removed_count);
    }

    if (new_card != NULL) {
        mvprintw(11, 5, "획득한 카드: %s", new_card->name);
        mvprintw(12, 5, "비용: %d", new_card->cost);

        if (new_card->damage > 0) {
            mvprintw(13, 5, "피해량: %d", new_card->damage);
        }

        if (new_card->block > 0) {
            mvprintw(13, 5, "방어도: %d", new_card->block);
        }

        mvprintw(15, 5, "%s", new_card->description);
    }

    mvprintw(18, 5, "아무 키나 누르면 다음 층으로 이동합니다.");

    refresh();
    getch();

    clear();
    refresh();
}

//가라앉은 보물 이벤트
int show_sunken_treasury_event_screen(void)
{
    int ch;

    while (1) {
        clear();

        mvprintw(2, 5, "==================== 이벤트: 가라앉은 보물 ====================");
        mvprintw(5, 5, "길을 따라가던 당신은 일부가 묻힌 장고를 발견합니다.");
        mvprintw(6, 5, "상자는 두 개가 있지만, 무거운 열쇠는 하나뿐입니다.");

        mvprintw(9, 5, "첫 번째 상자는 흔들 때 딸랑이는 소리가 납니다.");
        mvprintw(10, 5, "두 번째 상자는 거대하고 화려하지만, 명백히 수상합니다.");

        mvprintw(13, 5, "[1] 첫 번째 상자");
        mvprintw(14, 9, "골드 52~67을 얻습니다.");

        mvprintw(17, 5, "[2] 두 번째 상자");
        mvprintw(18, 9, "골드 303~363을 얻습니다. 탐욕 카드를 얻습니다.");
        mvprintw(19, 9, "탐욕은 사용할 수 없고 제거할 수 없습니다.");

        mvprintw(22, 5, "1 또는 2를 눌러 선택하세요.");

        refresh();

        ch = getch();

        if (ch == '1') {
            return 1;
        }

        if (ch == '2') {
            return 2;
        }
    }
}

//가라앉은 보물 이벤트 결과화면
void show_sunken_treasury_result_screen(int choice, int gold_gain, const Card *added_card, const Player *player)
{
    clear();

    if (choice == 1) {
        mvprintw(5, 5, "당신은 작은 상자를 열었습니다.");
        mvprintw(6, 5, "낡은 금화들이 기분 좋은 소리를 내며 쏟아집니다.");
    }
    else {
        mvprintw(5, 5, "당신은 거대한 상자를 열었습니다.");
        mvprintw(6, 5, "눈부신 골드가 쏟아지지만, 끈적한 욕망이 손끝에 달라붙습니다.");
    }

    mvprintw(9, 5, "획득 골드: %d", gold_gain);

    if (added_card != NULL) {
        mvprintw(11, 5, "획득한 카드: %s", added_card->name);
        mvprintw(12, 5, "%s", added_card->description);
    }

    if (player != NULL) {
        mvprintw(14, 5, "현재 골드: %d", player->gold);
        mvprintw(15, 5, "현재 덱 카드 수: %d", player->owned_deck_count);
    }

    mvprintw(18, 5, "아무 키나 누르면 다음 층으로 이동합니다.");

    refresh();
    getch();

    clear();
    refresh();
}

//스테이지 선택 함수
StageType show_stage_choice_screen(int floor, const MapFloor *map_floor)
{
    int selected;
    int ch;
    int i;

    if (map_floor == NULL || map_floor->choice_count <= 0) {
        show_current_stage_screen(floor, STAGE_ENEMY);
        return STAGE_ENEMY;
    }

    if (map_floor->choice_count == 1) {
        show_current_stage_screen(floor, map_floor->choices[0]);
        return map_floor->choices[0];
    }

    selected = 0;

    while (1) {
        clear();

        mvprintw(3, 5, "==================== 스테이지 선택 ====================");
        mvprintw(5, 5, "현재 층: %d층", floor);
        mvprintw(7, 5, "진행할 스테이지를 선택하세요.");

        for (i = 0; i < map_floor->choice_count; i++) {
            if (i == selected) {
                attron(A_REVERSE);
            }

            mvprintw(10 + i * 2, 7, "[%d] %s",
                     i + 1,
                     get_stage_type_name(map_floor->choices[i]));

            if (i == selected) {
                attroff(A_REVERSE);
            }
        }

        mvprintw(16, 5, "W/S 또는 ↑/↓ 이동, Enter 선택");
        mvprintw(17, 5, "숫자 키 1~%d로도 선택할 수 있습니다.", map_floor->choice_count);

        refresh();

        ch = getch();

        if (ch == KEY_UP || ch == 'w' || ch == 'W') {
            selected--;

            if (selected < 0) {
                selected = map_floor->choice_count - 1;
            }
        }
        else if (ch == KEY_DOWN || ch == 's' || ch == 'S') {
            selected++;

            if (selected >= map_floor->choice_count) {
                selected = 0;
            }
        }
        else if (ch == '\n' || ch == KEY_ENTER || ch == 10 || ch == 13) {
            clear();
            refresh();
            return map_floor->choices[selected];
        }
        else if (ch >= '1' && ch < '1' + map_floor->choice_count) {
            selected = ch - '1';

            clear();
            refresh();
            return map_floor->choices[selected];
        }
    }
}

static void show_run_result_screen(GameState *state, int is_clear)
{
    int selected = 0;
    int ch;
    int i;
    int row;
    long total;
    long hours;
    long minutes;
    long seconds;
    char line[256];

    const char *items[] = {
        "덱 자세히 보기",
        "유물 자세히 보기",
        "마치기"
    };

    if (state == NULL) {
        return;
    }

    while (1) {
        clear();

        if (is_clear) {
            mvprintw(1, 3, "==================== 게임 클리어 ====================");
            mvprintw(3, 5, "최종 보스를 처치했습니다!");
        } else {
            mvprintw(1, 3, "==================== 게임 오버 ====================");
            mvprintw(3, 5, "플레이어가 쓰러졌습니다.");
        }

        total = state->play_time_seconds;
        if (total < 0) {
            total = 0;
        }

        hours = total / 3600;
        minutes = (total % 3600) / 60;
        seconds = total % 60;

        mvprintw(6, 5, "------------------ 기록 ------------------");

        snprintf(line, sizeof(line), "플레이어       : %s", state->username);
        mvprintw(8, 5, "%s", line);

        mvprintw(9, 5, "플레이 시간    : %02ld:%02ld:%02ld",
                 hours, minutes, seconds);

        mvprintw(10, 5, "남은 체력      : %d / %d",
                 state->player.hp, state->player.max_hp);

        mvprintw(11, 5, "보유 골드      : %d",
                 state->player.gold);

        mvprintw(12, 5, "덱 카드 수     : %d장",
                 state->player.owned_deck_count);

        mvprintw(13, 5, "보유 유물 수   : %d개",
                 state->player.relic_count);

        if (is_clear) {
            mvprintw(15, 5, "클리어한 세이브 파일은 종료 후 삭제됩니다.");
        } else {
            mvprintw(15, 5, "사망하여 해당 세이브 파일은 삭제됩니다.");
        }

        row = 18;

        for (i = 0; i < 3; i++) {
            if (selected == i) {
                attron(A_REVERSE);
                mvprintw(row + i * 2, 7, "%s", items[i]);
                attroff(A_REVERSE);
            } else {
                mvprintw(row + i * 2, 7, "%s", items[i]);
            }
        }

        mvprintw(LINES - 3, 3, "↑ ↓ : 이동");
        mvprintw(LINES - 2, 3, "ENTER : 선택");

        refresh();

        ch = getch();

        if (ch == KEY_UP) {
            selected--;

            if (selected < 0) {
                selected = 2;
            }
        } else if (ch == KEY_DOWN) {
            selected++;

            if (selected > 2) {
                selected = 0;
            }
        } else if (ch == '\n' || ch == KEY_ENTER) {
            if (selected == 0) {
                show_card_pile_screen("덱 목록",
                                      state->player.owned_deck,
                                      state->player.owned_deck_count,
                                      0);
            } else if (selected == 1) {
                show_relic_inventory_screen(&state->player);
            } else {
                break;
            }
        } else if (ch == '1') {
            show_card_pile_screen("덱 목록",
                                  state->player.owned_deck,
                                  state->player.owned_deck_count,
                                  0);
        } else if (ch == '2') {
            show_relic_inventory_screen(&state->player);
        } else if (ch == '3' || ch == 'q' || ch == 'Q') {
            break;
        }
    }
}

void show_game_clear_screen(GameState *state)
{
    show_run_result_screen(state, 1);
}

void show_game_over_screen(GameState *state)
{
    show_run_result_screen(state, 0);
}
