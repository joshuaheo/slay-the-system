#ifndef SHOP_H
#define SHOP_H

#include "type.h"

typedef enum {
    SHOP_BUY_OK,
    SHOP_BUY_INVALID,
    SHOP_BUY_SOLD,
    SHOP_BUY_NOT_ENOUGH_GOLD,
    SHOP_BUY_NEED_CARD_SELECT,
    SHOP_BUY_DECK_FULL,
    SHOP_BUY_RELIC_FULL,
    SHOP_BUY_REMOVE_UNAVAILABLE
} ShopBuyResult;

void generate_shop(const Player *player, Shop *shop);
ShopBuyResult buy_shop_item(Player *player, ShopItem *item);
ShopBuyResult buy_shop_remove_card(Player *player, ShopItem *item, int deck_index);

#endif