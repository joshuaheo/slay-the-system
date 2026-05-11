#ifndef CARD_H
#define CARD_H

#include "type.h"

int get_card_pool_count(void);
Card get_card_from_pool(int index);
int add_card_to_deck(Player *player, Card card);
void init_starting_deck(Player *player);

#endif