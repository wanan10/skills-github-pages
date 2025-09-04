CC := gcc
CFLAGS := -std=c11 -O2 -Wall -Wextra -pedantic -D_POSIX_C_SOURCE=200809L -Iinclude
LDFLAGS := 

SRC_DIR := src
BIN_DIR := bin
TARGET := $(BIN_DIR)/satsolver

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:.c=.o)

.PHONY: all debug clean run

all: $(TARGET)

debug: CFLAGS := -std=c11 -g3 -O0 -Wall -Wextra -pedantic -Iinclude
debug: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	$(TARGET)

clean:
	rm -f $(SRC_DIR)/*.o $(TARGET)
