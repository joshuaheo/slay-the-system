#include <string.h>
#include "game.h"
#include "card.h"
#include "ui.h"
#include "save.h"
#include "map.h"
#include "relic.h"
#include "player.h"
#include "shop.h"
#include "event.h"

static int run_chest_stage(GameState *state);
static int run_rest_stage(GameState *state);

// Player에 대한 정보를 게임 시작 상태로 초기화하는 함수
void init_new_game(GameState *state, const char *username) {
    Player *player;

    if (state == NULL || username == NULL) {
        return;
    }

    memset(state, 0, sizeof(GameState));

    strncpy(state->username, username, MAX_NAME_LEN - 1);
    state->username[MAX_NAME_LEN - 1] = '\0';

    state->floor = 1;

    player = &state->player;

    strncpy(player->name, username, MAX_NAME_LEN - 1);
    player->name[MAX_NAME_LEN - 1] = '\0';

    player->max_hp = 80;
    player->hp = 80;
    player->block = 0;

    player->strength = 0;
    player->weak = 0;
    player->vulnerable = 0;

    init_starting_deck(player);

    player->draw_count = 0;
    player->hand_count = 0;
    player->discard_count = 0;
    player->exhaust_count = 0;

    player->gold = 999;

    player->relic_count = 0;
    add_relic_to_player(player, get_relic_from_pool(0));

    player->max_energy = 3;
    player->energy = 3;
}

//배틀 이후 플레이어 상태를 초기화해주는 함수
void cleanup_after_battle(Player *player) {
    if (player == NULL) {
        return;
    }

    player->block = 0;

    player->strength = 0;
    player->weak = 0;
    player->vulnerable = 0;

    player->draw_count = 0;
    player->hand_count = 0;
    player->discard_count = 0;
    player->exhaust_count = 0;

    player->energy = player->max_energy;
}

//상점 스테이지 실행 함수
static int run_shop_stage(GameState *state)
{
    Shop shop;

    if (state == NULL) {
        return 0;
    }

    generate_shop(&state->player, &shop);

    if (!show_shop_screen(state, &shop)) {
        return 0;
    }

    state->floor++;

    if (!save_game(state)) {
        return 0;
    }

    return 1;
}

//전투에서 이긴 후 흐름을 진행하는 함수
int handle_battle_win(GameState *state) {
    if (state == NULL) {
        return 0;
    }
    apply_relics_on_battle_win(&state->player);

    show_battle_reward_screen(state);

    cleanup_after_battle(&state->player);
    
    state->floor++;

    if (!save_game(state)) {
        return 0;
    }

    return 1;
}

//전투에서 진 후 흐름을 진행하는 함수
int handle_battle_lose(GameState *state) {
    if (state == NULL) {
        return 0;
    }

    delete_save_file(state->username);

    return 1;
}

//전투 결과 흐름을 판정하는 함수
int handle_battle_result(GameState *state, BattleResult result) {
    if (state == NULL) {
        return 0;
    }

    if (result == BATTLE_WIN) {
        return handle_battle_win(state);
    }

    if (result == BATTLE_LOSE) {
        return handle_battle_lose(state);
    }

    return 1;
}

//현재 스테이지 타입에 따라 작동하는 함수
int run_current_stage(GameState *state) {
    StageType stage;
    BattleResult battle_result;

    if (state == NULL) {
        return 0;
    }

    if (state->floor > MAX_FLOOR) {
        return 0;
    }

    stage = get_default_stage_type(state->floor);

    show_current_stage_screen(state->floor, stage);

    switch (stage) {
    case STAGE_ENEMY:
    case STAGE_ELITE:
    case STAGE_BOSS:
        prepare_battle_deck(&state->player);
        battle_result = show_temp_battle_screen(state, stage);

        if (battle_result == BATTLE_WIN) {
            if (stage == STAGE_BOSS || state->floor == MAX_FLOOR) {
                cleanup_after_battle(&state->player);
                delete_save_file(state->username);
                return 0;
            }

            return handle_battle_win(state);
        }

        if (battle_result == BATTLE_LOSE) {
            handle_battle_lose(state);
            return 0;
        }

        return 0;

    case STAGE_REST:
        return run_rest_stage(state);
    case STAGE_SHOP:
        return run_shop_stage(state);
    case STAGE_CHEST:
        return run_chest_stage(state);
    case STAGE_EVENT:
        return run_event_stage(state);

        return 1;

    default:
        return 0;
    }
}

//휴식 스테이지 실행하는 함수
static int run_rest_stage(GameState *state)
{
    int choice;
    int heal_amount;
    int healed;
    int remove_index;
    Card removed_card;

    if (state == NULL) {
        return 0;
    }

    while (1) {
        choice = show_rest_choice_screen(&state->player);

        if (choice == 1) {
            heal_amount = state->player.max_hp * 30 / 100;
            if (heal_amount <= 0) {
                heal_amount = 1;
            }

            healed = heal_player(&state->player, heal_amount);
            show_rest_result_screen(healed, &state->player);

            state->floor++;

            if (!save_game(state)) {
                return 0;
            }

            return 1;
        }

        if (choice == 2) {
            if (!can_remove_card_from_deck(&state->player)) {
                show_card_remove_unavailable_screen();
                continue;
            }

            remove_index = show_remove_card_screen(&state->player);

            if (remove_index < 0) {
                continue;
            }

            removed_card = state->player.owned_deck[remove_index];

            if (!remove_card_from_deck(&state->player, remove_index)) {
                return 0;
            }

            show_card_removed_screen(&removed_card);

            state->floor++;

            if (!save_game(state)) {
                return 0;
            }

            return 1;
        }
    }
}

//보물 스테이지 실행하는 함수
static int run_chest_stage(GameState *state)
{
    Relic relic;

    if (state == NULL) {
        return 0;
    }

    if (grant_random_standard_relic(&state->player, &relic)) {
        show_relic_obtained_screen("보물 상자", &relic);
    } else {
        show_no_relic_available_screen();
    }

    state->floor++;
    save_game(state);

    return 1;
}
