#include <string.h>
#include "game.h"
#include "card.h"
#include "ui.h"
#include "save.h"

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

    player->gold = 99;

    player->relic_count = 0;

    player->max_energy = 3;
    player->energy = 3;
}


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

int handle_battle_win(GameState *state) {
    if (state == NULL) {
        return 0;
    }

    show_battle_reward_screen(state);

    cleanup_after_battle(&state->player);

    if (!save_game(state)) {
        return 0;
    }

    return 1;
}

int handle_battle_lose(GameState *state) {
    if (state == NULL) {
        return 0;
    }

    delete_save_file(state->username);

    return 1;
}

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