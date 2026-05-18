#include "player.h"
#include <stdio.h>

//플레이어 체력을 회복시켜주는 함수
int heal_player(Player *player, int amount) {
    int before_hp;

    if (player == NULL || amount <= 0) {
        return 0;
    }

    if (player->hp >= player->max_hp) {
        return 0;
    }

    before_hp = player->hp;

    player->hp += amount;

    if (player->hp > player->max_hp) {
        player->hp = player->max_hp;
    }

    return player->hp - before_hp;
}

//덱에서 카드를 제거가능한 조건인지 확인하는 함수
int can_remove_card_from_deck(const Player *player) {
    if (player == NULL) {
        return 0;
    }

    return player->owned_deck_count > 10;
}

//덱에서 카드를 제거하는 함수
int remove_card_from_deck(Player *player, int index) {
    int i;

    if (player == NULL) {
        return 0;
    }

    if (!can_remove_card_from_deck(player)) {
        return 0;
    }

    if (index < 0 || index >= player->owned_deck_count) {
        return 0;
    }

    for (i = index; i < player->owned_deck_count - 1; i++) {
        player->owned_deck[i] = player->owned_deck[i + 1];
    }

    player->owned_deck_count--;

    return 1;
}
