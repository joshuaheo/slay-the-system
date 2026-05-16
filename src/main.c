#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "type.h"
#include "ui.h"
#include "login.h"
#include "save.h"
#include "game.h"
#include "card.h"

int main(void) {
    srand((unsigned int)time(NULL) ^ (unsigned int)getpid());
    MenuChoice choice;
    char username[MAX_NAME_LEN];
    GameState state;
    int has_save;
    BattleResult battle_result;

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

       prepare_battle_deck(&state.player);
       battle_result = show_temp_battle_screen(&state);

       if (battle_result == BATTLE_WIN) {
        show_battle_reward_screen(&state);
    }
        close_ui();
    } else if (choice == MENU_EXIT) {
        close_ui();
        printf("종료 선택됨\n");
    }

    return 0;
}