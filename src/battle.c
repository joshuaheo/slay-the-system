#include <locale.h>
#include <stdlib.h>
#include "battle.h"
#include "card.h"
#include "relic.h"
#include "enemy.h"

//공포장어용 매크로
#if 1
#define TERROR_EEL_CRASH 0
#define TERROR_EEL_THRASH 1
#define TERROR_EEL_TERROR 2
#define TERROR_EEL_STUN 3

#define TERROR_EEL_TERROR_USED 1
#define TERROR_EEL_VIGOR_READY 2
#define TERROR_EEL_STUN_PENDING 4
#define TERROR_EEL_TERROR_PENDING 8
#endif
static int g_shrink_effect_active = 0;
static void remove_card_from_hand(Player *player, int hand_index);
static int has_active_shrink_effect(Enemy enemies[], int enemy_count);
static void increase_bygone_effigy_slow(Enemy enemies[], int enemy_count);
static void lose_player_hp_by_card_or_power(Player *player, int amount);

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

//카드 특수 조건 검사
static int can_use_special_card(Player *player, Card card)
{
    if (player == NULL) {
        return 0;
    }

    if (card.special == SPECIAL_PACT_END) {
        return player->exhaust_count >= 3;
    }

    return 1;
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

//공포장어 체력 50프로 처리 함수
static void check_terror_eel_half_hp_trigger(Enemy *enemy)
{
    if (enemy == NULL) {
        return;
    }

    if (enemy->id != ENEMY_TERROR_EEL) {
        return;
    }

    if (enemy->hp <= 0) {
        return;
    }

    if (enemy->hp * 2 > enemy->max_hp) {
        return;
    }

    if (enemy->special_state & TERROR_EEL_TERROR_USED) {
        return;
    }

    enemy->special_state |= TERROR_EEL_TERROR_USED;
    enemy->special_state |= TERROR_EEL_STUN_PENDING;
    enemy->special_state &= ~TERROR_EEL_TERROR_PENDING;
    enemy->pattern_index = TERROR_EEL_STUN;
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
    if (enemy->id == ENEMY_BYGONE_EFFIGY && enemy->special_state > 0) {
    final_damage = final_damage * (100 + enemy->special_state * 10) / 100;
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
    if ((enemy->id == ENEMY_INLET || enemy->id == ENEMY_VANTOM) &&enemy->special_state > 0 &&final_damage > 0) {
    final_damage = 1;
    enemy->special_state--;
}
    enemy->hp -= final_damage;

    if (enemy->hp < 0) {
        enemy->hp = 0;
    }
    check_terror_eel_half_hp_trigger(enemy);
}

//적에게 상태이상을 부여하는 함수
static void apply_enemy_status_effect(Enemy *enemy, Card card)
{
    int artifact_blocked;

    if (enemy == NULL) {
        return;
    }

    if (enemy->hp <= 0) {
        return;
    }

    artifact_blocked = 0;

    if (card.weak > 0) {
        if (enemy->id == ENEMY_CUBEX_CONSTRUCT &&
            enemy->special_state > 0 &&
            artifact_blocked == 0) {
            enemy->special_state--;
            artifact_blocked = 1;
        } else {
            enemy->weak += card.weak;
        }
    }

    if (card.vulnerable > 0) {
        if (enemy->id == ENEMY_CUBEX_CONSTRUCT &&
            enemy->special_state > 0 &&
            artifact_blocked == 0) {
            enemy->special_state--;
            artifact_blocked = 1;
        } else {
            enemy->vulnerable += card.vulnerable;
        }
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

//손패 전체 소멸 함수
static int exhaust_all_cards_in_hand(Player *player)
{
    int exhausted_count;

    if (player == NULL) {
        return 0;
    }

    exhausted_count = 0;

    while (player->hand_count > 0) {
        if (player->exhaust_count >= MAX_DECK_SIZE) {
            break;
        }

        player->exhaust_pile[player->exhaust_count] = player->hand[0];
        player->exhaust_count++;
        player->exhausted_this_turn++;

        remove_card_from_hand(player, 0);
        exhausted_count++;
    }

    return exhausted_count;
}

//지옥불 특수 효과 함수
static void apply_fiend_fire_effect(Player *player, Enemy enemies[], int enemy_count, Card card, int target_index)
{
    int i;
    int exhausted_count;

    if (player == NULL || enemies == NULL) {
        return;
    }

    if (enemy_count <= 0) {
        return;
    }

    if (!is_valid_enemy_target(enemies, enemy_count, target_index)) {
        return;
    }

    exhausted_count = exhaust_all_cards_in_hand(player);

if (card.exhaust) {
    exhausted_count++;
}

for (i = 0; i < exhausted_count; i++) {
    if (enemies[target_index].hp <= 0) {
        break;
    }

    deal_damage_to_enemy(player, &enemies[target_index], card.damage);
}
}

//파워카드 보조 함수
static int add_active_power(Player *player, ActivePower power)
{
    if (player == NULL) {
        return 0;
    }

    if (player->active_power_count >= MAX_ACTIVE_POWERS) {
        return 0;
    }

    player->active_powers[player->active_power_count] = power;
    player->active_power_count++;

    return 1;
}

//턴 시작할 때 효과 나오는 파워카드 함수
void apply_player_turn_start_powers(Player *player, Enemy enemies[], int enemy_count)
{
    int i;
    int j;

    if (player == NULL) {
        return;
    }

    for (i = 0; i < player->active_power_count; i++) {
        if (player->active_powers[i].trigger != POWER_TRIGGER_TURN_START) {
            continue;
        }

        if (player->active_powers[i].hp_loss > 0) {
            lose_player_hp_by_card_or_power(player, player->active_powers[i].hp_loss);
        }
    

        if (player->active_powers[i].block > 0) {
            player->block += player->active_powers[i].block;
        }

        if (player->active_powers[i].strength > 0) {
            player->strength += player->active_powers[i].strength;
        }

        if (player->active_powers[i].energy > 0) {
            player->energy += player->active_powers[i].energy;
        }

        if (player->active_powers[i].draw > 0) {
            draw_cards(player, player->active_powers[i].draw);
        }

        if (player->active_powers[i].damage > 0 && enemies != NULL && enemy_count > 0) {
            for (j = 0; j < enemy_count; j++) {
                if (enemies[j].hp > 0) {
                    deal_damage_to_enemy(player, &enemies[j], player->active_powers[i].damage);
                }
            }
        }
    }
}

//핏빛 망토 등록 함수
static void apply_crimson_mantle_effect(Player *player, Card card)
{
    ActivePower power;

    if (player == NULL) {
        return;
    }

    power.special = SPECIAL_CRIMSON_MANTLE;
    power.trigger = POWER_TRIGGER_TURN_START;

    power.hp_loss = 1;
    power.block = card.block;
    power.strength = 0;
    power.weak = 0;
    power.vulnerable = 0;
    power.damage = 0;
    power.energy = 0;
    power.draw = 0;

    add_active_power(player, power);
}

//불의 심장 등록 함수
static void apply_pyre_effect(Player *player)
{
    ActivePower power;

    if (player == NULL) {
        return;
    }

    power.special = SPECIAL_PYRE;
    power.trigger = POWER_TRIGGER_TURN_START;

    power.hp_loss = 0;
    power.block = 0;
    power.strength = 0;
    power.weak = 0;
    power.vulnerable = 0;
    power.damage = 0;
    power.energy = 2;
    power.draw = 0;

    add_active_power(player, power);
}

//악마의 형상 등록 함수
static void apply_demon_form_effect(Player *player, Card card)
{
    ActivePower power;

    if (player == NULL) {
        return;
    }

    power.special = SPECIAL_DEMON_FORM;
    power.trigger = POWER_TRIGGER_TURN_START;

    power.hp_loss = 0;
    power.block = 0;
    power.strength = card.strength;
    power.weak = 0;
    power.vulnerable = 0;
    power.damage = 0;
    power.energy = 0;
    power.draw = 0;

    add_active_power(player, power);
}

//조약의 끝 등록 함수
static void apply_pact_end_effect(Player *player, Enemy enemies[], int enemy_count, Card card)
{
    int i;

    if (player == NULL || enemies == NULL) {
        return;
    }

    if (enemy_count <= 0) {
        return;
    }

    for (i = 0; i < enemy_count; i++) {
        if (enemies[i].hp > 0) {
            deal_damage_to_enemy(player, &enemies[i], card.damage);
        }
    }
}

//제압 카드 함수
static void apply_dominate_effect(Player *player, Enemy enemies[], int enemy_count, Card card, int target_index)
{
    if (player == NULL || enemies == NULL) {
        return;
    }

    if (enemy_count <= 0) {
        return;
    }

    if (!is_valid_enemy_target(enemies, enemy_count, target_index)) {
        return;
    }

    apply_enemy_status_effect(&enemies[target_index], card);

    player->strength += enemies[target_index].vulnerable;
}

//잊힌 의식 카드 함수
static void apply_forgotten_ritual_effect(Player *player, Card card)
{
    if (player == NULL) {
        return;
    }

    if (player->exhausted_this_turn > 0) {
        player->energy += card.energy;
    }
}

//잿빛 타격 카드 함수
static void apply_ashen_strike_effect(Player *player, Enemy enemies[], int enemy_count, Card card, int target_index)
{
    int damage;

    if (player == NULL || enemies == NULL) {
        return;
    }

    if (enemy_count <= 0) {
        return;
    }

    if (!is_valid_enemy_target(enemies, enemy_count, target_index)) {
        return;
    }

    damage = card.damage + player->exhaust_count * 3;

    deal_damage_to_enemy(player, &enemies[target_index], damage);
}

//악의 카드 함수
static void apply_spite_effect(Player *player, Enemy enemies[], int enemy_count, Card card, int target_index)
{
    int i;
    int hit_count;

    if (player == NULL || enemies == NULL) {
        return;
    }

    if (enemy_count <= 0) {
        return;
    }

    if (!is_valid_enemy_target(enemies, enemy_count, target_index)) {
        return;
    }

    hit_count = 1;

    if (player->hp_lost_this_turn > 0) {
        hit_count = 2;
    }

    for (i = 0; i < hit_count; i++) {
        if (enemies[target_index].hp <= 0) {
            break;
        }

        deal_damage_to_enemy(player, &enemies[target_index], card.damage);
    }
}

//녹아내리는 주먹 카드 함수
static void apply_molten_fist_effect(Player *player, Enemy enemies[], int enemy_count, Card card, int target_index)
{
    if (player == NULL || enemies == NULL) {
        return;
    }

    if (enemy_count <= 0) {
        return;
    }

    if (!is_valid_enemy_target(enemies, enemy_count, target_index)) {
        return;
    }

    deal_damage_to_enemy(player, &enemies[target_index], card.damage);

    if (enemies[target_index].hp > 0 && enemies[target_index].vulnerable > 0) {
        enemies[target_index].vulnerable *= 2;
    }
}

//카드 효과 전체를 실제로 적용하는 중심 함수
static void apply_card_effect(Player *player, Enemy enemies[], int enemy_count, Card card, int target_index)
{
    int i;

    if (player == NULL) {
        return;
    }
    if (card.special == SPECIAL_MOLTEN_FIST) {
        apply_molten_fist_effect(player, enemies, enemy_count, card, target_index);
        return;
    }
    if (card.special == SPECIAL_FIEND_FIRE) {
        apply_fiend_fire_effect(player, enemies, enemy_count, card, target_index);
        return;
    }
    if (card.special == SPECIAL_SPITE) {
        apply_spite_effect(player, enemies, enemy_count, card, target_index);
        return;
    }
    if (card.special == SPECIAL_FORGOTTEN_RITUAL) {
        apply_forgotten_ritual_effect(player, card);
        return;
    }
    if (card.special == SPECIAL_ASHEN_STRIKE) {
        apply_ashen_strike_effect(player, enemies, enemy_count, card, target_index);
        return;
    }

    if (card.special == SPECIAL_CRIMSON_MANTLE) {
        apply_crimson_mantle_effect(player, card);
        return;
    }

    if (card.special == SPECIAL_PYRE) {
        apply_pyre_effect(player);
        return;
    }
    if (card.special == SPECIAL_DEMON_FORM) {
        apply_demon_form_effect(player, card);
        return;
    }
    if (card.special == SPECIAL_PACT_END) {
        apply_pact_end_effect(player, enemies, enemy_count, card);
        return;
    }
    if (card.special == SPECIAL_DOMINATE) {
        apply_dominate_effect(player, enemies, enemy_count, card, target_index);
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
            player->exhausted_this_turn++;
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

//이번턴에 체력을 잃었는지 체크하는 함수
static void lose_player_hp_by_card_or_power(Player *player, int amount)
{
    if (player == NULL) {
        return;
    }

    if (amount <= 0) {
        return;
    }

    if (player->hp <= 0) {
        return;
    }

    player->hp -= amount;
    player->hp_lost_this_turn += amount;

    if (player->hp < 0) {
        player->hp = 0;
    }
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

    if (!can_use_special_card(player, card)) {
        return 0;
    }
    if (!can_use_card_on_target(card, enemies, enemy_count, target_index)) {
        return 0;
    }

    player->energy -= card.cost;

    if (card.hp_loss > 0) {
        lose_player_hp_by_card_or_power(player, card.hp_loss);
    }

    remove_card_from_hand(player, hand_index);

    if (player->hp <= 0) {
        move_used_card(player, card);
        return 1;
    }
    increase_bygone_effigy_slow(enemies, enemy_count);

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

//옛 시대의 우상 전용 버프 둔화 처리 함수
static void increase_bygone_effigy_slow(Enemy enemies[], int enemy_count)
{
    int i;

    if (enemies == NULL || enemy_count <= 0) {
        return;
    }

    if (enemy_count > MAX_ENEMIES) {
        enemy_count = MAX_ENEMIES;
    }

    for (i = 0; i < enemy_count; i++) {
        if (enemies[i].id == ENEMY_BYGONE_EFFIGY && enemies[i].hp > 0) {
            enemies[i].special_state++;
        }
    }
}

