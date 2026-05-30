#include <string.h>
#include <stdlib.h>

#include "enemy.h"
<<<<<<< HEAD
#include "game.h"
=======
#include "card.h"
>>>>>>> bdf9c9d15eab48477b408ea0a91012e118b9a69c

//적 패텀 매크로 함수
#if 1
#define SLUDGE_OIL_SPRAY 0
#define SLUDGE_SLAM      1
#define SLUDGE_RAGE      2

#define MAWLER_CLAW 0
#define MAWLER_RIP_AND_TEAR 1
#define MAWLER_ROAR 2

#define TERROR_EEL_CRASH 0
#define TERROR_EEL_THRASH 1
#define TERROR_EEL_TERROR 2
#define TERROR_EEL_STUN 3

#define TERROR_EEL_TERROR_USED 1
#define TERROR_EEL_VIGOR_READY 2
#define TERROR_EEL_STUN_PENDING 4
#define TERROR_EEL_TERROR_PENDING 8
#endif
//static 함수 모음
#if 1
static void jaw_worm_take_turn(Enemy *enemy, Player *player);
static void seapunk_take_turn(Enemy *enemy, Player *player);
static void fuzzy_wurm_crawler_take_turn(Enemy *enemy, Player *player);
static void shrinker_beetle_take_turn(Enemy *enemy, Player *player);

static void sludge_spinner_take_turn(Enemy *enemy, Player *player);
static int choose_next_sludge_spinner_action(int previous_action);

static void mawler_take_turn(Enemy *enemy, Player *player);
static int choose_next_mawler_action(int previous_action, int roar_used);

static void decrease_enemy_positive_value(int *value);
static void decrease_enemy_turn_statuses(Enemy *enemy);

static void inlet_take_turn(Enemy *enemy, Player *player);

static void cubex_construct_take_turn(Enemy *enemy, Player *player);

static void leaf_slime_take_turn(Enemy *enemy, Player *player);

static void twig_slime_take_turn(Enemy *enemy, Player *player);
static int choose_next_twig_slime_action(int previous_action, int streak);

static void bygone_effigy_take_turn(Enemy *enemy, Player *player);

static void byrdonis_take_turn(Enemy *enemy, Player *player);

static void terror_eel_take_turn(Enemy *enemy, Player *player);

static void vantom_take_turn(Enemy *enemy, Player *player);
#endif

//enemy_move 초기화 함수
static void clear_enemy_move(EnemyMove *move)
{
    if (move == NULL) {
        return;
    }

    memset(move, 0, sizeof(EnemyMove));
    move->hit_count = 1;
}

//적 공격 데미지 계산 함수(힘,약화,취약)
static int calculate_enemy_attack_damage_with_base(const Enemy *enemy,const Player *player,int base_damage)
{
    int damage;

    if (enemy == NULL || player == NULL) {
        return 0;
    }

    if (enemy->hp <= 0) {
        return 0;
    }

    damage = base_damage + enemy->strength;

    if (damage < 0) {
        damage = 0;
    }

    if (enemy->weak > 0) {
        damage = damage * 3 / 4;
    }

    if (player->vulnerable > 0) {
        damage = damage * 3 / 2;
    }

    if (damage < 0) {
        damage = 0;
    }

    return damage;
}

//적 공격이 실제로 적용되는 함수(방어도, 체력)
static void enemy_attack_player_with_damage(Enemy *enemy,Player *player,int base_damage)
{
    int damage;
    int blocked_damage;

    if (enemy == NULL || player == NULL) {
        return;
    }

    if (enemy->hp <= 0 || player->hp <= 0) {
        return;
    }

    damage = calculate_enemy_attack_damage_with_base(enemy,player,base_damage);

    if (damage <= 0) {
        return;
    }

    if (player->block >= damage) {
        player->block -= damage;
        damage = 0;
    } else {
        blocked_damage = player->block;
        player->block = 0;
        damage -= blocked_damage;
    }

    stats.total_damage_taken += damage;
    player->hp -= damage;

    if (player->hp < 0) {
        player->hp = 0;
    }
}

