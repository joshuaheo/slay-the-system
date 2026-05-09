CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
TARGET = build/game
SRCS = $(wildcard src/*.c)
LDFLAGS = -lncursesw

all:
	mkdir -p build
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -rf build
