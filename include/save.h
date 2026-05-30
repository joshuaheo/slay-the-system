#ifndef SAVE_H
#define SAVE_H

#include "type.h"

#define SAVE_DIR "saves"
#define SAVE_EXT ".sav"

int make_save_path(const char *username, int slot, char *path, int size);
int save_game(const GameState *state);
int load_game(const char *username, int slot, GameState *state);
int save_file_exists(const char *username, int slot);
int delete_save_file(const char *username, int slot);
int get_save_modified_time_string(const char *username, int slot, char *buffer, int size);

#endif
