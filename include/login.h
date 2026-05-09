#ifndef LOGIN_H
#define LOGIN_H

#include "type.h"

#define SAVE_DIR "saves"
#define SAVE_EXT ".sav"

int is_valid_username(const char *username);
int save_file_exists(const char *username);

#endif