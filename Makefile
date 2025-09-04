CC := gcc
CFLAGS := -O2 -std=c11 -Wall -Wextra -pedantic -march=native -D_POSIX_C_SOURCE=200809L
LDFLAGS :=

SRC_DIR := src
BUILD_DIR := build
BIN := bin/sat_sudoku

SRCS := \
	$(SRC_DIR)/util.c \
	$(SRC_DIR)/cnf.c \
	$(SRC_DIR)/dpll.c \
	$(SRC_DIR)/sudoku.c \
	$(SRC_DIR)/windoku.c \
	$(SRC_DIR)/main.c

OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean run

all: $(BIN)

$(BIN): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) bin

run: $(BIN)
	./$(BIN) --help

