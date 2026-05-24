#include <string.h>
#include <stdlib.h>
#include "event.h"
#include "player.h"
#include "save.h"
#include "ui.h"

#define EVENT_COUNT 2

static int run_random_event(GameState *state);
static int run_mutating_forest_event(GameState *state);
static int remove_one_card_by_choice(Player *player, Card *removed_card);

static int has_attack_card_to_corrupt(const Player *player);
static void append_corrupted_tag(Card *card);
static void corrupt_attack_card(Card *card);
static int run_symbiote_event(GameState *state);

//공격카드에 오염을 추가하는 함수
static int has_attack_card_to_corrupt(const Player *player)
{
    int i;

    if (player == NULL) {
        return 0;
    }

    for (i = 0; i < player->owned_deck_count; i++) {
        if (player->owned_deck[i].type == CARD_ATTACK &&
            player->owned_deck[i].damage > 0) {
            return 1;
        }
    }

    return 0;
}

//오염 태그를 붙이는 함수
static void append_corrupted_tag(Card *card)
{
    const char *tag = " [오염]";
    size_t name_len;
    size_t tag_len;

    if (card == NULL) {
        return;
    }

    if (strstr(card->name, tag) != NULL) {
        return;
    }

    name_len = strlen(card->name);
    tag_len = strlen(tag);

    if (name_len + tag_len >= MAX_NAME_LEN) {
        return;
    }

    strcat(card->name, tag);
}

//카드에 오염을 적용하는 함수
static void corrupt_attack_card(Card *card)
{
    if (card == NULL) {
        return;
    }

    if (card->type != CARD_ATTACK || card->damage <= 0) {
        return;
    }

    card->damage = card->damage * 3 / 2 ;
    card->hp_loss += 2;

    append_corrupted_tag(card);
}

//공생체 이벤트 함수
static int run_symbiote_event(GameState *state)
{
    int choice;
    int selected_index;
    Card removed_card;

    if (state == NULL) {
        return 0;
    }

    choice = show_symbiote_event_screen();

    if (choice == 1) {
        if (!has_attack_card_to_corrupt(&state->player)) {
            show_no_attack_card_screen();
            return 1;
        }

        selected_index = show_attack_card_select_screen(&state->player);

        if (selected_index < 0) {
            return 1;
        }

        corrupt_attack_card(&state->player.owned_deck[selected_index]);
        show_card_corrupted_screen(&state->player.owned_deck[selected_index]);

        return 1;
    }

    if (choice == 2) {
        if (!can_remove_card_from_deck(&state->player)) {
            show_card_remove_unavailable_screen();
            return 1;
        }

        selected_index = show_remove_card_screen(&state->player);

        if (selected_index < 0) {
            return 1;
        }

        removed_card = state->player.owned_deck[selected_index];

        if (remove_card_from_deck(&state->player, selected_index)) {
            show_card_removed_screen(&removed_card);
        }

        return 1;
    }

    return 1;
}

//랜덤 이벤트 선택 함수
static int run_random_event(GameState *state)
{
    int event_index;

    if (state == NULL) {
        return 0;
    }

    event_index = rand() % EVENT_COUNT;

    if (event_index == 0) {
        return run_symbiote_event(state);
    }

    return run_mutating_forest_event(state);
}

//카드 1장 제거 보조함수(2장 제거일경우)
static int remove_one_card_by_choice(Player *player, Card *removed_card)
{
    int selected_index;

    if (player == NULL) {
        return 0;
    }

    if (!can_remove_card_from_deck(player)) {
        return 0;
    }

    selected_index = show_remove_card_screen(player);

    if (selected_index < 0) {
        return 0;
    }

    if (removed_card != NULL) {
        *removed_card = player->owned_deck[selected_index];
    }

    return remove_card_from_deck(player, selected_index);
}

//이벤트 스테이지 실행 함수
int run_event_stage(GameState *state)
{
    if (state == NULL) {
        return 0;
    }

    if (!run_random_event(state)) {
        return 0;
    }

    state->floor++;

    if (!save_game(state)) {
        return 0;
    }

    return 1;
}

//변성체의 숲 이벤트 함수
static int run_mutating_forest_event(GameState *state)
{
    int choice;
    int lost_gold;
    int removed_count;
    Card removed_cards[2];

    if (state == NULL) {
        return 0;
    }

    removed_count = 0;

    choice = show_mutating_forest_event_screen();

    if (choice == 1) {
        lost_gold = state->player.gold;
        state->player.gold = 0;

        if (can_remove_card_from_deck(&state->player)) {
            if (remove_one_card_by_choice(&state->player, &removed_cards[removed_count])) {
                removed_count++;
            }
        }

        if (can_remove_card_from_deck(&state->player)) {
            if (remove_one_card_by_choice(&state->player, &removed_cards[removed_count])) {
                removed_count++;
            }
        }

        show_mutating_forest_removed_screen(removed_cards, removed_count, lost_gold);
        return 1;
    }

    if (choice == 2) {
        state->player.max_hp += 5;
        state->player.hp += 5;

        if (state->player.hp > state->player.max_hp) {
            state->player.hp = state->player.max_hp;
        }

        show_max_hp_increased_screen(&state->player, 5);
        return 1;
    }

    return 1;
}
