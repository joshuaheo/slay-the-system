#include <stdlib.h>
#include <string.h>

#include "shop.h"
#include "card.h"
#include "relic.h"

//랜덤 숫자 출력 함수
static int rand_between(int min, int max)
{
    return min + rand() % (max - min + 1);
}

//희귀도에 따른 카드 가격 선정 함수
static int get_card_price(CardRarity rarity)
{
    switch (rarity) {
    case CARD_COMMON:
        return rand_between(45, 55);
    case CARD_UNCOMMON:
        return rand_between(68, 82);
    case CARD_RARE:
        return rand_between(135, 165);
    case CARD_START:
    default:
        return rand_between(45, 55);
    }
}

//희귀도에 따른 유물 가격 선정 함수
static int get_relic_price(RelicRarity rarity)
{
    switch (rarity) {
    case RELIC_COMMON:
        return rand_between(143, 157);
    case RELIC_SHOP:
        return rand_between(143, 157);
    case RELIC_UNCOMMON:
        return rand_between(238, 262);
    case RELIC_RARE:
        return rand_between(285, 315);
    default:
        return rand_between(143, 157);
    }
}

//드랍 유물인지 판정하는 함수
static int is_drop_relic_rarity(RelicRarity rarity)
{
    return rarity == RELIC_COMMON ||
           rarity == RELIC_UNCOMMON ||
           rarity == RELIC_RARE;
}

//상점 유물인지 판정하는 함수
static int is_shop_relic_rarity(RelicRarity rarity)
{
    return rarity == RELIC_SHOP;
}

//중복 유물인지 판정하는 함수
static int shop_has_relic(const Shop *shop, RelicId id)
{
    int i;

    if (shop == NULL) {
        return 0;
    }

    for (i = 0; i < shop->item_count; i++) {
        if (shop->items[i].type == SHOP_ITEM_RELIC &&
            shop->items[i].available &&
            shop->items[i].relic.id == id) {
            return 1;
        }
    }

    return 0;
}

//타입에 따른 랜덤카드를 가져오는 함수
static int get_random_card_by_type(CardType type, Card *out_card)
{
    int candidates[256];
    int candidate_count = 0;
    int pool_count;
    int i;
    Card card;

    if (out_card == NULL) {
        return 0;
    }

    pool_count = get_card_pool_count();

    for (i = 0; i < pool_count && candidate_count < 256; i++) {
        card = get_card_from_pool(i);

        if (card.type == type) {
            candidates[candidate_count] = i;
            candidate_count++;
        }
    }

    if (candidate_count <= 0) {
        return 0;
    }

    *out_card = get_card_from_pool(candidates[rand() % candidate_count]);
    return 1;
}

//상점유물인지 드랍유물인지에 따른 랜던 유물을 가져오는 함수
static int get_random_relic_by_filter(const Player *player, const Shop *shop, int (*filter)(RelicRarity), Relic *out_relic)
{
    int candidates[256];
    int candidate_count = 0;
    int pool_count;
    int i;
    Relic relic;

    if (player == NULL || shop == NULL || filter == NULL || out_relic == NULL) {
        return 0;
    }

    pool_count = get_relic_pool_count();

    for (i = 0; i < pool_count && candidate_count < 256; i++) {
        relic = get_relic_from_pool(i);

        if (filter(relic.rarity) &&
            !has_relic(player, relic.id) &&
            !shop_has_relic(shop, relic.id)) {
            candidates[candidate_count] = i;
            candidate_count++;
        }
    }

    if (candidate_count <= 0) {
        return 0;
    }

    *out_relic = get_relic_from_pool(candidates[rand() % candidate_count]);
    return 1;
}

//상점 아이템 설정 함수
static void init_shop_item(ShopItem *item)
{
    if (item == NULL) {
        return;
    }

    memset(item, 0, sizeof(ShopItem));
    item->type = SHOP_ITEM_EMPTY;
    item->available = 0;
    item->sold = 0;
    item->price = 0;
    item->original_price = 0;
    item->discounted = 0;
}

//카드 상품 추가 함수
static void add_card_item(Shop *shop, Card card)
{
    ShopItem *item;
    int price;

    if (shop == NULL || shop->item_count >= SHOP_ITEM_COUNT) {
        return;
    }

    item = &shop->items[shop->item_count];
    init_shop_item(item);

    price = get_card_price(card.rarity);

    item->type = SHOP_ITEM_CARD;
    item->card = card;
    item->price = price;
    item->original_price = price;
    item->available = 1;

    shop->item_count++;
}

//유물 상품 추가 함수
static void add_relic_item(Shop *shop, Relic relic)
{
    ShopItem *item;
    int price;

    if (shop == NULL || shop->item_count >= SHOP_ITEM_COUNT) {
        return;
    }

    item = &shop->items[shop->item_count];
    init_shop_item(item);

    price = get_relic_price(relic.rarity);

    item->type = SHOP_ITEM_RELIC;
    item->relic = relic;
    item->price = price;
    item->original_price = price;
    item->available = 1;

    shop->item_count++;
}

//카드 제거 상품 추가 함수
static void add_remove_card_item(Shop *shop)
{
    ShopItem *item;

    if (shop == NULL || shop->item_count >= SHOP_ITEM_COUNT) {
        return;
    }

    item = &shop->items[shop->item_count];
    init_shop_item(item);

    item->type = SHOP_ITEM_REMOVE_CARD;
    item->price = SHOP_REMOVE_PRICE;
    item->original_price = SHOP_REMOVE_PRICE;
    item->available = 1;

    shop->item_count++;
}

