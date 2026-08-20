# ===== Compiler =====
CC = gcc

# ===== Flags =====
CFLAGS = -Wall -Wextra -Wpedantic -Wshadow -Wpointer-arith -Wcast-align \
         -Wstrict-prototypes -Wmissing-prototypes -Wwrite-strings -Wconversion \
         -Wformat=2 -Wfloat-equal -Wundef -Wvla -std=c99 -g -Iinclude \
         $(shell pkg-config --cflags gtk+-3.0)

# ===== Linker Flags =====
LDFLAGS = -lm \
          $(shell sdl2-config --cflags --libs) \
          -lSDL2_image \
          $(shell pkg-config --libs gtk+-3.0)

# ===== Directories =====
SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = $(BIN_DIR)/obj

# ===== Files =====
SRC = $(shell find $(SRC_DIR) -name '*.c')
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TARGET = $(BIN_DIR)/app

# ===== Rules =====
all: $(TARGET)

$(TARGET): $(OBJ) | $(BIN_DIR)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR)

re: clean all

.PHONY: all clean re
