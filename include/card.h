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
Card create_greed_card(void);

//카드 인덱스
enum {
    CARD_INDEX_STRIKE = 0,
    CARD_INDEX_BASH,
    CARD_INDEX_DEFEND,
    CARD_INDEX_SWORD_BOOMERANG,
    CARD_INDEX_TWIN_STRIKE,
    CARD_INDEX_BREAKTHROUGH,
    CARD_INDEX_THUNDERCLAP,
    CARD_INDEX_IRON_WAVE,
    CARD_INDEX_POMMEL_STRIKE,
    CARD_INDEX_BLOODLETTING,
    CARD_INDEX_TREMBLE, //index 10
    CARD_INDEX_SHRUG_IT_OFF,
    CARD_INDEX_BLOOD_WALL,
    CARD_INDEX_HEMOKINESIS,
    CARD_INDEX_UPPERCUT,
    CARD_INDEX_BLUDGEON,
    CARD_INDEX_TAUNT,
    CARD_INDEX_INFLAME,
    CARD_INDEX_OFFERING,
    CARD_INDEX_IMPERVIOUS,
    CARD_INDEX_ULTIMATE_STRIKE, //index 20
    CARD_INDEX_ULTIMATE_DEFEND, //index 21
    CARD_INDEX_FIEND_FIRE, //index 22
    CARD_INDEX_CRIMSON_MANTLE, //index 23
    CARD_INDEX_PYRE, //index 24
    CARD_INDEX_DEMON_FORM //index 25
};

#endif