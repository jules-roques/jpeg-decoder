# --- Variables ---

CC = gcc
LD = gcc

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
INCLUDE_DIR = include

# Target executable
TARGET = $(BIN_DIR)/jpeg2ppm

# Flags
CFLAGS = -Wall -Wextra -std=c99 -I$(INCLUDE_DIR) -O0 -g -pg 
LDFLAGS = -lm -pg

# --- File Lists ---

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))
INCLUDE_FILES = $(wildcard $(INCLUDE_DIR)/*.h)


# --- Main Rules ---

all: $(TARGET)

# Rule to link the final executable
$(TARGET): $(OBJ_FILES) | $(BIN_DIR)
	$(LD) $(OBJ_FILES) $(LDFLAGS) -o $@

# Pattern rule to compile .c files into .o files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INCLUDE_FILES) | $(OBJ_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

# --- Directory Creation Rules ---

$(BIN_DIR):
	@mkdir -p $@

$(OBJ_DIR):
	@mkdir -p $@

# --- Other Targets ---

# Declare targets that are not files
.PHONY: all clean tests

# Rule to run tests (builds the main project first)
tests: all
	make -C tests/

# Rule to clean up the project
clean:
	@echo "Cleaning project..."
	rm -rf $(TARGET) $(OBJ_DIR)
	rm -rf *.out
	rm -rf *.ppm
	rm -rf *.pgm
	rm -rf images/*.ppm
	rm -rf images/*.pgm
	make -C tests/ clean
	@echo "Done."