#include <locale.h>
#include <stdlib.h>
#include "battle.h"
#include "card.h"
#include "relic.h"
#include "enemy.h"

static int g_shrink_effect_active = 0;

static int has_active_shrink_effect(Enemy enemies[], int enemy_count);

//카드가 몇 번 공격하는지 계산하는 함수.
static int get_card_hit_count(const Card *card)
{
    if (card == NULL) {
        return 0;
    }

    if (card->damage <= 0) {
        return 0;
    }

    if (card->hit_count <= 0) {
        return 1;
    }

    return card->hit_count;
}

//TARGET_ENEMY 카드가 지정한 적을 대상으로 사용할 수 있는지 검사하는 함수
static int is_valid_enemy_target(Enemy enemies[], int enemy_count, int target_index)
{
    if (enemies == NULL) {
        return 0;
    }

    if (enemy_count <= 0) {
        return 0;
    }

    if (target_index < 0 || target_index >= enemy_count) {
        return 0;
    }

    if (enemies[target_index].hp <= 0) {
        return 0;
    }

    return 1;
}

//적들 중 살아있는 적이 하나라도 있는지 확인하는 함수
static int has_alive_enemy(Enemy enemies[], int enemy_count)
{
    int i;

    if (enemies == NULL) {
        return 0;
    }

    if (enemy_count <= 0) {
        return 0;
    }

    for (i = 0; i < enemy_count; i++) {
        if (enemies[i].hp > 0) {
            return 1;
        }
    }

    return 0;
}

//살아있는 적들 중 하나를 랜덤으로 골라 그 인덱스를 반환하는 함수
static int get_random_alive_enemy_index(Enemy enemies[], int enemy_count)
{
    int alive_indices[MAX_ENEMIES];
    int alive_count = 0;
    int random_index;
    int i;

    if (enemies == NULL) {
        return -1;
    }

    if (enemy_count <= 0) {
        return -1;
    }

    if (enemy_count > MAX_ENEMIES) {
        enemy_count = MAX_ENEMIES;
    }

    for (i = 0; i < enemy_count; i++) {
        if (enemies[i].hp > 0) {
            alive_indices[alive_count] = i;
            alive_count++;
        }
    }

    if (alive_count <= 0) {
        return -1;
    }

    random_index = rand() % alive_count;

    return alive_indices[random_index];
}

//카드의 target 타입에 따라 이 카드를 사용할 수 있는지 판단하는 함수.
static int can_use_card_on_target(Card card, Enemy enemies[], int enemy_count, int target_index)
{
    if (card.target == TARGET_SELF) {
        return 1;
    }

    if (card.target == TARGET_ENEMY) {
        return is_valid_enemy_target(enemies, enemy_count, target_index);
    }

    if (card.target == TARGET_ALL_ENEMIES) {
        return has_alive_enemy(enemies, enemy_count);
    }

    if (card.target == TARGET_RANDOM_ENEMY) {
        return has_alive_enemy(enemies, enemy_count);
    }

    return 0;
}

//적에게 주는 최종데미지를 계산하는 함수
static void deal_damage_to_enemy(Player *player, Enemy *enemy, int damage)
{
    int final_damage;

    if (player == NULL || enemy == NULL) {
        return;
    }

    if (damage <= 0) {
        return;
    }

    if (enemy->hp <= 0) {
        return;
    }

    final_damage = damage + player->strength;

    if (final_damage < 0) {
        final_damage = 0;
    }

    if (player->weak > 0) {
        final_damage = final_damage * 3 / 4;
    }

    if (g_shrink_effect_active) {
        final_damage = final_damage * 7 / 10;
    }

    if (enemy->vulnerable > 0) {
        final_damage = final_damage * 3 / 2;
    }

    if (enemy->block > 0) {
        if (enemy->block >= final_damage) {
            enemy->block -= final_damage;
            final_damage = 0;
        } else {
            final_damage -= enemy->block;
            enemy->block = 0;
        }
    }

    enemy->hp -= final_damage;

    if (enemy->hp < 0) {
        enemy->hp = 0;
    }
}

//적에게 상태이상을 부여하는 함수
static void apply_enemy_status_effect(Enemy *enemy, Card card)
{
    if (enemy == NULL) {
        return;
    }

    if (enemy->hp <= 0) {
        return;
    }

    if (card.weak > 0) {
        enemy->weak += card.weak;
    }

    if (card.vulnerable > 0) {
        enemy->vulnerable += card.vulnerable;
    }
}

//지정된 적 하나에게 카드 효과를 적용하는 함수
static void apply_enemy_effect(Player *player, Enemy *enemy, Card card)
{
    int i;
    int hit_count;

    if (player == NULL || enemy == NULL) {
        return;
    }

    if (enemy->hp <= 0) {
        return;
    }

    hit_count = get_card_hit_count(&card);

    for (i = 0; i < hit_count; i++) {
        deal_damage_to_enemy(player, enemy, card.damage);
    }

    apply_enemy_status_effect(enemy, card);
}

//무작위 적 대상 카드 효과를 적용하는 함수
static void apply_random_enemy_effect(Player *player, Enemy enemies[], int enemy_count, Card card)
{
    int i;
    int hit_count;
    int target_index;

    if (player == NULL || enemies == NULL) {
        return;
    }

    if (enemy_count <= 0) {
        return;
    }

    if (enemy_count > MAX_ENEMIES) {
        enemy_count = MAX_ENEMIES;
    }

    hit_count = get_card_hit_count(&card);

    for (i = 0; i < hit_count; i++) {
        target_index = get_random_alive_enemy_index(enemies, enemy_count);

        if (target_index < 0) {
            return;
        }

        deal_damage_to_enemy(player, &enemies[target_index], card.damage);
        apply_enemy_status_effect(&enemies[target_index], card);
    }
}

