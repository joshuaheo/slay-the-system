#include "card.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//카드 전체 배열
static const Card card_pool[] = {
    {
        .name = "타격",
        .description = "적 하나에게 피해를 6 줍니다.",
        .type = CARD_ATTACK,
        .rarity = CARD_START,
        .target = TARGET_ENEMY,
        .cost = 1,

        .damage = 6,
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
    },
    {
    .name = "궁극의 타격",
    .description = "피해를 14 줍니다.",
    .type = CARD_ATTACK,
    .rarity = CARD_RARE,
    .target = TARGET_ENEMY,
    .cost = 1,

    .damage = 14,
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
    .name = "궁극의 수비",
    .description = "방어도를 11 얻습니다.",
    .type = CARD_SKILL,
    .rarity = CARD_RARE,
    .target = TARGET_SELF,
    .cost = 1,

    .damage = 0,
    .block = 11,

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
    "지옥불",
    "손에 있는 모든 카드를 소멸시키고 소멸시킨 카드 1장당 피해를 7 줍니다.",
    CARD_ATTACK,
    CARD_RARE,
    TARGET_ENEMY,
    2,
    7,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    SPECIAL_FIEND_FIRE
},
{
    "핏빛 망토",
    "내 턴 시작 시 체력을 1 잃고 방어도를 8 얻습니다.",
    CARD_POWER,
    CARD_RARE,
    TARGET_SELF,
    1,
    0,
    8,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    SPECIAL_CRIMSON_MANTLE
},
{
    "불의 심장",
    "매 턴 시작 시 에너지를 2 얻습니다.",
    CARD_POWER,
    CARD_RARE,
    TARGET_SELF,
    2,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    SPECIAL_PYRE
},
{
    "악마의 형상",
    "내 턴 시작 시 힘을 2 얻습니다.",
    CARD_POWER,
    CARD_RARE,
    TARGET_SELF,
    2,
    0,
    0,
    2,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    SPECIAL_DEMON_FORM
},
{
    "조약의 끝",
    "소멸 카드 더미에 카드가 3장 이상 있을 때만 사용할 수 있습니다. 모든 적에게 피해를 17 줍니다.",
    CARD_ATTACK,
    CARD_RARE,
    TARGET_ALL_ENEMIES,
    0,
    17,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    SPECIAL_PACT_END
},
{
    "제압",
    "취약을 2 부여합니다. 대상 적이 보유한 취약 수치만큼 힘을 얻습니다. 소멸.",
    CARD_SKILL,
    CARD_UNCOMMON,
    TARGET_ENEMY,
    1,
    0,
    0,
    0,
    0,
    2,
    0,
    0,
    0,
    1,
    1,
    SPECIAL_DOMINATE
},
{
    "잊힌 의식",
    "이번 턴에 카드를 소멸시켰다면 에너지를 3 얻습니다. 소멸.",
    CARD_SKILL,
    CARD_UNCOMMON,
    TARGET_SELF,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    3,
    0,
    1,
    1,
    SPECIAL_FORGOTTEN_RITUAL
},
{
    "잿빛 타격",
    "피해를 6 줍니다. 소멸 카드 더미에 있는 카드 1장당 피해량이 3 증가합니다.",
    CARD_ATTACK,
    CARD_UNCOMMON,
    TARGET_ENEMY,
    1,
    6,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    SPECIAL_ASHEN_STRIKE
},
{
    "악의",
    "피해를 5 줍니다. 이번 턴 동안 스스로 체력을 잃었다면 2번 적중합니다.",
    CARD_ATTACK,
    CARD_UNCOMMON,
    TARGET_ENEMY,
    0,
    5,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    SPECIAL_SPITE
},
{
    "녹아내리는 주먹",
    "피해를 10 줍니다. 적이 보유한 취약이 2배로 증가합니다. 소멸.",
    CARD_ATTACK,
    CARD_COMMON,
    TARGET_ENEMY,
    1,
    10,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    SPECIAL_MOLTEN_FIST
},
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
<<<<<<< HEAD
=======

//점액투성이 카드 생성 함수
Card create_goop_card(void)
{
    Card card;

    memset(&card, 0, sizeof(Card));

    strncpy(card.name, "점액투성이", MAX_NAME_LEN - 1);
    card.name[MAX_NAME_LEN - 1] = '\0';

    strncpy(card.description, "카드를 1장 뽑습니다. 소멸.", sizeof(card.description) - 1);
    card.description[sizeof(card.description) - 1] = '\0';

    card.type = CARD_STATUS;
    card.rarity = CARD_COMMON;
    card.target = TARGET_SELF;
    card.cost = 1;
    card.draw = 1;
    card.exhaust = 1;
    card.hit_count = 1;
    card.special = SPECIAL_NONE;

    return card;
}

//버린 카드 더미에 카드 추가 함수
int add_card_to_discard(Player *player, Card card)
{
    if (player == NULL) {
        return 0;
    }

    if (player->discard_count >= MAX_DECK_SIZE) {
        return 0;
    }

    player->discard_pile[player->discard_count] = card;
    player->discard_count++;

    return 1;
}

//부상 카드 생성 함수
Card create_wound_card(void)
{
    Card card;

    memset(&card, 0, sizeof(Card));

    strncpy(card.name, "부상", MAX_NAME_LEN - 1);
    card.name[MAX_NAME_LEN - 1] = '\0';

    strncpy(card.description, "사용할 수 없습니다.", sizeof(card.description) - 1);
    card.description[sizeof(card.description) - 1] = '\0';

    card.type = CARD_STATUS;
    card.rarity = CARD_COMMON;
    card.target = TARGET_SELF;
    card.cost = -1;

    card.damage = 0;
    card.block = 0;
    card.strength = 0;
    card.weak = 0;
    card.vulnerable = 0;
    card.draw = 0;
    card.energy = 0;
    card.hp_loss = 0;
    card.exhaust = 0;
    card.hit_count = 0;
    card.special = SPECIAL_NONE;

    return card;
}

//탐욕 카드 생성 함수
Card create_greed_card(void)
{
    Card card;

    memset(&card, 0, sizeof(Card));

    strncpy(card.name, "탐욕", MAX_NAME_LEN - 1);
    card.name[MAX_NAME_LEN - 1] = '\0';

    strncpy(card.description, "사용할 수 없습니다. 제거할 수 없습니다.", sizeof(card.description) - 1);
    card.description[sizeof(card.description) - 1] = '\0';

    card.type = CARD_STATUS;
    card.rarity = CARD_COMMON;
    card.target = TARGET_SELF;
    card.cost = -1;

    card.damage = 0;
    card.block = 0;
    card.strength = 0;
    card.weak = 0;
    card.vulnerable = 0;
    card.draw = 0;
    card.energy = 0;
    card.hp_loss = 0;
    card.exhaust = 0;
    card.hit_count = 0;
    card.special = SPECIAL_NONE;

    return card;
}
>>>>>>> bdf9c9d15eab48477b408ea0a91012e118b9a69c
