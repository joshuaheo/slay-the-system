#ifndef UI_H
#define UI_H

typedef enum {
    MENU_START_GAME = 1,
    MENU_EXIT = 2
} MenuChoice;

void init_ui(void);
void close_ui(void);
MenuChoice show_start_screen(void);

#endif