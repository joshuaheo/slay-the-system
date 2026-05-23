#include <string.h>
#include <stdlib.h>

#include "enemy.h"

#define SLUDGE_OIL_SPRAY 0
#define SLUDGE_SLAM      1
#define SLUDGE_RAGE      2

static void jaw_worm_take_turn(Enemy *enemy, Player *player);
static void seapunk_take_turn(Enemy *enemy, Player *player);
static void fuzzy_wurm_crawler_take_turn(Enemy *enemy, Player *player);
static void shrinker_beetle_take_turn(Enemy *enemy, Player *player);
static void sludge_spinner_take_turn(Enemy *enemy, Player *player);
static int choose_next_sludge_spinner_action(int previous_action);


static void decrease_enemy_positive_value(int *value);
static void decrease_enemy_turn_statuses(Enemy *enemy);

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
    case ENEMY_SLIME:
        enemy->grade = ENEMY_NORMAL;

        strncpy(enemy->name, "슬라임", MAX_NAME_LEN - 1);
        enemy->name[MAX_NAME_LEN - 1] = '\0';

        enemy->max_hp = 30;
        enemy->hp = 30;
        enemy->damage = 6;
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

//직전행동을 제외한 2개 중 하나를 랜덤으로 고르는 함수
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

