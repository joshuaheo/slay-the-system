#ifndef CARD_H
#define CARD_H

#include "type.h"

int get_card_pool_count(void);
Card get_card_from_pool(int index);
int add_card_to_deck(Player *player, Card card);
void init_starting_deck(Player *player);

void prepare_battle_deck(Player *player);
void shuffle_draw_pile(Player *player);
void draw_cards(Player *player, int count);
void discard_hand(Player *player);
Card create_goop_card(void);
int add_card_to_discard(Player *player, Card card);
Card create_wound_card(void);
#endif