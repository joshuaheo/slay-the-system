#include <string.h>
#include <stdlib.h>
#include "event.h"
#include "player.h"
#include "save.h"
#include "ui.h"
#include "card.h"

#define EVENT_COUNT 5

#define EVENT_SYMBIOTE 0
#define EVENT_MUTATING_FOREST 1
#define EVENT_JUNGLE_MAZE 2
#define EVENT_AMALGAMATOR 3
#define EVENT_SUNKEN_TREASURY 4

static int run_random_event(GameState *state);
static int run_mutating_forest_event(GameState *state);
static int remove_one_card_by_choice(Player *player, Card *removed_card);

static int has_attack_card_to_corrupt(const Player *player);
static void append_corrupted_tag(Card *card);
static void corrupt_attack_card(Card *card);
static int run_symbiote_event(GameState *state);
static int run_jungle_maze_event(GameState *state);

static int run_event_by_id(GameState *state, int event_id);
static int is_event_available(const GameState *state, int event_id);

static int run_amalgamator_event(GameState *state);
static int can_run_amalgamator_event(const Player *player);
static int count_cards_by_name_prefix(const Player *player, const char *prefix);
static int remove_first_card_by_name_prefix(Player *player, const char *prefix);
static int run_sunken_treasury_event(GameState *state);

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
    int available_events[EVENT_COUNT];
    int available_count;
    int i;
    int selected_event;

    if (state == NULL) {
        return 0;
    }

    available_count = 0;

    for (i = 0; i < EVENT_COUNT; i++) {
        if (is_event_available(state, i)) {
            available_events[available_count] = i;
            available_count++;
        }
    }

    if (available_count <= 0) {
        return 1;
    }

    selected_event = available_events[rand() % available_count];

    return run_event_by_id(state, selected_event);
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

//정글 미로 탐험 이벤트 함수
static int run_jungle_maze_event(GameState *state)
{
    int choice;
    int gold_gain;
    int hp_loss;

    if (state == NULL) {
        return 0;
    }

    choice = show_jungle_maze_event_screen();

    if (choice == 1) {
        gold_gain = 135 + rand() % 31;
        hp_loss = 10;

        state->player.gold += gold_gain;
        state->player.hp -= hp_loss;

        if (state->player.hp < 1) {
            state->player.hp = 1;
        }

        show_jungle_maze_result_screen(choice, gold_gain, hp_loss, &state->player);
        return 1;
    }

    if (choice == 2) {
        gold_gain = 35 + rand() % 31;
        hp_loss = 0;

        state->player.gold += gold_gain;

        show_jungle_maze_result_screen(choice, gold_gain, hp_loss, &state->player);
        return 1;
    }

    return 1;
}

//이벤트 가능한지 확인
static int is_event_available(const GameState *state, int event_id)
{
    if (state == NULL) {
        return 0;
    }

    if (event_id == EVENT_AMALGAMATOR) {
        return can_run_amalgamator_event(&state->player);
    }

    return 1;
}

//이벤트 실행 함수
static int run_event_by_id(GameState *state, int event_id)
{
    if (state == NULL) {
        return 0;
    }

    if (event_id == EVENT_SYMBIOTE) {
        return run_symbiote_event(state);
    }
    else if (event_id == EVENT_MUTATING_FOREST) {
        return run_mutating_forest_event(state);
    }
    else if (event_id == EVENT_JUNGLE_MAZE) {
        return run_jungle_maze_event(state);
    }
    else if (event_id == EVENT_AMALGAMATOR) {
        return run_amalgamator_event(state);
    }
    else if (event_id == EVENT_SUNKEN_TREASURY) {
        return run_sunken_treasury_event(state);
    }

    return 1;
}

//특정 카드 몇장있는지 확인 함수
static int count_cards_by_name_prefix(const Player *player, const char *prefix)
{
    int i;
    int count;
    size_t prefix_len;

    if (player == NULL || prefix == NULL) {
        return 0;
    }

    count = 0;
    prefix_len = strlen(prefix);

    for (i = 0; i < player->owned_deck_count; i++) {
        if (strncmp(player->owned_deck[i].name, prefix, prefix_len) == 0) {
            count++;
        }
    }

    return count;
}

//융합자 이벤트 실행가능한지 체크하는 함수
static int can_run_amalgamator_event(const Player *player)
{
    if (player == NULL) {
        return 0;
    }

    if (player->owned_deck_count < 11) {
        return 0;
    }

    if (count_cards_by_name_prefix(player, "타격") < 2) {
        return 0;
    }

    if (count_cards_by_name_prefix(player, "수비") < 2) {
        return 0;
    }

    return 1;
}

//특정이름의 카드 제거 함수
static int remove_first_card_by_name_prefix(Player *player, const char *prefix)
{
    int i;
    size_t prefix_len;

    if (player == NULL || prefix == NULL) {
        return 0;
    }

    prefix_len = strlen(prefix);

    for (i = 0; i < player->owned_deck_count; i++) {
        if (strncmp(player->owned_deck[i].name, prefix, prefix_len) == 0) {
            return remove_card_from_deck(player, i);
        }
    }

    return 0;
}

//융합자 이벤트 실행 함수
static int run_amalgamator_event(GameState *state)
{
    int choice;
    Card new_card;

    if (state == NULL) {
        return 0;
    }

    choice = show_amalgamator_event_screen();

    if (choice == 1) {
        new_card = get_card_from_pool(CARD_INDEX_ULTIMATE_DEFEND);

        if (state->player.owned_deck_count >= MAX_DECK_SIZE) {
            remove_first_card_by_name_prefix(&state->player, "수비");
            remove_first_card_by_name_prefix(&state->player, "수비");
            add_card_to_deck(&state->player, new_card);
        } else {
            add_card_to_deck(&state->player, new_card);
            remove_first_card_by_name_prefix(&state->player, "수비");
            remove_first_card_by_name_prefix(&state->player, "수비");
        }

        show_amalgamator_result_screen(&new_card, "수비", 2);
        return 1;
    }

    if (choice == 2) {
        new_card = get_card_from_pool(CARD_INDEX_ULTIMATE_STRIKE);

        if (state->player.owned_deck_count >= MAX_DECK_SIZE) {
            remove_first_card_by_name_prefix(&state->player, "타격");
            remove_first_card_by_name_prefix(&state->player, "타격");
            add_card_to_deck(&state->player, new_card);
        } else {
            add_card_to_deck(&state->player, new_card);
            remove_first_card_by_name_prefix(&state->player, "타격");
            remove_first_card_by_name_prefix(&state->player, "타격");
        }

        show_amalgamator_result_screen(&new_card, "타격", 2);
        return 1;
    }

    return 1;
}

//가라앉은 보물 이벤트 함수
static int run_sunken_treasury_event(GameState *state)
{
    int choice;
    int gold_gain;
    Card greed;

    if (state == NULL) {
        return 0;
    }

    choice = show_sunken_treasury_event_screen();

    if (choice == 1) {
        gold_gain = 52 + rand() % 16;

        state->player.gold += gold_gain;

        show_sunken_treasury_result_screen(choice, gold_gain, NULL, &state->player);
        return 1;
    }

    if (choice == 2) {
        gold_gain = 303 + rand() % 61;
        greed = create_greed_card();

        state->player.gold += gold_gain;
        add_card_to_deck(&state->player, greed);

        show_sunken_treasury_result_screen(choice, gold_gain, &greed, &state->player);
        return 1;
    }

    return 1;
}