//enemy_move 적용 함수
static void apply_enemy_move(Enemy *enemy,Player *player,const EnemyMove *move)
{
    int i;
    int hit_count;

    if (enemy == NULL || player == NULL || move == NULL) {
        return;
    }

    if (move->block > 0) {
        enemy->block += move->block;
    }

    if (move->strength > 0) {
        enemy->strength += move->strength;
    }

    if (move->weak > 0) {
        player->weak += move->weak;
    }

    if (move->vulnerable > 0) {
        player->vulnerable += move->vulnerable;
    }

    if (move->has_attack) {
        hit_count = move->hit_count;

        if (hit_count <= 0) {
            hit_count = 1;
        }

        for (i = 0; i < hit_count; i++) {
            enemy_attack_player_with_damage(enemy,player,move->damage);

            if (player->hp <= 0) {
                return;
            }
        }
    }
}

//적 스탯 초기화 함수
void init_enemy(Enemy *enemy, EnemyId id)
{
    if (enemy == NULL) {
        return;
    }

    memset(enemy, 0, sizeof(Enemy));

    enemy->id = id;

    enemy->block = 0;
    enemy->strength = 0;
    enemy->weak = 0;
    enemy->vulnerable = 0;

    enemy->turn_count = 0;
    enemy->pattern_index = 0;
    enemy->special_state = 0;

    switch (id) {
    case ENEMY_VANTOM:
    strncpy(enemy->name, "밴텀", MAX_NAME_LEN - 1);
    enemy->name[MAX_NAME_LEN - 1] = '\0';
    enemy->grade = ENEMY_BOSS;
    enemy->max_hp = 173;
    enemy->hp = enemy->max_hp;
    enemy->damage = 7;
    enemy->pattern_index = 0;
    enemy->special_state = 8;
    break;
    case ENEMY_TERROR_EEL:
    strncpy(enemy->name, "공포 장어", MAX_NAME_LEN - 1);
    enemy->name[MAX_NAME_LEN - 1] = '\0';
    enemy->grade = ENEMY_ELITE;
    enemy->max_hp = 140;
    enemy->hp = enemy->max_hp;
    enemy->damage = 16;
    enemy->pattern_index = TERROR_EEL_CRASH;
    enemy->special_state = 0;
    break;
    case ENEMY_BYRDONIS:
    strncpy(enemy->name, "버도니스", MAX_NAME_LEN - 1);
    enemy->name[MAX_NAME_LEN - 1] = '\0';
    enemy->grade = ENEMY_ELITE;
    enemy->max_hp = 81 + rand() % 4;
    enemy->hp = enemy->max_hp;
    enemy->damage = 17;
    enemy->pattern_index = 0;
    enemy->special_state = 0;
    break;
    case ENEMY_BYGONE_EFFIGY:
    strncpy(enemy->name, "옛 시대의 우상", MAX_NAME_LEN - 1);
    enemy->name[MAX_NAME_LEN - 1] = '\0';
    enemy->grade = ENEMY_ELITE;
    enemy->max_hp = 127;
    enemy->hp = enemy->max_hp;
    enemy->damage = 13;
    enemy->pattern_index = 0;
    enemy->special_state = 0;
    break;
    case ENEMY_TWIG_SLIME:
    strncpy(enemy->name, "가지 슬라임", MAX_NAME_LEN - 1);
    enemy->name[MAX_NAME_LEN - 1] = '\0';
    enemy->grade = ENEMY_NORMAL;
    enemy->max_hp = 26 + rand() % 3;
    enemy->hp = enemy->max_hp;
    enemy->damage = 11;
    enemy->pattern_index = 0;
    enemy->special_state = -1;
    break;
    case ENEMY_LEAF_SLIME:
    strncpy(enemy->name, "나뭇잎 슬라임", MAX_NAME_LEN - 1);
    enemy->name[MAX_NAME_LEN - 1] = '\0';
    enemy->grade = ENEMY_NORMAL;
    enemy->max_hp = 32 + rand() % 4;
    enemy->hp = enemy->max_hp;
    enemy->damage = 8;
    enemy->pattern_index = 0;
    enemy->special_state = 0;
    break;
    case ENEMY_CUBEX_CONSTRUCT:
    strncpy(enemy->name, "큐브형 구조체", MAX_NAME_LEN - 1);
    enemy->name[MAX_NAME_LEN - 1] = '\0';
    enemy->grade = ENEMY_NORMAL;
    enemy->max_hp = 65;
    enemy->hp = enemy->max_hp;
    enemy->block = 13;
    enemy->damage = 7;
    enemy->pattern_index = 0;
    enemy->special_state = 1;
    break;
    case ENEMY_INLET:
    strncpy(enemy->name, "잉클릿", MAX_NAME_LEN - 1);
    enemy->name[MAX_NAME_LEN - 1] = '\0';
    enemy->grade = ENEMY_NORMAL;
    enemy->max_hp = 11 + rand() % 7;
    enemy->hp = enemy->max_hp;
    enemy->damage = 3;
    enemy->pattern_index = 1;
    enemy->special_state = 1;
    break;
    case ENEMY_MAWLER:
    strncpy(enemy->name, "장수아귀", MAX_NAME_LEN - 1);
    enemy->name[MAX_NAME_LEN - 1] = '\0';
    enemy->grade = ENEMY_NORMAL;
    enemy->max_hp = 72;
    enemy->hp = enemy->max_hp;
    enemy->damage = 4;
    enemy->pattern_index = MAWLER_CLAW;
    enemy->special_state = -1;
    break;
    case ENEMY_SLUDGE_SPINNER:
        enemy->grade = ENEMY_NORMAL;

        strncpy(enemy->name, "오물팽이", MAX_NAME_LEN - 1);
        enemy->name[MAX_NAME_LEN - 1] = '\0';

        enemy->max_hp = 38;
        enemy->hp = 38;
        enemy->damage = 8;
        
        enemy->pattern_index = SLUDGE_OIL_SPRAY;
        enemy->special_state = -1;
        break;
    case ENEMY_SHRINKER_BEETLE:
        enemy->grade = ENEMY_NORMAL;

        strncpy(enemy->name, "압축벌레", MAX_NAME_LEN - 1);
        enemy->name[MAX_NAME_LEN - 1] = '\0';

        enemy->max_hp = 39;
        enemy->hp = 39;
        enemy->damage = 7;
        break;
    case ENEMY_FUZZY_WURM_CRAWLER:
        enemy->grade = ENEMY_NORMAL;

        strncpy(enemy->name, "복슬지렁이", MAX_NAME_LEN - 1);
        enemy->name[MAX_NAME_LEN - 1] = '\0';

        enemy->max_hp = 56;
        enemy->hp = 56;
        enemy->damage = 4;
        break;    
    case ENEMY_JAW_WORM:
        enemy->grade = ENEMY_NORMAL;

        strncpy(enemy->name, "깨작이", MAX_NAME_LEN - 1);
        enemy->name[MAX_NAME_LEN - 1] = '\0';

        enemy->max_hp = 40;
        enemy->hp = 40;
        enemy->damage = 12;
        break;
           
    case ENEMY_SEAPUNK:
        enemy->grade = ENEMY_NORMAL;
        strncpy(enemy->name, "불량 해초", MAX_NAME_LEN - 1);
        enemy->name[MAX_NAME_LEN - 1] = '\0';
        enemy->max_hp = 45;
        enemy->hp = 45;
        enemy->damage = 11;
        break;

    default:
        enemy->grade = ENEMY_NORMAL;

        strncpy(enemy->name, "Unknown", MAX_NAME_LEN - 1);
        enemy->name[MAX_NAME_LEN - 1] = '\0';

        enemy->max_hp = 1;
        enemy->hp = 1;
        enemy->damage = 0;
        break;
    }
}

