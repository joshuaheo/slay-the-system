#include "card.h"
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

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
    CARD_INDEX_IMPERVIOUS, //index 19
};

//카드 전체 배열
static const Card card_pool[] = {
    {
        .name = "타격",
        .description = "적 하나에게 피해를 6 줍니다.",
        .type = CARD_ATTACK,
        .rarity = CARD_START,
        .target = TARGET_ENEMY,
        .cost = 1,

        .damage = 999,
        .block = 0,

        .strength = 0,
        .weak = 0,
        .vulnerable = 0,

        .draw = 0,
        .energy = 0,

        .hp_loss = 0,

        .exhaust = 0,
        .hit_count = 1,

        .special = SPECIAL_NONE
    },
     {
        .name = "강타",
        .description = "피해를 8 줍니다. 취약을 2 부여합니다.",
        .type = CARD_ATTACK,
        .rarity = CARD_START,
        .target = TARGET_ENEMY,
        .cost = 2,

        .damage = 8,
        .block = 0,

        .strength = 0,
        .weak = 0,
        .vulnerable = 2,

        .draw = 0,
        .energy = 0,

        .hp_loss = 0,

        .exhaust = 0,
        .hit_count = 1,

        .special = SPECIAL_NONE
    },
    {
        .name = "수비",
        .description = "방어도를 5 얻습니다.",
        .type = CARD_SKILL,
        .rarity = CARD_START,
        .target = TARGET_SELF,
        .cost = 1,

        .damage = 0,
        .block = 5,

        .strength = 0,
        .weak = 0,
        .vulnerable = 0,

        .draw = 0,
        .energy = 0,

        .hp_loss = 0,

        .exhaust = 0,
        .hit_count = 0,

        .special = SPECIAL_NONE
    },
    {
        .name = "부메랑 칼날",
        .description = "무작위 적에게 피해를 3만큼 3번 줍니다.",
        .type = CARD_ATTACK,
        .rarity = CARD_COMMON,
        .target = TARGET_RANDOM_ENEMY,
        .cost = 1,

        .damage = 3,
        .block = 0,

        .strength = 0,
        .weak = 0,
        .vulnerable = 0,

        .draw = 0,
        .energy = 0,

        .hp_loss = 0,

        .exhaust = 0,
        .hit_count = 3,

        .special = SPECIAL_NONE
    },
    {
        .name = "이중 타격",
        .description = "피해를 5만큼 2번 줍니다.",
        .type = CARD_ATTACK,
        .rarity = CARD_COMMON,
        .target = TARGET_ENEMY,
        .cost = 1,

        .damage = 5,
        .block = 0,

        .strength = 0,
        .weak = 0,
        .vulnerable = 0,

        .draw = 0,
        .energy = 0,

        .hp_loss = 0,

        .exhaust = 0,
        .hit_count = 2,

        .special = SPECIAL_NONE
    },
    {
        .name = "정면 돌파",
        .description = "체력을 1 잃습니다. 모든 적에게 피해를 9 줍니다.",
        .type = CARD_ATTACK,
        .rarity = CARD_COMMON,
        .target = TARGET_ALL_ENEMIES,
        .cost = 1,

        .damage = 9,
        .block = 0,

        .strength = 0,
        .weak = 0,
        .vulnerable = 0,

        .draw = 0,
        .energy = 0,

        .hp_loss = 1,

        .exhaust = 0,
        .hit_count = 1,

        .special = SPECIAL_NONE
    },
    {
        .name = "천둥",
        .description = "모든 적에게 피해를 4 주고 취약을 1 부여합니다.",
        .type = CARD_ATTACK,
        .rarity = CARD_COMMON,
        .target = TARGET_ALL_ENEMIES,
        .cost = 1,

        .damage = 4,
        .block = 0,

        .strength = 0,
        .weak = 0,
        .vulnerable = 1,

        .draw = 0,
        .energy = 0,

        .hp_loss = 0,

        .exhaust = 0,
        .hit_count = 1,

        .special = SPECIAL_NONE
    },
    {
        .name = "철의 파동",
        .description = "방어도를 5 얻습니다. 피해를 5 줍니다.",
        .type = CARD_ATTACK,
        .rarity = CARD_COMMON,
        .target = TARGET_ENEMY,
        .cost = 1,

        .damage = 5,
        .block = 5,

        .strength = 0,
        .weak = 0,
        .vulnerable = 0,

        .draw = 0,
        .energy = 0,

        .hp_loss = 0,

        .exhaust = 0,
        .hit_count = 1,

        .special = SPECIAL_NONE
    },
    {
        .name = "폼멜 타격",
        .description = "피해를 9 줍니다. 카드를 1장 뽑습니다.",
        .type = CARD_ATTACK,
        .rarity = CARD_COMMON,
        .target = TARGET_ENEMY,
        .cost = 1,

        .damage = 9,
        .block = 0,

        .strength = 0,
        .weak = 0,
        .vulnerable = 0,

        .draw = 1,
        .energy = 0,

        .hp_loss = 0,

        .exhaust = 0,
        .hit_count = 1,

        .special = SPECIAL_NONE
    },
    {
        .name = "사혈",
        .description = "체력을 3 잃습니다. 에너지 2를 얻습니다.",
        .type = CARD_SKILL,
        .rarity = CARD_COMMON,
        .target = TARGET_SELF,
        .cost = 0,

        .damage = 0,
        .block = 0,

        .strength = 0,
        .weak = 0,
        .vulnerable = 0,

        .draw = 0,
        .energy = 2,

        .hp_loss = 3,

        .exhaust = 0,
        .hit_count = 0,

        .special = SPECIAL_NONE
    },
     {
        .name = "떨림",
        .description = "취약을 3 부여합니다. 소멸.",
        .type = CARD_SKILL,
        .rarity = CARD_COMMON,
        .target = TARGET_ENEMY,
        .cost = 1,

        .damage = 0,
        .block = 0,

        .strength = 0,
        .weak = 0,
        .vulnerable = 3,

        .draw = 0,
        .energy = 0,

        .hp_loss = 0,

        .exhaust = 1,
        .hit_count = 0,

        .special = SPECIAL_NONE
    },
    {
        .name = "흘려보내기",
        .description = "방어도를 8 얻습니다. 카드를 1장 뽑습니다.",
        .type = CARD_SKILL,
        .rarity = CARD_COMMON,
        .target = TARGET_SELF,
        .cost = 1,

        .damage = 0,
        .block = 8,

        .strength = 0,
        .weak = 0,
        .vulnerable = 0,

        .draw = 1,
        .energy = 0,

        .hp_loss = 0,

        .exhaust = 0,
        .hit_count = 0,

        .special = SPECIAL_NONE
    },
    {
        .name = "피의 벽",
        .description = "체력을 2 잃습니다. 방어도를 16 얻습니다.",
        .type = CARD_SKILL,
        .rarity = CARD_COMMON,
        .target = TARGET_SELF,
        .cost = 2,

        .damage = 0,
        .block = 16,

        .strength = 0,
        .weak = 0,
        .vulnerable = 0,

        .draw = 0,
        .energy = 0,

        .hp_loss = 2,

        .exhaust = 0,
        .hit_count = 0,

        .special = SPECIAL_NONE
    },
    {
        .name = "혈류",
        .description = "체력을 2 잃습니다. 피해를 15 줍니다.",
        .type = CARD_ATTACK,
        .rarity = CARD_UNCOMMON,
        .target = TARGET_ENEMY,
        .cost = 1,

        .damage = 15,
        .block = 0,

        .strength = 0,
        .weak = 0,
        .vulnerable = 0,

        .draw = 0,
        .energy = 0,

        .hp_loss = 2,

        .exhaust = 0,
        .hit_count = 1,

        .special = SPECIAL_NONE
    },
    {
        .name = "어퍼컷",
        .description = "피해를 12 줍니다. 약화를 1 부여합니다. 취약을 1 부여합니다.",
        .type = CARD_ATTACK,
        .rarity = CARD_UNCOMMON,
        .target = TARGET_ENEMY,
        .cost = 2,

        .damage = 12,
        .block = 0,

        .strength = 0,
        .weak = 1,
        .vulnerable = 1,

        .draw = 0,
        .energy = 0,

        .hp_loss = 0,

        .exhaust = 0,
        .hit_count = 1,

        .special = SPECIAL_NONE
    },
    {
        .name = "몽둥이질",
        .description = "피해를 32 줍니다.",
        .type = CARD_ATTACK,
        .rarity = CARD_UNCOMMON,
        .target = TARGET_ENEMY,
        .cost = 3,

        .damage = 32,
        .block = 0,

        .strength = 0,
        .weak = 0,
        .vulnerable = 0,

        .draw = 0,
        .energy = 0,

        .hp_loss = 0,

        .exhaust = 0,
        .hit_count = 1,

        .special = SPECIAL_NONE
    },
    {
        .name = "도발",
        .description = "방어도를 7 얻습니다. 취약을 1 부여합니다.",
        .type = CARD_SKILL,
        .rarity = CARD_UNCOMMON,
        .target = TARGET_ENEMY,
        .cost = 1,

        .damage = 0,
        .block = 7,

        .strength = 0,
        .weak = 0,
        .vulnerable = 1,

        .draw = 0,
        .energy = 0,

        .hp_loss = 0,

        .exhaust = 0,
        .hit_count = 0,

        .special = SPECIAL_NONE
    },
    {
        .name = "발화",
        .description = "힘을 2 얻습니다.",
        .type = CARD_POWER,
        .rarity = CARD_UNCOMMON,
        .target = TARGET_SELF,
        .cost = 1,

        .damage = 0,
        .block = 0,

        .strength = 2,
        .weak = 0,
        .vulnerable = 0,

        .draw = 0,
        .energy = 0,

        .hp_loss = 0,

        .exhaust = 0,
        .hit_count = 0,

        .special = SPECIAL_NONE
    },
    {
        .name = "제물",
        .description = "체력을 6 잃습니다. 에너지 2를 얻습니다. 카드를 3장 뽑습니다. 소멸.",
        .type = CARD_SKILL,
        .rarity = CARD_RARE,
        .target = TARGET_SELF,
        .cost = 0,

        .damage = 0,
        .block = 0,

        .strength = 0,
        .weak = 0,
        .vulnerable = 0,

        .draw = 3,
        .energy = 2,

        .hp_loss = 6,

        .exhaust = 1,
        .hit_count = 0,

        .special = SPECIAL_NONE
    },
    {
        .name = "무적",
        .description = "방어도를 30 얻습니다. 소멸.",
        .type = CARD_SKILL,
        .rarity = CARD_RARE,
        .target = TARGET_SELF,
        .cost = 2,

        .damage = 0,
        .block = 30,

        .strength = 0,
        .weak = 0,
        .vulnerable = 0,

        .draw = 0,
        .energy = 0,

        .hp_loss = 0,

        .exhaust = 1,
        .hit_count = 0,

        .special = SPECIAL_NONE
    }
};