//카드에 할인을 적용하는 함수
static void apply_card_discounts(Shop *shop)
{
    int card_indices[SHOP_CARD_COUNT];
    int count = 0;
    int first;
    int second;
    int i;
    ShopItem *item;

    if (shop == NULL) {
        return;
    }

    for (i = 0; i < shop->item_count; i++) {
        if (shop->items[i].type == SHOP_ITEM_CARD && shop->items[i].available) {
            card_indices[count] = i;
            count++;
        }
    }

    if (count <= 0) {
        return;
    }

    first = rand() % count;

    item = &shop->items[card_indices[first]];
    item->discounted = 1;
    item->price = item->original_price / 2;

    if (count <= 1) {
        return;
    }

    do {
        second = rand() % count;
    } while (second == first);

    item = &shop->items[card_indices[second]];
    item->discounted = 1;
    item->price = item->original_price / 2;
}

//유물에 할인을 적용하는 함수
static void apply_relic_discount(Shop *shop)
{
    int relic_indices[SHOP_RELIC_COUNT];
    int count = 0;
    int selected;
    int i;
    ShopItem *item;

    if (shop == NULL) {
        return;
    }

    for (i = 0; i < shop->item_count; i++) {
        if (shop->items[i].type == SHOP_ITEM_RELIC && shop->items[i].available) {
            relic_indices[count] = i;
            count++;
        }
    }

    if (count <= 0) {
        return;
    }

    selected = rand() % count;

    item = &shop->items[relic_indices[selected]];
    item->discounted = 1;
    item->price = item->original_price / 2;
}

//상점 전체 함수
void generate_shop(const Player *player, Shop *shop)
{
    Card card;
    Relic relic;
    int i;

    if (shop == NULL) {
        return;
    }

    memset(shop, 0, sizeof(Shop));

    for (i = 0; i < SHOP_ITEM_COUNT; i++) {
        init_shop_item(&shop->items[i]);
    }

    if (get_random_card_by_type(CARD_ATTACK, &card)) {
        add_card_item(shop, card);
    }

    if (get_random_card_by_type(CARD_ATTACK, &card)) {
        add_card_item(shop, card);
    }

    if (get_random_card_by_type(CARD_SKILL, &card)) {
        add_card_item(shop, card);
    }

    if (get_random_card_by_type(CARD_SKILL, &card)) {
        add_card_item(shop, card);
    }

    if (get_random_card_by_type(CARD_POWER, &card)) {
        add_card_item(shop, card);
    }

    apply_card_discounts(shop);

    if (player != NULL &&
        get_random_relic_by_filter(player, shop, is_drop_relic_rarity, &relic)) {
        add_relic_item(shop, relic);
    }

    if (player != NULL &&
        get_random_relic_by_filter(player, shop, is_shop_relic_rarity, &relic)) {
        add_relic_item(shop, relic);
    } else if (player != NULL &&
               get_random_relic_by_filter(player, shop, is_drop_relic_rarity, &relic)) {
        add_relic_item(shop, relic);
    }

    apply_relic_discount(shop);

    add_remove_card_item(shop);
}

//플레이어 덱에 카드 추가 함수
static int add_card_to_player_deck(Player *player, Card card)
{
    if (player == NULL) {
        return 0;
    }

    if (player->owned_deck_count >= MAX_DECK_SIZE) {
        return 0;
    }

    player->owned_deck[player->owned_deck_count] = card;
    player->owned_deck_count++;

    return 1;
}

//상점 아이템을 사는 함수
ShopBuyResult buy_shop_item(Player *player, ShopItem *item)
{
    if (player == NULL || item == NULL || !item->available) {
        return SHOP_BUY_INVALID;
    }

    if (item->sold) {
        return SHOP_BUY_SOLD;
    }

    if (player->gold < item->price) {
        return SHOP_BUY_NOT_ENOUGH_GOLD;
    }

    switch (item->type) {
    case SHOP_ITEM_CARD:
        if (player->owned_deck_count >= MAX_DECK_SIZE) {
            return SHOP_BUY_DECK_FULL;
        }

        player->gold -= item->price;

        if (!add_card_to_player_deck(player, item->card)) {
            return SHOP_BUY_DECK_FULL;
        }

        item->sold = 1;
        return SHOP_BUY_OK;

    case SHOP_ITEM_RELIC:
        if (player->relic_count >= MAX_RELICS) {
            return SHOP_BUY_RELIC_FULL;
        }

        player->gold -= item->price;

        if (!add_relic_to_player(player, item->relic)) {
            return SHOP_BUY_INVALID;
        }

        item->sold = 1;
        return SHOP_BUY_OK;

    case SHOP_ITEM_REMOVE_CARD:
        return SHOP_BUY_NEED_CARD_SELECT;

    default:
        return SHOP_BUY_INVALID;
    }
}

//카드 제거를 구매한 경우 작동하는 함수
ShopBuyResult buy_shop_remove_card(Player *player, ShopItem *item, int deck_index)
{
    int i;

    if (player == NULL || item == NULL || !item->available) {
        return SHOP_BUY_INVALID;
    }

    if (item->sold) {
        return SHOP_BUY_SOLD;
    }

    if (item->type != SHOP_ITEM_REMOVE_CARD) {
        return SHOP_BUY_INVALID;
    }

    if (player->gold < item->price) {
        return SHOP_BUY_NOT_ENOUGH_GOLD;
    }

    if (player->owned_deck_count <= SHOP_MIN_DECK_SIZE) {
        return SHOP_BUY_REMOVE_UNAVAILABLE;
    }

    if (deck_index < 0 || deck_index >= player->owned_deck_count) {
        return SHOP_BUY_INVALID;
    }

    player->gold -= item->price;

    for (i = deck_index; i < player->owned_deck_count - 1; i++) {
        player->owned_deck[i] = player->owned_deck[i + 1];
    }

    player->owned_deck_count--;
    item->sold = 1;

    return SHOP_BUY_OK;
}