//적이 살아있는지 확인하는 함수
int is_enemy_alive(const Enemy *enemy)
{
    if (enemy == NULL) {
        return 0;
    }

    return enemy->hp > 0;
}

//적이 플레이어를 공격하는 함수
void enemy_attack_player(Enemy *enemy, Player *player)
{
    int damage;
    int blocked_damage;

    if (enemy == NULL || player == NULL) {
        return;
    }

    if (enemy->hp <= 0 || player->hp <= 0) {
        return;
    }

    damage = calculate_enemy_attack_damage_with_base(enemy, player,enemy->damage);

    if (damage <= 0) {
        return;
    }

    if (player->block >= damage) {
        player->block -= damage;
        damage = 0;
    } else {
        blocked_damage = player->block;
        player->block = 0;
        damage -= blocked_damage;
    }

    player->hp -= damage;

    if (player->hp < 0) {
        player->hp = 0;
    }
}

//적별 패턴 연결 함수
static void enemy_take_turn(Enemy *enemy, Player *player)
{
    if (enemy == NULL || player == NULL) {
        return;
    }

    if (!is_enemy_alive(enemy) || player->hp <= 0) {
        return;
    }

    switch (enemy->id) {
    case ENEMY_VANTOM:
    vantom_take_turn(enemy, player);
    break;
    case ENEMY_TERROR_EEL:
        terror_eel_take_turn(enemy, player);
        break;
    case ENEMY_BYRDONIS:
        byrdonis_take_turn(enemy, player);
        break;
    case ENEMY_BYGONE_EFFIGY:
        bygone_effigy_take_turn(enemy, player);
        break;
    case ENEMY_TWIG_SLIME:
        twig_slime_take_turn(enemy, player);
        break;
    case ENEMY_LEAF_SLIME:
        leaf_slime_take_turn(enemy, player);
        break;
    case ENEMY_CUBEX_CONSTRUCT:
        cubex_construct_take_turn(enemy, player);
        break;
    case ENEMY_INLET:
        inlet_take_turn(enemy, player);
        break;
    case ENEMY_MAWLER:
        mawler_take_turn(enemy, player);
        break;
    case ENEMY_SLUDGE_SPINNER:
        sludge_spinner_take_turn(enemy, player);
        break;
    case ENEMY_SHRINKER_BEETLE:
        shrinker_beetle_take_turn(enemy, player);
        break;
    case ENEMY_FUZZY_WURM_CRAWLER:
        fuzzy_wurm_crawler_take_turn(enemy, player);
        break;
    case ENEMY_JAW_WORM:
        jaw_worm_take_turn(enemy, player);
        break;
    case ENEMY_SEAPUNK:    
        seapunk_take_turn(enemy, player);    
        break;

    case ENEMY_SLIME:
    default:
        enemy_attack_player_with_damage(enemy, player, enemy->damage);
        break;
    }
    decrease_enemy_turn_statuses(enemy);
    enemy->turn_count++;
}

