#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "save.h"
#include "login.h"

//saves 폴더가 있는지 확인하는 함수 access(), mkdir() 사용
static int ensure_save_dir(void)
{
    if (access(SAVE_DIR, F_OK) == 0) {
        return 1;
    }

    if (mkdir(SAVE_DIR, 0755) == 0) {
        return 1;
    }

    if (errno == EEXIST) {
        return 1;
    }

    return 0;
}

//세이브 파일에 데이터를 적는 함수 write() 사용
static int write_all(int fd, const void *buffer, size_t size)
{
    const char *ptr = (const char *)buffer;
    size_t total = 0;

    while (total < size) {
        ssize_t written = write(fd, ptr + total, size - total);

        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            return 0;
        }

        if (written == 0) {
            return 0;
        }

        total += written;
    }

    return 1;
}

//세이브 파일에서 데이터를 읽는 함수 read() 사용
static int read_all(int fd, void *buffer, size_t size)
{
    char *ptr = (char *)buffer;
    size_t total = 0;

    while (total < size) {
        ssize_t read_size = read(fd, ptr + total, size - total);

        if (read_size < 0) {
            if (errno == EINTR) {
                continue;
            }
            return 0;
        }

        if (read_size == 0) {
            return 0;
        }

        total += read_size;
    }

    return 1;
}

//세이브 파일 경로를 만드는 함수 
int make_save_path(const char *username, int slot, char *path, int size)
{
    if (username == NULL || path == NULL || size <= 0) {
        return 0;
    }

    if (!is_valid_username(username)) {
        return 0;
    }

    if (slot < 1 || slot > MAX_SAVE_SLOTS) {
        return 0;
    }

    snprintf(path, size, "%s/%s_%d%s", SAVE_DIR, username, slot, SAVE_EXT);
    return 1;
}

//해당 유저의 세이브 파일이 존재하는지 확인하는 함수 access() 사용
int save_file_exists(const char *username, int slot)
{
    char path[256];

    if (!make_save_path(username, slot, path, sizeof(path))) {
        return 0;
    }

    return access(path, F_OK) == 0;
}

//게임 저장 함수 open(), write(), fsync(), close() 사용
int save_game(const GameState *state)
{
    char path[256];
    int fd;

    if (state == NULL) {
        return 0;
    }

    if (!is_valid_username(state->username)) {
        return 0;
    }

    if (!ensure_save_dir()) {
        return 0;
    }

    if (!make_save_path(state->username, state->save_slot, path, sizeof(path))) {
    return 0;
    }

    fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        return 0;
    }

    if (!write_all(fd, state, sizeof(GameState))) {
        close(fd);
        return 0;
    }

    if (fsync(fd) != 0) {
        close(fd);
        return 0;
    }

    if (close(fd) != 0) {
        return 0;
    }

    return 1;
}

//게임 로드 함수 open(), read(), close() 사용
int load_game(const char *username, int slot, GameState *state)
{
    char path[256];
    int fd;

    if (username == NULL || state == NULL) {
        return 0;
    }

    if (!make_save_path(username, slot, path, sizeof(path))) {
        return 0;
    }

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        return 0;
    }

    if (!read_all(fd, state, sizeof(GameState))) {
        close(fd);
        return 0;
    }

    if (close(fd) != 0) {
        return 0;
    }

    state->save_slot = slot;

    return 1;
}

//세이브 파일 삭제 함수 unlink() 사용
int delete_save_file(const char *username, int slot)
{
    char path[256];

    if (!make_save_path(username, slot, path, sizeof(path))) {
        return 0;
    }

    if (unlink(path) != 0) {
        if (errno == ENOENT) {
            return 1;
        }
        return 0;
    }

    return 1;
}

//마지막 저장 함수 시간 추가 함수 stat() 사용
int get_save_modified_time_string(const char *username, int slot, char *buffer, int size)
{
    char path[256];
    struct stat st;
    struct tm *time_info;

    if (buffer == NULL || size <= 0) {
        return 0;
    }

    if (!make_save_path(username, slot, path, sizeof(path))) {
        return 0;
    }

    if (stat(path, &st) != 0) {
        return 0;
    }

    time_info = localtime(&st.st_mtime);
    if (time_info == NULL) {
        return 0;
    }

    if (strftime(buffer, size, "%Y-%m-%d %H:%M:%S", time_info) == 0) {
        return 0;
    }

    return 1;
}
