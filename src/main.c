#include <stdio.h>
#include <ncurses.h>
#include "type.h"
#include "ui.h"

int main(void) {
    MenuChoice choice;

    init_ui();

    choice = show_start_screen();

    close_ui();

    if (choice == MENU_START_GAME) {
        printf("게임시작 선택됨\n");
    } else if (choice == MENU_EXIT) {
        printf("종료 선택됨\n");
    }

    return 0;
}