//카드 덱에 카드가 몇개 있는지 반환하는 함수
int get_card_pool_count(void)
{
    return sizeof(card_pool) / sizeof(card_pool[0]);
}

//카드덱으로부터 무슨 카드를 가져올지 파악하고 반환하는 함수
Card get_card_from_pool(int index)
{
    int count;

    count = get_card_pool_count();

    if (index < 0 || index >= count) {
        return card_pool[0];
    }

    return card_pool[index];
}

//자신의 카드덱에 카드를 추가하는 함수
int add_card_to_deck(Player *player, Card card)
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

//스타딩덱 만들어주는 함수
void init_starting_deck(Player *player)
{
    int i;

    if (player == NULL) {
        return;
    }

    player->owned_deck_count = 0;

    for (i = 0; i < 5; i++) {
        add_card_to_deck(player, get_card_from_pool(CARD_INDEX_STRIKE));
    }

    for (i = 0; i < 4; i++) {
        add_card_to_deck(player, get_card_from_pool(CARD_INDEX_DEFEND));
    }

    add_card_to_deck(player, get_card_from_pool(CARD_INDEX_BASH));
}

//draw_pile 카드 순서를 랜덤하게 섞는 함수
void shuffle_draw_pile(Player *player)
{
    int i;

    if (player == NULL) {
        return;
    }

    for (i = player->draw_count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Card temp = player->draw_pile[i];

        player->draw_pile[i] = player->draw_pile[j];
        player->draw_pile[j] = temp;
    }
}

