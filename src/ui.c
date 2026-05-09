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