//적 턴 진행 함수
void enemies_take_turn(Enemy enemies[], int enemy_count, Player *player)
{
    int i;

    if (enemies == NULL || player == NULL) {
        return;
    }

    for (i = 0; i < enemy_count; i++) {
        if (player->hp <= 0) {
            return;
        }

        enemy_take_turn(&enemies[i], player);
    }
}

//일반 몬스터 블랑 해초 함수
static void seapunk_take_turn(Enemy *enemy, Player *player)
{
    EnemyMove move;

    if (enemy == NULL || player == NULL) {
        return;
    }

    clear_enemy_move(&move);

    if (enemy->pattern_index == 0) {
        move.has_attack = 1;
        move.damage = 11;
        move.hit_count = 1;

        enemy->pattern_index = 1;
    }
    else if (enemy->pattern_index == 1) {
        move.has_attack = 1;
        move.damage = 4;
        move.hit_count = 2;

        enemy->pattern_index = 2;
    }
    else {
        move.block = 7;
        move.strength = 1;

        enemy->pattern_index = 0;
    }

    apply_enemy_move(enemy, player, &move);
}

//일반 몬스터 깨작이 함수
static void jaw_worm_take_turn(Enemy *enemy, Player *player)
{
    EnemyMove move;

    if (enemy == NULL || player == NULL) {
        return;
    }

    clear_enemy_move(&move);

    if (enemy->pattern_index == 0) {
        move.has_attack = 1;
        move.damage = 12;
        move.hit_count = 1;

        enemy->pattern_index = 1;
    }
    else if (enemy->pattern_index == 1) {
        move.has_attack = 1;
        move.damage = 6;
        move.hit_count = 1;
        move.block = 5;

        enemy->pattern_index = 2;
    }
    else {
        move.strength = 2;

        enemy->pattern_index = 0;
    }

    apply_enemy_move(enemy, player, &move);
}

//일반 몬스터 복슬지렁이 함수
static void fuzzy_wurm_crawler_take_turn(Enemy *enemy, Player *player)
{
    EnemyMove move;

    if (enemy == NULL || player == NULL) {
        return;
    }

    clear_enemy_move(&move);

    if (enemy->pattern_index == 0) {
        move.has_attack = 1;
        move.damage = 4;
        move.hit_count = 1;

        enemy->pattern_index = 1;
    }
    else if (enemy->pattern_index == 1) {
        move.strength = 7;

        enemy->pattern_index = 2;
    }
    else if (enemy->pattern_index == 2) {
        move.has_attack = 1;
        move.damage = 4;
        move.hit_count = 1;

        enemy->pattern_index = 3;
    }
    else {
        move.has_attack = 1;
        move.damage = 4;
        move.hit_count = 1;

        enemy->pattern_index = 1;
    }

    apply_enemy_move(enemy, player, &move);
}

