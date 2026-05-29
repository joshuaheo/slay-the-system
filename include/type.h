#ifndef TYPE_H
#define TYPE_H

#define MAX_NAME_LEN 64
#define MAX_DECK_SIZE 200
#define MAX_HAND_SIZE 10
#define MAX_RELICS 20
#define MAX_ENEMIES 3
#define MAX_FLOOR 15
#define MAX_STAGE_CHOICES 2
#define MAX_ACTIVE_POWERS 20
#define MAX_SAVE_SLOTS 3

//카드 타입
typedef enum {
    CARD_ATTACK,
    CARD_SKILL,
    CARD_POWER,
    CARD_STATUS
} CardType;

//공용구조체로만 해결되지 않는 카드들
typedef enum {
    SPECIAL_NONE,
    SPECIAL_FIEND_FIRE,
    SPECIAL_CRIMSON_MANTLE,
    SPECIAL_PYRE,
    SPECIAL_DEMON_FORM,
    SPECIAL_PACT_END,
    SPECIAL_DOMINATE,
    SPECIAL_FORGOTTEN_RITUAL,
    SPECIAL_ASHEN_STRIKE,
    SPECIAL_SPITE,
    SPECIAL_MOLTEN_FIST
} SpecialEffect;

//파워카드 발동 시점
typedef enum {
    POWER_TRIGGER_NONE,
    POWER_TRIGGER_TURN_START
} PowerTrigger;

//파워카드 턴별 발동
typedef struct {
    SpecialEffect special;
    PowerTrigger trigger;

    int hp_loss;
    int block;
    int strength;
    int weak;
    int vulnerable;
    int damage;
    int energy;
    int draw;
} ActivePower;

//카드 대상범위(개인,전체,무작위,스스로(ex-방어))
typedef enum {
    TARGET_ENEMY,
    TARGET_SELF,
    TARGET_ALL_ENEMIES,
    TARGET_RANDOM_ENEMY
} TargetType;

//적 구분 구조체
typedef enum {
    ENEMY_SLIME,
    ENEMY_JAW_WORM,
    ENEMY_SEAPUNK,
    ENEMY_FUZZY_WURM_CRAWLER,
    ENEMY_SHRINKER_BEETLE,
    ENEMY_SLUDGE_SPINNER,
    ENEMY_MAWLER,
    ENEMY_INLET,
    ENEMY_CUBEX_CONSTRUCT,
    ENEMY_LEAF_SLIME,
    ENEMY_TWIG_SLIME,
    ENEMY_BYGONE_EFFIGY,
    ENEMY_BYRDONIS,
    ENEMY_TERROR_EEL,
    ENEMY_VANTOM
} EnemyId;

//카드 희귀도(시작,일반,고급,희귀)
typedef enum {
    CARD_START,
    CARD_COMMON,
    CARD_UNCOMMON,
    CARD_RARE
} CardRarity;

//카드 공용 구조체(이름, 타입, 희귀도, 적 범위, 코스트, 데미지, 방어도, 힘, 약화, 취약, 카드뽑는수, 에너지획득, 자해, 소멸, 공격횟수)
typedef struct {
    char name[MAX_NAME_LEN];
    char description[128];

    CardType type;
    CardRarity rarity;
    TargetType target;

    int cost;

    int damage;
    int block;

    int strength;
    int weak;
    int vulnerable;

    int draw;
    int energy;

    int hp_loss;

    int exhaust;
    int hit_count;

    SpecialEffect special; //공용구조체로 해결되지 않는카드 구분
} Card;

//유물 효과 구분
typedef enum {
    RELIC_NONE,
    RELIC_BURNING_BLOOD,
    RELIC_ANCHOR,
    RELIC_VAJRA,
    RELIC_PEAR,
    RELIC_OLD_COIN,
    RELIC_LEES_WAFFLE,
    RELIC_PLANISPHERE,
    RELIC_CANDELABRA,
    RELIC_MERCURY_HOURGLASS,
    RELIC_BAG_OF_MARBLES,
    RELIC_STRAWBERRY,
    RELIC_LANTERN,
    RELIC_RED_MASK,
    RELIC_BAG_OF_PREPARATION,
    RELIC_MANGO,
    RELIC_CHANDELIER,
    RELIC_BRIMSTONE,
    RELIC_CAPTAINS_WHEEL,
    RELIC_ICE_CREAM,
    RELIC_STONE_CALENDAR,
    RELIC_HAPPY_FLOWER,

    RELIC_COUNT
 } RelicId;

