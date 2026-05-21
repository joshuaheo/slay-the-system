#define _XOPEN_SOURCE 700
#include <locale.h>
#include <stdio.h>
#include <ncurses.h>
#include <string.h>
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

static void show_card_pile_screen(const char *title,const Card *cards,int count,int reverse_order);

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
BattleResult show_temp_battle_screen(GameState *state)
{
    Player *player;
    const Card *selected_card;
    BattleResult final_result = BATTLE_CONTINUE;
    BattleResult battle_result;

    Enemy enemies[1];
    int enemy_count = 1;
    int target_index = 0;

    const int battle_width = 90;
    const int battle_height = 28;
    const int max_display_hand = 10;
    const int card_slot_width = 18;

    int hand_count;
    int selected = 0;
    int ch;
    int i;

    if (state == NULL) {
        return BATTLE_CONTINUE;
    }

    player = &state->player;

    init_enemy(&enemies[0],ENEMY_SLIME);

    apply_relics_on_battle_start(player);

    while (1) {
        int start_y = (LINES - battle_height) / 3;
        int start_x = (COLS - battle_width) / 2;

        if (start_y < 0) {
            start_y = 0;
        }

        if (start_x < 0) {
            start_x = 0;
        }

        hand_count = player->hand_count;

        if (hand_count > max_display_hand) {
            hand_count = max_display_hand;
        }

        if (selected < 0) {
            selected = 0;
        }

        if (hand_count > 0 && selected >= hand_count) {
            selected = hand_count - 1;
        }

        clear();

        print_battle_line(start_y + 0, start_x, battle_width);
        mvprintw(start_y + 1, start_x,
                 "Floor %-3d        Gold %-4d        Relic: ",
                 state->floor,
                 player->gold);
                 print_relic_summary(player);
        print_battle_line(start_y + 2, start_x, battle_width);

        mvprintw(start_y + 4, start_x, "Enemy: %s", enemies[0].name);
        mvprintw(start_y + 5, start_x,
                 "HP %d/%d   Block %d   Str %d   Weak %d   Vul %d",
                 enemies[0].hp,
                 enemies[0].max_hp,
                 enemies[0].block,
                 enemies[0].strength,
                 enemies[0].weak,
                 enemies[0].vulnerable);
        mvprintw(start_y + 6, start_x,
                 "Intent: Attack 6");

        print_battle_dash(start_y + 8, start_x, battle_width);

        mvprintw(start_y + 10, start_x,
                 "Player: %s",
                 player->name);
        mvprintw(start_y + 11, start_x,
                 "HP %d/%d   Block %d   Energy %d/%d   Str %d   Weak %d   Vul %d",
                 player->hp,
                 player->max_hp,
                 player->block,
                 player->energy,
                 player->max_energy,
                 player->strength,
                 player->weak,
                 player->vulnerable);

        print_battle_line(start_y + 13, start_x, battle_width);

        mvprintw(start_y + 14, start_x, "Selected Card");

        if (hand_count > 0) {
            selected_card = &player->hand[selected];

            mvprintw(start_y + 16, start_x,
                     "%s   Cost %d",
                     selected_card->name,
                     selected_card->cost);

            mvprintw(start_y + 17, start_x,
                     "%s",
                     selected_card->description);
        } else {
            selected_card = NULL;

            mvprintw(start_y + 16, start_x,
                     "현재 손패에 카드가 없습니다.");
        }

        print_battle_line(start_y + 19, start_x, battle_width);

        mvprintw(start_y + 21, start_x, "Hand");

        for (i = 0; i < hand_count; i++) {
            int row = start_y + 22 + (i / 5);
            int col = start_x + (i % 5) * card_slot_width;

            if (i == selected) {
                attron(A_REVERSE);
            }

            mvprintw(row, col,
                     "[%d]%s(%d)",
                     i + 1,
                     player->hand[i].name,
                     player->hand[i].cost);

            if (i == selected) {
                attroff(A_REVERSE);
            }
        }
        mvprintw(start_y + 25, start_x,
         "← → 선택   A/D 선택   Enter 사용   I 인벤토리   E 턴 종료   Q 종료");

        refresh();

        ch = getch();

        if (ch == KEY_LEFT || ch == 'a' || ch == 'A') {
            if (hand_count > 0) {
                selected--;

                if (selected < 0) {
                    selected = hand_count - 1;
                }
            }
        } else if (ch == KEY_RIGHT || ch == 'd' || ch == 'D') {
            if (hand_count > 0) {
                selected++;

                if (selected >= hand_count) {
                    selected = 0;
                }
            }
        } else if (ch == '\n' || ch == KEY_ENTER) {
            if (hand_count <= 0) {
                mvprintw(start_y + 27, start_x,
                         "사용할 카드가 없습니다.");
                refresh();
                getch();
            } else if (enemies[0].hp <= 0) {
                mvprintw(start_y + 27, start_x,
                         "이미 적을 처치했습니다.");
                refresh();
                getch();
            } else if (player->hp <= 0) {
                mvprintw(start_y + 27, start_x,
                         "플레이어가 쓰러져 카드를 사용할 수 없습니다.");
                refresh();
                getch();
            } else if (play_card(player, enemies, enemy_count, selected, target_index)) {
                battle_result = check_battle_result(player, enemies, enemy_count);
                if (battle_result != BATTLE_CONTINUE) {    
                    final_result = battle_result;
                    show_battle_result_message(battle_result);
                    break;
                }
                hand_count = player->hand_count;

                if (hand_count > max_display_hand) {
                    hand_count = max_display_hand;
                }

                if (hand_count <= 0) {
                    selected = 0;
                } else if (selected >= hand_count) {
                    selected = hand_count - 1;
                }
            } 
            else {
                mvprintw(start_y + 27, start_x,
                         "카드를 사용할 수 없습니다. 에너지 또는 대상 상태를 확인하세요.");
                refresh();
                getch(); 
	    }
	} else if (ch == 'i' || ch == 'I') {
    clear();

    mvprintw(3, 5, "[ INVENTORY ]");
    mvprintw(5, 5, "1. Draw Deck");
    mvprintw(6, 5, "2. Discard Pile");
    mvprintw(7, 5, "3. Exhaust Pile");
    mvprintw(8, 5, "4. Relics");

    refresh();

    int sub = getch();

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
} else if (ch == 'e' || ch == 'E') {
            if (player->hp <= 0) {
                mvprintw(start_y + 27, start_x,
                         "플레이어가 쓰러져 턴을 종료할 수 없습니다.");
                refresh();
                getch();
            } else if (enemies[0].hp <= 0) {
                mvprintw(start_y + 27, start_x,
                         "이미 적을 처치했습니다.");
                refresh();
                getch();
            } else {
                discard_hand(player);
                enemies_take_turn(enemies,enemy_count,player);
                battle_result = check_battle_result(player, enemies, enemy_count);

                if (battle_result != BATTLE_CONTINUE) {
                    final_result = battle_result;
                    show_battle_result_message(battle_result);
                    break;
                }
                decrease_turn_statuses(player,enemies,enemy_count);

                player->block = 0;
                player->energy = player->max_energy;

                draw_cards(player, 5);

                hand_count = player->hand_count;

                if (hand_count > max_display_hand) {
                    hand_count = max_display_hand;
                }

                if (hand_count <= 0) {
                    selected = 0;
                } else if (selected >= hand_count) {
                    selected = hand_count - 1;
                }
            }
        } else if (ch == 'q' || ch == 'Q') {
            final_result = BATTLE_CONTINUE;
            break;
        }
    }
    return final_result;
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
void show_battle_reward_screen(GameState *state)
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

    if (state == NULL) {
        return;
    }

    player = &state->player;
    selected = 0;
    card_width = 26;

    gold_reward = generate_gold_reward(20, 30);
    player->gold += gold_reward;

    generate_card_rewards(rewards, CARD_REWARD_COUNT);

    while (1) {
        clear();

        start_y = 2;
        start_x = 4;

        mvprintw(start_y, start_x, "전투 승리!");
        mvprintw(start_y + 2, start_x, "획득 골드: %d", gold_reward);
        mvprintw(start_y + 3, start_x, "현재 골드: %d", player->gold);

        mvprintw(start_y + 5, start_x, "카드 보상을 선택하세요.");
        mvprintw(start_y + 6, start_x, "←/→ 또는 A/D: 선택 이동");
        mvprintw(start_y + 7, start_x, "Enter 또는 1/2/3: 선택");
        mvprintw(start_y + 8, start_x, "S 또는 0: 카드 보상 넘기기");

        for (i = 0; i < CARD_REWARD_COUNT; i++) {
            int card_x;

            card_x = start_x + i * 32;

            if (i == selected) {
                mvprintw(start_y + 11, card_x, ">");
            } else {
                mvprintw(start_y + 11, card_x, " ");
            }

            mvprintw(start_y + 11, card_x + 2, "[%d]", i + 1);
            mvprintw(start_y + 12, card_x + 2, "%s", rewards[i].name);
            mvprintw(start_y + 13, card_x + 2, "비용: %d", rewards[i].cost);

            print_wrapped_text(start_y + 15,
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
    int cols;
    const char *screen_title;
    int desc_x;

    if (relic == NULL) {
        return;
    }

    getmaxyx(stdscr, rows, cols);

    screen_title = title != NULL ? title : "유물 획득";
    desc_x = (cols - (int)strlen(relic->description)) / 2;
    if (desc_x < 0) {
        desc_x = 0;
    }

    clear();

    mvprintw(rows / 2 - 5, (cols - (int)strlen(screen_title)) / 2, "%s", screen_title);
    mvprintw(rows / 2 - 3, (cols - 24) / 2, "새로운 유물을 얻었습니다!");
    mvprintw(rows / 2 - 1, (cols - (int)strlen(relic->name) - 4) / 2, "[ %s ]", relic->name);
    mvprintw(rows / 2 + 1, desc_x, "%s", relic->description);
    mvprintw(rows / 2 + 4, (cols - 30) / 2, "아무 키나 누르면 진행합니다.");

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

        if (ch == KEY_UP) {
            if (selected > 0) {
                selected--;
            }
        } else if (ch == KEY_DOWN) {
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
