#define _XOPEN_SOURCE 700
#include <locale.h>
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include "ui.h"

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
void close_ui(void) {
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

//전투화면 출력 함수

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

//임시 전투화면 출력 함수

void show_temp_battle_screen(const GameState *state)
{
    const Player *player;
    const Card *selected_card;

    const int battle_width = 90;
    const int battle_height = 28;
    const int max_display_hand = 10;
    const int card_slot_width = 18;

    int hand_count;
    int selected = 0;
    int ch;
    int i;

    if (state == NULL) {
        return;
    }

    player = &state->player;

    hand_count = player->owned_deck_count;

    if (hand_count > max_display_hand) {
        hand_count = max_display_hand;
    }

    if (hand_count <= 0) {
        while (1) {
            int start_y = (LINES - battle_height) / 3;
            int start_x = (COLS - battle_width) / 2;

            if (start_y < 0) {
                start_y = 0;
            }

            if (start_x < 0) {
                start_x = 0;
            }

            clear();

            print_battle_line(start_y + 0, start_x, battle_width);
            mvprintw(start_y + 1, start_x,
                     "Floor %-3d        Gold %-4d        Relic: 없음",
                     state->floor,
                     player->gold);
            print_battle_line(start_y + 2, start_x, battle_width);

            mvprintw(start_y + 4, start_x, "현재 표시할 카드가 없습니다.");
            mvprintw(start_y + 6, start_x, "Q 종료");

            refresh();

            ch = getch();

            if (ch == 'q' || ch == 'Q') {
                break;
            }
        }

        return;
    }

    while (1) {
        int start_y = (LINES - battle_height) / 3;
        int start_x = (COLS - battle_width) / 2;

        if (start_y < 0) {
            start_y = 0;
        }

        if (start_x < 0) {
            start_x = 0;
        }

        if (selected < 0) {
            selected = 0;
        }

        if (selected >= hand_count) {
            selected = hand_count - 1;
        }

        selected_card = &player->owned_deck[selected];

        clear();

        print_battle_line(start_y + 0, start_x, battle_width);
        mvprintw(start_y + 1, start_x,
                 "Floor %-3d        Gold %-4d        Relic: 없음",
                 state->floor,
                 player->gold);
        print_battle_line(start_y + 2, start_x, battle_width);

        mvprintw(start_y + 4, start_x, "Enemy: Slime");
        mvprintw(start_y + 5, start_x,
                 "HP 30/30   Block 0   Str 0   Weak 0   Vul 0");
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

        mvprintw(start_y + 16, start_x,
                 "%s   Cost %d",
                 selected_card->name,
                 selected_card->cost);

        mvprintw(start_y + 17, start_x,
                 "%s",
                 selected_card->description);

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
                     player->owned_deck[i].name,
                     player->owned_deck[i].cost);

            if (i == selected) {
                attroff(A_REVERSE);
            }
        }

        mvprintw(start_y + 25, start_x,
                 "← → 선택   A/D 선택   Enter 확인   Q 종료");

        refresh();

        ch = getch();

        if (ch == KEY_LEFT || ch == 'a' || ch == 'A') {
            selected--;

            if (selected < 0) {
                selected = hand_count - 1;
            }
        } else if (ch == KEY_RIGHT || ch == 'd' || ch == 'D') {
            selected++;

            if (selected >= hand_count) {
                selected = 0;
            }
        } else if (ch == '\n' || ch == KEY_ENTER) {
            mvprintw(start_y + 27, start_x,
                     "%s 선택됨. 아직 카드 사용 기능은 구현하지 않았습니다.",
                     selected_card->name);
            refresh();
            getch();
        } else if (ch == 'q' || ch == 'Q') {
            break;
        }
    }
}