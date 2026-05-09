#include <stdio.h>
#include <string.h>
#include "save.h"
#include "login.h"

int make_save_path(const char *username, char *path, int size) {
    if (username == NULL || path == NULL || size <= 0) {
        return 0;
    }

    snprintf(path, size, "%s/%s%s", SAVE_DIR, username, SAVE_EXT);
    return 1;
}

int save_file_exists(const char *username) {
    char path[256];
    FILE *fp;

    if (!is_valid_username(username)) {
        return 0;
    }

    if (!make_save_path(username, path, sizeof(path))) {
        return 0;
    }

    fp = fopen(path, "rb");
    if (fp == NULL) {
        return 0;
    }

    fclose(fp);
    return 1;
}

int save_game(const GameState *state) {
    char path[256];
    FILE *fp;

    if (state == NULL) {
        return 0;
    }

    if (!is_valid_username(state->username)) {
        return 0;
    }

    if (!make_save_path(state->username, path, sizeof(path))) {
        return 0;
    }

    fp = fopen(path, "wb");
    if (fp == NULL) {
        return 0;
    }

    if (fwrite(state, sizeof(GameState), 1, fp) != 1) {
        fclose(fp);
        return 0;
    }

    fclose(fp);
    return 1;
}

int load_game(const char *username, GameState *state) {
    char path[256];
    FILE *fp;

    if (username == NULL || state == NULL) {
        return 0;
    }

    if (!is_valid_username(username)) {
        return 0;
    }

    if (!make_save_path(username, path, sizeof(path))) {
        return 0;
    }

    fp = fopen(path, "rb");
    if (fp == NULL) {
        return 0;
    }

    if (fread(state, sizeof(GameState), 1, fp) != 1) {
        fclose(fp);
        return 0;
    }

    fclose(fp);
    return 1;
}