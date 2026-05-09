#include <stdio.h>
#include "type.h"
#include "ui.h"

int main(void) {
    MenuChoice choice;
    char username[MAX_NAME_LEN];

    init_ui();

    choice = show_start_screen();

    if (choice == MENU_START_GAME) {
        show_login_screen(username, MAX_NAME_LEN);
        close_ui();

        printf("입력한 아이디: %s\n", username);
    } else if (choice == MENU_EXIT) {
        close_ui();
        printf("종료 선택됨\n");
    }

    return 0;
}