#ifndef SAVE_H
#define SAVE_H

#include "type.h"

#define SAVE_DIR "saves"
#define SAVE_EXT ".sav"

int make_save_path(const char *username, char *path, int size);
int save_game(const GameState *state);
int load_game(const char *username, GameState *state);
int save_file_exists(const char *username);

#endif