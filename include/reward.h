#ifndef REWARD_H
#define REWARD_H

#include "type.h"

#define CARD_REWARD_COUNT 3

void generate_card_rewards(Card rewards[], int reward_count);
int generate_gold_reward(int min_gold, int max_gold);

#endif