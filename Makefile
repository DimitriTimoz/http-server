
# Compiler
CC = gcc
CFLAGS = -Wall -Iinclude -g

# Source and target directories
SRC_DIR = src
OBJ_DIR = obj
INCLUDE_DIR = include
TARGET = main

# Source and object files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

# Rule to build the program
all: $(TARGET)

# Link object files to create the final executable
$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $(OBJ_FILES)

# Compile each source file to an object file
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create the object directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean up build files
clean:
	rm -f $(TARGET) $(OBJ_DIR)/*.o