//유물 희귀도 구분
 typedef enum {
    RELIC_STARTER,
    RELIC_COMMON,
    RELIC_UNCOMMON,
    RELIC_RARE,
    RELIC_SHOP,
    RELIC_EVENT
} RelicRarity;

//유물 설명 및 이름
typedef struct {
    RelicId id;
    RelicRarity rarity;
    char name[MAX_NAME_LEN];
    char description[128];
} Relic;

//플레이어 상태
typedef struct {
    char name[MAX_NAME_LEN];

    int max_hp;
    int hp;
    int block;

    int strength;
    int weak;
    int vulnerable;

    Card owned_deck[MAX_DECK_SIZE];
    int owned_deck_count;

    Card draw_pile[MAX_DECK_SIZE];
    int draw_count;

    Card hand[MAX_HAND_SIZE];
    int hand_count;

    Card discard_pile[MAX_DECK_SIZE];
    int discard_count;

    Card exhaust_pile[MAX_DECK_SIZE];
    int exhaust_count;

    int exhausted_this_turn;
    int hp_lost_this_turn;

    int gold;

    Relic relics[MAX_RELICS];
    int relic_count;

    ActivePower active_powers[MAX_ACTIVE_POWERS];
    int active_power_count;

    int energy;
    int max_energy;
} Player;

#define SHOP_CARD_COUNT 5
#define SHOP_RELIC_COUNT 2
#define SHOP_REMOVE_COUNT 1
#define SHOP_ITEM_COUNT 8
#define SHOP_REMOVE_PRICE 75
#define SHOP_MIN_DECK_SIZE 10

//상점 상품 타입
typedef enum {
    SHOP_ITEM_EMPTY,
    SHOP_ITEM_CARD,
    SHOP_ITEM_RELIC,
    SHOP_ITEM_REMOVE_CARD
} ShopItemType;

//상점 아이템 특성
typedef struct {
    ShopItemType type;

    int price;
    int original_price;
    int discounted;
    int sold;
    int available;

    Card card;
    Relic relic;
} ShopItem;

//적의 종류 구분 구조체
typedef enum {
    ENEMY_NORMAL,
    ENEMY_ELITE,
    ENEMY_BOSS
} EnemyGrade;

//상점 구조체
typedef struct {
    ShopItem items[SHOP_ITEM_COUNT];
    int item_count;
} Shop;

//적 상태
typedef struct {
    EnemyId id;
    EnemyGrade grade;

    char name[MAX_NAME_LEN];

    int max_hp;
    int hp;
    int block;

    int strength;
    int weak;
    int vulnerable;

    int damage;

    int turn_count;
    int pattern_index;
    int special_state;
} Enemy;

//한턴별 적의 행동 공용 구조체
typedef struct {
    int damage;
    int hit_count;

    int block;
    int strength;

    int weak;
    int vulnerable;

    int has_attack;
} EnemyMove;

//게임상태
typedef enum {
    GAME_MENU,
    GAME_BATTLE,
    GAME_CLEAR,
    GAME_OVER
} GameMode;

//현재 스테이지
typedef enum {
    STAGE_ENEMY,
    STAGE_ELITE,
    STAGE_REST,
    STAGE_SHOP,
    STAGE_CHEST,
    STAGE_EVENT,
    STAGE_BOSS
} StageType;

//스테이지 선택요소
typedef struct {
    StageType choices[MAX_STAGE_CHOICES]; //선택 목록
    int choice_count; //선택 가능 갯수
} MapFloor;

//세이브용 게임진행상황
typedef struct {
    char username[MAX_NAME_LEN];

    Player player;
 
    int floor; //마지막으로 클리어한 층
    int save_slot;
} GameState;

#endif
