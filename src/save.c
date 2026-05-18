#include <stdio.h>
#include <string.h>
#include "save.h"
#include "login.h"

// username을 이용해 세이브 파일 경로를 생성하는 함수
int make_save_path(const char *username, char *path, int size) {
    if (username == NULL || path == NULL || size <= 0) {
        return 0;
    }

    snprintf(path, size, "%s/%s%s", SAVE_DIR, username, SAVE_EXT);
    return 1;
}

//해당 username의 세이브 파일 존재 여부 확인
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

// GameState 구조체를 username에 해당하는 세이브 파일에 저장하는 함수.
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

// username에 해당하는 세이브 파일에서 GameState를 읽어와 복원한다.
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

//세이브 파일을 지우는 함수
int delete_save_file(const char *username) {
    char path[256];

    if (!is_valid_username(username)) {
        return 0;
    }

    if (!make_save_path(username, path, sizeof(path))) {
        return 0;
    }

    if (remove(path) != 0) {
        return 0;
    }

    return 1;
}