//버림더미에 있는 카드들을 드로우 더미로 옮기는 함수
static void move_discard_to_draw(Player *player)
{
    int i;

    if (player == NULL) {
        return;
    }

    if (player->discard_count <= 0) {
        return;
    }

    player->draw_count = 0;

    for (i = 0; i < player->discard_count; i++) {
        if (player->draw_count < MAX_DECK_SIZE) {
            player->draw_pile[player->draw_count] = player->discard_pile[i];
            player->draw_count++;
        }
    }

    player->discard_count = 0;

    shuffle_draw_pile(player);
}

//드로우 더미에서 카드를 count장 뽑는 함수
void draw_cards(Player *player, int count)
{
    int i;
    Card drawn_card;

    if (player == NULL) {
        return;
    }

    for (i = 0; i < count; i++) {
        if (player->draw_count <= 0) {
            move_discard_to_draw(player);
        }

        if (player->draw_count <= 0) {
            return;
        }

        player->draw_count--;
        drawn_card = player->draw_pile[player->draw_count];

        if (player->hand_count < MAX_HAND_SIZE) {
            player->hand[player->hand_count] = drawn_card;
            player->hand_count++;
        } else {
            if (player->discard_count < MAX_DECK_SIZE) {
                player->discard_pile[player->discard_count] = drawn_card;
                player->discard_count++;
            }
        }
    }
}

//현재 손패를 전부 버림더미에 보내는 함수
void discard_hand(Player *player)
{
    int i;

    if (player == NULL) {
        return;
    }

    for (i = 0; i < player->hand_count; i++) {
        if (player->discard_count < MAX_DECK_SIZE) {
            player->discard_pile[player->discard_count] = player->hand[i];
            player->discard_count++;
        }
    }

    player->hand_count = 0;
}

//전투시작 전 카드더미를 준비하는 함수
void prepare_battle_deck(Player *player)
{
    int i;

    if (player == NULL) {
        return;
    }

    player->draw_count = 0;
    player->hand_count = 0;
    player->discard_count = 0;
    player->exhaust_count = 0;

    for (i = 0; i < player->owned_deck_count; i++) {
        if (player->draw_count < MAX_DECK_SIZE) {
            player->draw_pile[player->draw_count] = player->owned_deck[i];
            player->draw_count++;
        }
    }

    shuffle_draw_pile(player);
    draw_cards(player, 5);
}