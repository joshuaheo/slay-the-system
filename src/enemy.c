#include <string.h>

#include "enemy.h"
#include "game.h"

//슬라임 생성 함수
void init_slime(Enemy *enemy)
{
    if (enemy == NULL) {
        return;
    }

    strncpy(enemy->name, "Slime", MAX_NAME_LEN - 1);
    enemy->name[MAX_NAME_LEN - 1] = '\0';

    enemy->max_hp = 30;
    enemy->hp = 30;
    enemy->block = 0;

    enemy->strength = 0;
    enemy->weak = 0;
    enemy->vulnerable = 0;

    enemy->damage = 6;
}

//적이 살아있는지 확인하는 함수
int is_enemy_alive(const Enemy *enemy)
{
    if (enemy == NULL) {
        return 0;
    }

    return enemy->hp > 0;
}

//적의 공격 최종데미지 계산함수
int calculate_enemy_attack_damage(const Enemy *enemy, const Player *player)
{
    int damage;

    if (enemy == NULL || player == NULL) {
        return 0;
    }

    if (enemy->hp <= 0) {
        return 0;
    }

    damage = enemy->damage + enemy->strength;

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

    damage = calculate_enemy_attack_damage(enemy, player);

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

//임시 적의 패턴 함수
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

        if (is_enemy_alive(&enemies[i])) {
            enemy_attack_player(&enemies[i], player);
        }
    }
}
