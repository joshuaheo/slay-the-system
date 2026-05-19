#ifndef RELIC_H
#define RELIC_H

#include "type.h"

int get_relic_pool_count(void);
Relic get_relic_from_pool(int index);

int has_relic(const Player *player, RelicId id);
int add_relic_to_player(Player *player, Relic relic);

int get_random_available_relic_by_rarity(
    const Player *player,
    RelicRarity rarity,
    Relic *out_relic
);

int get_random_available_standard_relic(
    const Player *player,
    Relic *out_relic
);

int grant_random_standard_relic(Player *player, Relic *out_relic);

#endif