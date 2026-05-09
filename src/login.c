#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "login.h"

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
