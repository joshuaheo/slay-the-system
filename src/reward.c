#include <stdlib.h>
#include <string.h>
#include "reward.h"
#include "card.h"

//카드 보상 희귀도 설정 함수
static CardRarity roll_card_reward_rarity(void)
{
    int roll;

    roll = rand() % 100;

    if (roll < 60) {
        return CARD_COMMON;
    }

    if (roll < 90) {
        return CARD_UNCOMMON;
    }

    return CARD_RARE;
}

//중복 보상 판정 함수
static int is_same_card(const Card *a, const Card *b)
{
    if (a == NULL || b == NULL) {
        return 0;
    }

    return strcmp(a->name, b->name) == 0;
}

//즁복 보상 방지 함수
static int is_duplicate_reward(Card rewards[], int reward_count, const Card *card)
{
    int i;

    if (rewards == NULL || card == NULL) {
        return 0;
    }

    for (i = 0; i < reward_count; i++) {
        if (is_same_card(&rewards[i], card)) {
            return 1;
        }
    }

    return 0;
}

//카드 보상 희귀도 생성 함수
static int get_random_card_by_rarity(CardRarity rarity, Card *out_card)
{
    int i;
    int pool_count;
    int match_count;
    int selected;
    Card card;

    if (out_card == NULL) {
        return 0;
    }

    pool_count = get_card_pool_count();
    match_count = 0;

    for (i = 0; i < pool_count; i++) {
        card = get_card_from_pool(i);

        if (card.rarity == rarity) {
            match_count++;
        }
    }

    if (match_count <= 0) {
        return 0;
    }

    selected = rand() % match_count;

    for (i = 0; i < pool_count; i++) {
        card = get_card_from_pool(i);

        if (card.rarity == rarity) {
            if (selected == 0) {
                *out_card = card;
                return 1;
            }

            selected--;
        }
    }

    return 0;
}

//희귀도에 따른 카드 보상 생성 함수
static int get_random_reward_card(Card *out_card)
{
    int retry;
    CardRarity rarity;

    if (out_card == NULL) {
        return 0;
    }

    retry = 0;

    while (retry < 30) {
        rarity = roll_card_reward_rarity();

        if (get_random_card_by_rarity(rarity, out_card)) {
            return 1;
        }

        retry++;
    }

    if (get_random_card_by_rarity(CARD_COMMON, out_card)) {
        return 1;
    }

    if (get_random_card_by_rarity(CARD_UNCOMMON, out_card)) {
        return 1;
    }

    if (get_random_card_by_rarity(CARD_RARE, out_card)) {
        return 1;
    }

    return 0;
}

//카드 보상 생성 함수
void generate_card_rewards(Card rewards[], int reward_count)
{
    int i;
    int retry;
    Card card;

    if (rewards == NULL || reward_count <= 0) {
        return;
    }

    for (i = 0; i < reward_count; i++) {
        retry = 0;

        while (retry < 50) {
            if (get_random_reward_card(&card)) {
                if (!is_duplicate_reward(rewards, i, &card)) {
                    rewards[i] = card;
                    break;
                }
            }

            retry++;
        }

        if (retry >= 50) {
            get_random_reward_card(&rewards[i]);
        }
    }
}

//골드 보상 생성 함수
int generate_gold_reward(int min_gold, int max_gold)
{
    int range;

    if (min_gold > max_gold) {
        return min_gold;
    }

    range = max_gold - min_gold + 1;

    return min_gold + rand() % range;
}
