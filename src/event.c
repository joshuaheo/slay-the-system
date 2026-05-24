#include <string.h>

#include "event.h"
#include "player.h"
#include "save.h"
#include "ui.h"

static int has_attack_card_to_corrupt(const Player *player);
static void append_corrupted_tag(Card *card);
static void corrupt_attack_card(Card *card);
static int run_symbiote_event(GameState *state);

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

static void corrupt_attack_card(Card *card)
{
    if (card == NULL) {
        return;
    }

    if (card->type != CARD_ATTACK || card->damage <= 0) {
        return;
    }

    card->damage = card->damage * 3 / 2;
    card->hp_loss += 2;

    append_corrupted_tag(card);
}

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

int run_event_stage(GameState *state)
{
    if (state == NULL) {
        return 0;
    }

    if (!run_symbiote_event(state)) {
        return 0;
    }

    state->floor++;

    if (!save_game(state)) {
        return 0;
    }

    return 1;
}