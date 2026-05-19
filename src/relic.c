#include <stdlib.h>

#include "relic.h"

//유물 목록
static const Relic relic_pool[] = {
    {
        RELIC_BURNING_BLOOD,
        RELIC_STARTER,
        "불타는 피",
        "전투 승리 시 체력을 6 회복합니다."
    },
    {
        RELIC_ANCHOR,
        RELIC_COMMON,
        "닻",
        "전투 시작 시 방어도를 10 얻습니다."
    },
    {
        RELIC_VAJRA,
        RELIC_COMMON,
        "금강저",
        "전투 시작 시 힘을 1 얻습니다."
    },
    {
        RELIC_PEAR,
        RELIC_UNCOMMON,
        "배",
        "획득 시, 최대 체력이 10 증가합니다."
    },
    {
        RELIC_OLD_COIN,
        RELIC_RARE,
        "낡은 동전",
        "획득 시, 골드를 300 얻습니다."
    },
    {
        RELIC_LEES_WAFFLE,
        RELIC_SHOP,
        "리의 와플",
        "획득 시, 최대 체력이 7 상승하고 모든 체력을 회복합니다."
    }
};

//유물 희귀도 
static int is_standard_relic_rarity(RelicRarity rarity)
{
    return rarity == RELIC_COMMON ||
           rarity == RELIC_UNCOMMON ||
           rarity == RELIC_RARE;
}

//희귀도별 드랍율 설정
static RelicRarity choose_standard_relic_rarity(void)
{
    int roll = rand() % 100;

    if (roll < 60) {
        return RELIC_COMMON;
    }

    if (roll < 90) {
        return RELIC_UNCOMMON;
    }

    return RELIC_RARE;
}

//유물 적용 함수
static void apply_relic_on_obtain(Player *player, Relic relic)
{
    if (player == NULL) {
        return;
    }

    switch (relic.id) {
    case RELIC_PEAR:
        player->max_hp += 10;
        player->hp += 10;
        if (player->hp > player->max_hp) {
            player->hp = player->max_hp;
        }
        break;

    case RELIC_OLD_COIN:
        player->gold += 300;
        break;

    case RELIC_LEES_WAFFLE:
        player->max_hp += 7;
        player->hp = player->max_hp;
        break;

    default:
        break;
    }
}


int get_relic_pool_count(void)
{
    return (int)(sizeof(relic_pool) / sizeof(relic_pool[0]));
}


Relic get_relic_from_pool(int index)
{
    int count = get_relic_pool_count();

    if (index < 0 || index >= count) {
        return relic_pool[0];
    }

    return relic_pool[index];
}

int has_relic(const Player *player, RelicId id)
{
    int i;

    if (player == NULL) {
        return 0;
    }

    for (i = 0; i < player->relic_count; i++) {
        if (player->relics[i].id == id) {
            return 1;
        }
    }

    return 0;
}

int add_relic_to_player(Player *player, Relic relic)
{
    if (player == NULL) {
        return 0;
    }

    if (relic.id == RELIC_NONE) {
        return 0;
    }

    if (player->relic_count >= MAX_RELICS) {
        return 0;
    }

    if (has_relic(player, relic.id)) {
        return 0;
    }

    player->relics[player->relic_count] = relic;
    player->relic_count++;

    apply_relic_on_obtain(player, relic);

    return 1;
}

int get_random_available_relic_by_rarity(
    const Player *player,
    RelicRarity rarity,
    Relic *out_relic
)
{
    int candidates[RELIC_COUNT];
    int candidate_count = 0;
    int pool_count = get_relic_pool_count();
    int i;
    int selected_index;

    if (player == NULL || out_relic == NULL) {
        return 0;
    }

    for (i = 0; i < pool_count; i++) {
        if (relic_pool[i].rarity == rarity &&
            !has_relic(player, relic_pool[i].id)) {
            candidates[candidate_count] = i;
            candidate_count++;
        }
    }

    if (candidate_count == 0) {
        return 0;
    }

    selected_index = candidates[rand() % candidate_count];
    *out_relic = relic_pool[selected_index];

    return 1;
}

int get_random_available_standard_relic(
    const Player *player,
    Relic *out_relic
)
{
    int candidates[RELIC_COUNT];
    int candidate_count = 0;
    int pool_count = get_relic_pool_count();
    int i;
    int selected_index;

    if (player == NULL || out_relic == NULL) {
        return 0;
    }

    for (i = 0; i < pool_count; i++) {
        if (is_standard_relic_rarity(relic_pool[i].rarity) &&
            !has_relic(player, relic_pool[i].id)) {
            candidates[candidate_count] = i;
            candidate_count++;
        }
    }

    if (candidate_count == 0) {
        return 0;
    }

    selected_index = candidates[rand() % candidate_count];
    *out_relic = relic_pool[selected_index];

    return 1;
}

int grant_random_standard_relic(Player *player, Relic *out_relic)
{
    RelicRarity rarity;
    Relic relic;

    if (player == NULL || out_relic == NULL) {
        return 0;
    }

    rarity = choose_standard_relic_rarity();

    if (!get_random_available_relic_by_rarity(player, rarity, &relic)) {
        if (!get_random_available_standard_relic(player, &relic)) {
            return 0;
        }
    }

    if (!add_relic_to_player(player, relic)) {
        return 0;
    }

    *out_relic = relic;

    return 1;
}