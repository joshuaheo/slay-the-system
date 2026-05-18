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
#include "reward.h"
#include "map.h"

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

//전투화면 부분

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

    init_slime(&enemies[0]);

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
                 "Floor %-3d        Gold %-4d        Relic: 없음",
                 state->floor,
                 player->gold);
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
                 "← → 선택   A/D 선택   Enter 사용   E 턴 종료   Q 종료");

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