//일반 몬스터 압축벌레 함수
static void shrinker_beetle_take_turn(Enemy *enemy, Player *player)
{
    EnemyMove move;

    if (enemy == NULL || player == NULL) {
        return;
    }

    clear_enemy_move(&move);

    if (enemy->turn_count == 0) {
        return;
    }

    if (enemy->pattern_index == 0) {
        move.has_attack = 1;
        move.damage = 7;
        move.hit_count = 1;

        enemy->pattern_index = 1;
    }
    else {
        move.has_attack = 1;
        move.damage = 13;
        move.hit_count = 1;

        enemy->pattern_index = 0;
    }

    apply_enemy_move(enemy, player, &move);
}

//일반 몬스터 오물팽이 함수
static void sludge_spinner_take_turn(Enemy *enemy, Player *player)
{
    EnemyMove move;
    int action;

    if (enemy == NULL || player == NULL) {
        return;
    }

    clear_enemy_move(&move);

    action = enemy->pattern_index;

    if (action == SLUDGE_OIL_SPRAY) {
        move.has_attack = 1;
        move.damage = 8;
        move.hit_count = 1;
        move.weak = 1;
    }
    else if (action == SLUDGE_SLAM) {
        move.has_attack = 1;
        move.damage = 11;
        move.hit_count = 1;
    }
    else {
        move.has_attack = 1;
        move.damage = 6;
        move.hit_count = 1;
        move.strength = 3;
    }

    enemy->special_state = action;
    enemy->pattern_index = choose_next_sludge_spinner_action(enemy->special_state);

    apply_enemy_move(enemy, player, &move);
}

//장수아귀 직전행동을 제외한 2개중 하나를 랜덤으로 고르는 함수
static int choose_next_mawler_action(int previous_action, int roar_used)
{
    int candidates[3];
    int count;

    count = 0;

    if (previous_action != MAWLER_CLAW) {
        candidates[count] = MAWLER_CLAW;
        count++;
    }

    if (previous_action != MAWLER_RIP_AND_TEAR) {
        candidates[count] = MAWLER_RIP_AND_TEAR;
        count++;
    }

    if (!roar_used) {
        candidates[count] = MAWLER_ROAR;
        count++;
    }

    if (count <= 0) {
        return MAWLER_CLAW;
    }

    return candidates[rand() % count];
}

//일반 몬스터 장수아귀 함수
static void mawler_take_turn(Enemy *enemy, Player *player)
{
    EnemyMove move;
    int action;
    int previous_action;
    int roar_used;

    if (enemy == NULL || player == NULL) {
        return;
    }

    clear_enemy_move(&move);

    action = enemy->pattern_index;

    if (enemy->special_state >= 10) {
        roar_used = 1;
    } else {
        roar_used = 0;
    }

    if (action == MAWLER_CLAW) {
        move.has_attack = 1;
        move.damage = 4;
        move.hit_count = 2;
    }
    else if (action == MAWLER_RIP_AND_TEAR) {
        move.has_attack = 1;
        move.damage = 14;
        move.hit_count = 1;
    }
    else {
        move.vulnerable = 3;
        roar_used = 1;
    }

    apply_enemy_move(enemy, player, &move);

    previous_action = action;
    enemy->special_state = previous_action + roar_used * 10;
    enemy->pattern_index = choose_next_mawler_action(previous_action, roar_used);
}

//오물팽이 직전행동을 제외한 2개 중 하나를 랜덤으로 고르는 함수
static int choose_next_sludge_spinner_action(int previous_action)
{
    int candidates[2];
    int count;
    int i;

    count = 0;

    for (i = 0; i < 3; i++) {
        if (i != previous_action) {
            candidates[count] = i;
            count++;
        }
    }

    return candidates[rand() % count];
}

