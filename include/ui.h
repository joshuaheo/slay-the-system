#ifndef UI_H
#define UI_H

#include "type.h"

typedef enum {
    MENU_START_GAME = 1,
    MENU_EXIT = 2
} MenuChoice;

void init_ui(void);
void close_ui(void);
MenuChoice show_start_screen(void);
int show_login_screen(char *username, int size);
void show_invalid_username_screen(void);

void show_temp_battle_screen( GameState *state);
#endif