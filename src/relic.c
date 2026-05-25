#include <stdlib.h>

#include "player.h"
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
    }, //index 5
    {
    RELIC_PLANISPHERE,
    RELIC_UNCOMMON,
    "별자리판",
    "이벤트 방에 진입할 때마다 체력을 5 회복합니다."
    },
};

//드랍율에 적용받는 희귀도인지 확인하는 함수
static int is_standard_relic_rarity(RelicRarity rarity)
{
    return rarity == RELIC_COMMON ||
           rarity == RELIC_UNCOMMON ||
           rarity == RELIC_RARE;
}

//희귀도별 드랍율 설정해주는 함수
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

//획득 즉시 발동하는 유물 효과를 적용하는 함수
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

// 전체 유물 풀에 등록된 유물 개수를 반환하는 함수
int get_relic_pool_count(void)
{
    return (int)(sizeof(relic_pool) / sizeof(relic_pool[0]));
}

// 유물 풀에서 index에 해당하는 유물을 반환하는 함수
Relic get_relic_from_pool(int index)
{
    int count = get_relic_pool_count();

    if (index < 0 || index >= count) {
        return relic_pool[0];
    }

    return relic_pool[index];
}

//플레이어가 해당 유물을 가지고 있는지 확인하는 함수
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

//플레이어에게 유물을 추가하는 함수
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

//특정 희귀도에서 플레이어가 아직 가지지 않은 유물을 랜덤으로 선택하는 함수
int get_random_available_relic_by_rarity(const Player *player, RelicRarity rarity, Relic *out_relic)
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

// 표준 드랍풀에서 플레이어가 아직 가지지 않은 유물을 랜덤으로 선택하는 함수
int get_random_available_standard_relic(const Player *player, Relic *out_relic)
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

// 표준 드랍풀에서 랜덤 유물을 선택해 플레이어에게 지급합니다.
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

// 전투 시작 시 발동하는 유물 효과를 적용합니다.
void apply_relics_on_battle_start(Player *player)
{
    if (player == NULL) {
        return;
    }

    if (has_relic(player, RELIC_ANCHOR)) {
        player->block += 10;
    }

    if (has_relic(player, RELIC_VAJRA)) {
        player->strength += 1;
    }
}

// 전투 승리 시 발동하는 유물 효과를 적용합니다.
void apply_relics_on_battle_win(Player *player)
{
    if (player == NULL) {
        return;
    }

    if (has_relic(player, RELIC_BURNING_BLOOD)) {
        heal_player(player, 6);
    }
}

//이벤트 스테이지 들어갔을떄 적용되는 유물
void apply_relics_on_stage_enter(Player *player, StageType stage)
{
    if (player == NULL) {
        return;
    }

    if (stage == STAGE_EVENT && has_relic(player, RELIC_PLANISPHERE)) {
        heal_player(player, 5);
    }
}