//일반 몬스터 잉클릿 함수
static void inlet_take_turn(Enemy *enemy, Player *player)
{
    EnemyMove move;
    int action;

    if (enemy == NULL || player == NULL) {
        return;
    }

    clear_enemy_move(&move);

    action = enemy->pattern_index;

    if (action == 0) {
        move.has_attack = 1;
        move.damage = 3;
        move.hit_count = 1;
    }
    else if (action == 1) {
        move.has_attack = 1;
        move.damage = 2;
        move.hit_count = 3;
    }
    else {
        move.has_attack = 1;
        move.damage = 10;
        move.hit_count = 1;
    }

    apply_enemy_move(enemy, player, &move);

    enemy->pattern_index = rand() % 3;
}

//일반 몬스터 큐브형 구조체 함수
static void cubex_construct_take_turn(Enemy *enemy, Player *player)
{
    EnemyMove move;
    int action;

    if (enemy == NULL || player == NULL) {
        return;
    }

    clear_enemy_move(&move);

    action = enemy->pattern_index;

    if (action == 0) {
        move.strength = 2;
    }
    else if (action == 1 || action == 2) {
        move.has_attack = 1;
        move.damage = 7;
        move.hit_count = 1;
        move.strength = 2;
    }
    else if (action == 3) {
        move.has_attack = 1;
        move.damage = 5;
        move.hit_count = 2;
    }
    else {
        move.block = 15;
    }

    apply_enemy_move(enemy, player, &move);

    enemy->pattern_index++;

    if (enemy->pattern_index > 4) {
        enemy->pattern_index = 0;
    }
}

//적 버프 감소함수를 위한 보조함수
static void decrease_enemy_positive_value(int *value)
{
    if (value == NULL) {
        return;
    }

    if (*value > 0) {
        (*value)--;
    }
}

//적 상태 버프 감소 함수
static void decrease_enemy_turn_statuses(Enemy *enemy)
{
    if (enemy == NULL) {
        return;
    }

    decrease_enemy_positive_value(&enemy->weak);
    decrease_enemy_positive_value(&enemy->vulnerable);
}

//일반 몬스터 나뭇잎 슬라임 함수 
static void leaf_slime_take_turn(Enemy *enemy, Player *player)
{
    EnemyMove move;
    Card goop;

    if (enemy == NULL || player == NULL) {
        return;
    }

    clear_enemy_move(&move);

    if (enemy->pattern_index == 0) {
        goop = create_goop_card();
        add_card_to_discard(player, goop);

        goop = create_goop_card();
        add_card_to_discard(player, goop);
    } else {
        move.has_attack = 1;
        move.damage = 8;
        move.hit_count = 1;

        apply_enemy_move(enemy, player, &move);
    }

    enemy->pattern_index = 1 - enemy->pattern_index;
}

//가지 슬라임 다음 패턴 판정 함수
static int choose_next_twig_slime_action(int previous_action, int streak)
{
    if (previous_action == 0 && streak >= 1) {
        return 1;
    }

    if (previous_action == 1 && streak >= 2) {
        return 0;
    }

    return rand() % 2;
}

//일반 몬스터 가지 슬라임 함수
static void twig_slime_take_turn(Enemy *enemy, Player *player)
{
    EnemyMove move;
    Card goop;
    int action;
    int previous_action;
    int previous_streak;
    int next_streak;

    if (enemy == NULL || player == NULL) {
        return;
    }

    clear_enemy_move(&move);

    action = enemy->pattern_index;

    if (action == 0) {
        goop = create_goop_card();
        add_card_to_discard(player, goop);
    } else {
        move.has_attack = 1;
        move.damage = 11;
        move.hit_count = 1;
        apply_enemy_move(enemy, player, &move);
    }

    previous_action = -1;
    previous_streak = 0;

    if (enemy->special_state >= 0) {
        previous_action = enemy->special_state % 10;
        previous_streak = enemy->special_state / 10;
    }

    if (action == previous_action) {
        next_streak = previous_streak + 1;
    } else {
        next_streak = 1;
    }

    enemy->special_state = action + next_streak * 10;
    enemy->pattern_index = choose_next_twig_slime_action(action, next_streak);
}

//엘리트 몬스터 옛 시대의 우상 함수
static void bygone_effigy_take_turn(Enemy *enemy, Player *player)
{
    EnemyMove move;

    if (enemy == NULL || player == NULL) {
        return;
    }

    clear_enemy_move(&move);

    if (enemy->pattern_index == 0) {
        enemy->pattern_index = 1;
    }
    else if (enemy->pattern_index == 1) {
        move.strength = 10;
        enemy->pattern_index = 2;
        apply_enemy_move(enemy, player, &move);
    }
    else {
        move.has_attack = 1;
        move.damage = 13;
        move.hit_count = 1;
        apply_enemy_move(enemy, player, &move);
    }
}

