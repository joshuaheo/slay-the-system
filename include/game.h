#ifndef GAME_H
#define GAME_H
#include "battle.h"
#include "type.h"

void init_new_game(GameState *state, const char *username);

void cleanup_after_battle(Player *player);
int handle_battle_win(GameState *state);
int handle_battle_lose(GameState *state);
int handle_battle_result(GameState *state, BattleResult result);

int run_current_stage(GameState *state);

typedef struct {
    int enemies_defeated;
    int battles_won;

    int total_damage_dealt;
    int total_damage_taken;

    int total_gold_earned;
    int cards_played;
} Statistics;

extern Statistics stats;
#endif
