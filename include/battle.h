#ifndef BATTLE_H
#define BATTLE_H

#include "type.h"

typedef enum {
    BATTLE_CONTINUE,
    BATTLE_WIN,
    BATTLE_LOSE
} BattleResult;

int play_card(Player *player, Enemy enemies[], int enemy_count, int hand_index, int target_index);
void decrease_turn_statuses(Player *player, Enemy enemies[], int enemy_count);

int are_all_enemies_dead(Enemy enemies[], int enemy_count);
BattleResult check_battle_result(Player *player, Enemy enemies[], int enemy_count);

#endif