//옛 시대의 우상 둔화 초기화 함수
void reset_bygone_effigy_slow(Enemy enemies[], int enemy_count)
{
    int i;

    if (enemies == NULL || enemy_count <= 0) {
        return;
    }

    if (enemy_count > MAX_ENEMIES) {
        enemy_count = MAX_ENEMIES;
    }

    for (i = 0; i < enemy_count; i++) {
        if (enemies[i].id == ENEMY_BYGONE_EFFIGY) {
            enemies[i].special_state = 0;
        }
    }
}

//엘리트 몬스터 새도니스 함수
static void byrdonis_take_turn(Enemy *enemy, Player *player)
{
    EnemyMove move;

    if (enemy == NULL || player == NULL) {
        return;
    }

    clear_enemy_move(&move);

    if (enemy->pattern_index == 0) {
        move.has_attack = 1;
        move.damage = 17;
        move.hit_count = 1;

        enemy->pattern_index = 1;
    } else {
        move.has_attack = 1;
        move.damage = 3;
        move.hit_count = 3;

        enemy->pattern_index = 0;
    }

    apply_enemy_move(enemy, player, &move);

    enemy->strength++;
}

//엘리트 몬스터 공포장어 함수
static void terror_eel_take_turn(Enemy *enemy, Player *player)
{
    EnemyMove move;
    int damage_bonus;

    if (enemy == NULL || player == NULL) {
        return;
    }

    clear_enemy_move(&move);

    if (enemy->special_state & TERROR_EEL_STUN_PENDING) {
        enemy->special_state &= ~TERROR_EEL_STUN_PENDING;
        enemy->special_state |= TERROR_EEL_TERROR_PENDING;
        enemy->pattern_index = TERROR_EEL_TERROR;
        return;
    }

    if (enemy->special_state & TERROR_EEL_TERROR_PENDING) {
        move.vulnerable = 99;

        enemy->special_state &= ~TERROR_EEL_TERROR_PENDING;
        enemy->pattern_index = TERROR_EEL_CRASH;

        apply_enemy_move(enemy, player, &move);
        return;
    }

    damage_bonus = 0;

    if (enemy->special_state & TERROR_EEL_VIGOR_READY) {
        damage_bonus = 6;
    }

    if (enemy->pattern_index == TERROR_EEL_CRASH) {
        move.has_attack = 1;
        move.damage = 16 + damage_bonus;
        move.hit_count = 1;

        enemy->special_state &= ~TERROR_EEL_VIGOR_READY;
        enemy->pattern_index = TERROR_EEL_THRASH;
    }
    else {
        move.has_attack = 1;
        move.damage = 3 + damage_bonus;
        move.hit_count = 3;

        enemy->special_state &= ~TERROR_EEL_VIGOR_READY;
        enemy->pattern_index = TERROR_EEL_CRASH;

        apply_enemy_move(enemy, player, &move);

        enemy->special_state |= TERROR_EEL_VIGOR_READY;
        return;
    }

    apply_enemy_move(enemy, player, &move);
}

//보스 밴텀 함수
static void vantom_take_turn(Enemy *enemy, Player *player)
{
    EnemyMove move;
    Card wound;
    int i;

    if (enemy == NULL || player == NULL) {
        return;
    }

    clear_enemy_move(&move);

    if (enemy->pattern_index == 0) {
        move.has_attack = 1;
        move.damage = 7;
        move.hit_count = 1;

        enemy->pattern_index = 1;
        apply_enemy_move(enemy, player, &move);
        return;
    }

    if (enemy->pattern_index == 1) {
        move.has_attack = 1;
        move.damage = 6;
        move.hit_count = 2;

        enemy->pattern_index = 2;
        apply_enemy_move(enemy, player, &move);
        return;
    }

    if (enemy->pattern_index == 2) {
    move.has_attack = 1;
    move.damage = 27;
    move.hit_count = 1;

    apply_enemy_move(enemy, player, &move);

    wound = create_wound_card();

    for (i = 0; i < 3; i++) {
        add_card_to_discard(player, wound);
    }

    enemy->pattern_index = 3;
    return;
}

    move.strength = 2;

    enemy->pattern_index = 0;
    apply_enemy_move(enemy, player, &move);
}
