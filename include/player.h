#ifndef PLAYER_H
#define PLAYER_H

#include "type.h"

int heal_player(Player *player, int amount);
int can_remove_card_from_deck(const Player *player);
int remove_card_from_deck(Player *player, int index);

#endif