#include <stdio.h>
#include "type.h"
#include "ui.h"
#include "login.h"

int main(void) {
    MenuChoice choice;
    char username[MAX_NAME_LEN];
    int has_save;

    init_ui();

    choice = show_start_screen();

    if (choice == MENU_START_GAME) {
        while (1) {
            show_login_screen(username, MAX_NAME_LEN);

            if (is_valid_username(username)) {
                break;
            }

            show_invalid_username_screen();
        }

        has_save = save_file_exists(username);

        close_ui();

        if (has_save) {
            printf("기존 세이브 파일 발견: %s\n", username);
        } else {
            printf("새 계정으로 시작: %s\n", username);
        }
    } else if (choice == MENU_EXIT) {
        close_ui();
        printf("종료 선택됨\n");
    }

    return 0;
}