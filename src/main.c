#include <stdio.h>
#include "type.h"
#include "ui.h"
#include "login.h"
#include "save.h"
#include "game.h"

int main(void) {
    MenuChoice choice;
    char username[MAX_NAME_LEN];
    GameState state;
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

        if (has_save) {
            if (!load_game(username, &state)) {
                close_ui();
                printf("세이브 파일 불러오기 실패\n");
                return 1;
            }
        } else {
            init_new_game(&state, username);

            if (!save_game(&state)) {
                close_ui();
                printf("새 세이브 파일 생성 실패\n");
                return 1;
            }
        }

        close_ui();

        if (has_save) {
            printf("기존 세이브 파일 불러오기 성공\n");
        } else {
            printf("새 세이브 파일 생성 성공\n");
        }

        printf("username: %s\n", state.username);
        printf("floor: %d\n", state.floor);
        printf("hp: %d / %d\n", state.player.hp, state.player.max_hp);
        printf("gold: %d\n", state.player.gold);
        printf("owned deck count: %d\n", state.player.owned_deck_count);
    } else if (choice == MENU_EXIT) {
        close_ui();
        printf("종료 선택됨\n");
    }

    return 0;
}