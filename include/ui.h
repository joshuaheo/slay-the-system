#ifndef UI_H
#define UI_H

#include "type.h"
#include "battle.h"
typedef enum {
    MENU_START_GAME = 1,
    MENU_EXIT = 2
} MenuChoice;

void init_ui(void);
void close_ui(void);
MenuChoice show_start_screen(void);
int show_login_screen(char *username, int size);
void show_invalid_username_screen(void);

BattleResult show_temp_battle_screen(GameState *state, StageType stage);
void show_battle_reward_screen(GameState *state);
void show_current_stage_screen(int floor, StageType stage);

int show_rest_choice_screen(const Player *player);
int show_remove_card_screen(const Player *player);
void show_rest_result_screen(int healed, const Player *player);
void show_card_removed_screen(const Card *card);
void show_card_remove_unavailable_screen(void);

void show_relic_obtained_screen(const char *title, const Relic *relic);
void show_no_relic_available_screen(void);

void show_deck_screen(const Player *player);
void show_relic_inventory_screen(const Player *player);

void show_card_detail_screen(const Card *card);
void show_relic_detail_screen(const Relic *relic);

int show_shop_screen(GameState *state, Shop *shop);
#endif
