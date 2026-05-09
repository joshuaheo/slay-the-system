#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "login.h"

// username을 이용해 세이브 파일 경로를 생성하는 함수
static void make_save_path(const char *username, char *path, int size) {
    snprintf(path, size, "%s/%s%s", SAVE_DIR, username, SAVE_EXT);
}

//아이디가 유효한 아이디인지 확인하는 함수
int is_valid_username(const char *username) {
    int i;
    int len;

    if (username == NULL) {
        return 0;
    }

    len = (int)strlen(username);

    if (len <= 0) {
        return 0;
    }

    if (len >= MAX_NAME_LEN) {
        return 0;
    }

    for (i = 0; i < len; i++) {
        unsigned char ch = (unsigned char)username[i];

        if (!(isalnum(ch) || ch == '_' || ch == '-')) {
            return 0;
        }
    }

    return 1;
}

//해당 username의 세이브 파일 존재 여부 확인
int save_file_exists(const char *username) {
    char path[256];
    FILE *fp;

    if (!is_valid_username(username)) {
        return 0;
    }

    make_save_path(username, path, sizeof(path));

    fp = fopen(path, "rb");
    if (fp == NULL) {
        return 0;
    }

    fclose(fp);
    return 1;
}
