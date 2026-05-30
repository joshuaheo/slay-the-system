#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/random.h>
#include "type.h"
#include "ui.h"
#include "login.h"
#include "save.h"
#include "game.h"

volatile sig_atomic_t g_quit_requested = 0;

//핸들러 함수
static void handle_sigint(int sig)
{
    (void)sig;
    g_quit_requested = 1;
}

//시그널 콜 호출시 나오는 메시지
static void exit_safely_after_sigint(void)
{
    close_ui();
    printf("Ctrl+C 입력으로 안전 종료되었습니다.\n");
    printf("전투 중 상태는 저장되지 않으며, 마지막 자동저장 지점부터 재개됩니다.\n");
}

//랜덤 시드를 만드는 함수 (getrandom()사용, 실패시 time * getpid 가능)
static void seed_random_once(void)
{
    static int seeded = 0;
    unsigned int seed;
    ssize_t result;

    if (seeded) {
        return;
    }

    result = getrandom(&seed, sizeof(seed), 0);

    if (result == (ssize_t)sizeof(seed)) {
        srand(seed);
    } else {
        srand((unsigned int)time(NULL) ^ (unsigned int)getpid());
    }

    seeded = 1;
}

int main(void) {
    MenuChoice choice;
    char username[MAX_NAME_LEN];
    GameState state;
    int slot;
    SaveSlotAction action;
    signal(SIGINT, handle_sigint);
    seed_random_once();

    init_ui();

    while (1) {
        if (g_quit_requested) {
            exit_safely_after_sigint();
            return 0;
        }
        choice = show_start_screen();

        if (choice == MENU_EXIT) {
            break;
        }

        if (choice == MENU_START_GAME) {
            while (1) {
                show_login_screen(username, MAX_NAME_LEN);

                if (is_valid_username(username)) {
                    break;
                }

                show_invalid_username_screen();
            }

            while (1) {
                slot = show_save_slot_screen(username);

                if (slot == 0) {
                    break;
                }

                if (save_file_exists(username, slot)) {
                    action = show_save_slot_action_screen(username, slot);

                    if (action == SAVE_ACTION_BACK) {
                        continue;
                    }

                    if (action == SAVE_ACTION_LOAD) {
                        if (!load_game(username, slot, &state)) {
                            close_ui();
                            printf("세이브 파일 불러오기 실패\n");
                            return 1;
                        }
                    } else if (action == SAVE_ACTION_NEW) {
                        init_new_game(&state, username);
                        state.save_slot = slot;

                        if (!save_game(&state)) {
                            close_ui();
                            printf("새 세이브 파일 생성 실패\n");
                            return 1;
                        }
                    }
                } else {
                    init_new_game(&state, username);
                    state.save_slot = slot;

                    if (!save_game(&state)) {
                        close_ui();
                        printf("새 세이브 파일 생성 실패\n");
                        return 1;
                    }
                }
                start_play_timer();
                while (state.floor <= MAX_FLOOR) {
                    if (g_quit_requested) {
                        exit_safely_after_sigint();
                        return 0;
                    }

                    if (!run_current_stage(&state)) {
                        if (g_quit_requested) {
                            exit_safely_after_sigint();
                            return 0;
                        }
                        break;
                    }
                }

                if (g_quit_requested) {
                    exit_safely_after_sigint();
                    return 0;
                }
                break;
            }
        }
    }

    close_ui();
    printf("종료 선택됨\n");

    return 0;
}