//카드 효과 전체를 실제로 적용하는 중심 함수
static void apply_card_effect(Player *player, Enemy enemies[], int enemy_count, Card card, int target_index)
{
    int i;

    if (player == NULL) {
        return;
    }

    if (card.block > 0) {
        player->block += card.block;
    }

    if (card.strength > 0) {
        player->strength += card.strength;
    }

    if (card.energy > 0) {
        player->energy += card.energy;
    }

    if (card.target == TARGET_SELF) {
        return;
    }

    if (enemies == NULL || enemy_count <= 0) {
        return;
    }

    if (card.target == TARGET_ENEMY) {
        if (is_valid_enemy_target(enemies, enemy_count, target_index)) {
            apply_enemy_effect(player, &enemies[target_index], card);
        }

        return;
    }

    if (card.target == TARGET_ALL_ENEMIES) {
        for (i = 0; i < enemy_count; i++) {
            if (enemies[i].hp > 0) {
                apply_enemy_effect(player, &enemies[i], card);
            }
        }

        return;
    }

    if (card.target == TARGET_RANDOM_ENEMY) {
        apply_random_enemy_effect(player, enemies, enemy_count, card);
        return;
    }
}

//사용한 카드를 손패에서 제거하는 함수
static void remove_card_from_hand(Player *player, int index)
{
    int i;

    if (player == NULL) {
        return;
    }

    if (index < 0 || index >= player->hand_count) {
        return;
    }

    for (i = index; i < player->hand_count - 1; i++) {
        player->hand[i] = player->hand[i + 1];
    }

    player->hand_count--;
}

//사용한 카드를 버림더미 또는 소멸더미로 보내는 함수
static void move_used_card(Player *player, Card card)
{
    if (player == NULL) {
        return;
    }

    if (card.exhaust) {
        if (player->exhaust_count < MAX_DECK_SIZE) {
            player->exhaust_pile[player->exhaust_count] = card;
            player->exhaust_count++;
        }

        return;
    }

    if (player->discard_count < MAX_DECK_SIZE) {
        player->discard_pile[player->discard_count] = card;
        player->discard_count++;
    }
}

//양수 값을 1줄이는 보조함수
void decrease_positive_value(int *value)
{
    if (value == NULL) {
        return;
    }

    if (*value > 0) {
        (*value)--;
    }
}

//플레이어 버프 감소 함수
void decrease_player_turn_statuses(Player *player)
{
    if (player == NULL) {
        return;
    }

    decrease_positive_value(&player->weak);
    decrease_positive_value(&player->vulnerable);
}

//카드 실제 사용 함수
int play_card(Player *player, Enemy enemies[], int enemy_count, int hand_index, int target_index)
{
    Card card;

    if (player == NULL) {
        return 0;
    }

    if (player->hp <= 0) {
        return 0;
    }

    if (hand_index < 0 || hand_index >= player->hand_count) {
        return 0;
    }

    card = player->hand[hand_index];

    if (card.cost < 0) {
        return 0;
    }

    if (player->energy < card.cost) {
        return 0;
    }

    if (!can_use_card_on_target(card, enemies, enemy_count, target_index)) {
        return 0;
    }

    player->energy -= card.cost;

    if (card.hp_loss > 0) {
        player->hp -= card.hp_loss;

        if (player->hp < 0) {
            player->hp = 0;
        }
    }

    remove_card_from_hand(player, hand_index);

    if (player->hp <= 0) {
        move_used_card(player, card);
        return 1;
    }

    g_shrink_effect_active = has_active_shrink_effect(enemies, enemy_count);

    apply_card_effect(player, enemies, enemy_count, card, target_index);

    g_shrink_effect_active = 0;

    if (card.draw > 0) {
        draw_cards(player, card.draw);
    }

    move_used_card(player, card);

    return 1;
}

//적이 모두 죽었는지 확인하는 함수
int are_all_enemies_dead(Enemy enemies[], int enemy_count)
{
    int i;

    if (enemies == NULL || enemy_count <= 0) {
        return 1;
    }

    for (i = 0; i < enemy_count; i++) {
        if (enemies[i].hp > 0) {
            return 0;
        }
    }

    return 1;
}

//배틀 결과를 판정하는 함수
BattleResult check_battle_result(Player *player, Enemy enemies[], int enemy_count)
{
    if (player == NULL) {
        return BATTLE_LOSE;
    }

    if (player->hp <= 0) {
        return BATTLE_LOSE;
    }

    if (are_all_enemies_dead(enemies, enemy_count)) {
        return BATTLE_WIN;
    }

    return BATTLE_CONTINUE;
}

//압축벌레가 살아있고 첫턴이 지났는지 확인하는 함수
static int has_active_shrink_effect(Enemy enemies[], int enemy_count)
{
    int i;

    if (enemies == NULL || enemy_count <= 0) {
        return 0;
    }

    if (enemy_count > MAX_ENEMIES) {
        enemy_count = MAX_ENEMIES;
    }

    for (i = 0; i < enemy_count; i++) {
        if (enemies[i].id == ENEMY_SHRINKER_BEETLE &&
            enemies[i].hp > 0 &&
            enemies[i].turn_count > 0) {
            return 1;
        }
    }

    return 0;
}
