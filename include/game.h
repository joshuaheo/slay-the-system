#ifndef GAME_H
#define GAME_H
#include "battle.h"
#include "type.h"

void init_new_game(GameState *state, const char *username);

void cleanup_after_battle(Player *player);
int handle_battle_win(GameState *state, StageType stage);
int handle_battle_lose(GameState *state);

int run_current_stage(GameState *state);

void start_play_timer(void);
void update_play_time(GameState *state);

#endif
