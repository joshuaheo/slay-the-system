CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
TARGET = build/game
SRCS = $(wildcard src/*.c)

all:
  mkdir -p build
  $(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
  rm -rf build
