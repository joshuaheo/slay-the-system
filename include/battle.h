#ifndef BATTLE_H
#define BATTLE_H

#include "type.h"

int play_card(Player *player, Enemy enemies[], int enemy_count, int hand_index, int target_index);
void decrease_turn_statuses(Player *player, Enemy enemies[], int enemy_count);

#endif