#include <string.h>
#include "game.h"

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

    player->owned_deck_count = 10;
    player->draw_count = 0;
    player->hand_count = 0;
    player->discard_count = 0;
    player->exhaust_count = 0;

    player->gold = 99;

    player->relic_count = 0;

    player->max_energy = 3;
    player->energy = 3;
}
