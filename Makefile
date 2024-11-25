# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Wno-unused-variable -pthread

# Target executable
TARGET = HW4

# Source file
SRC = HW4.c

# Default target: compile the program
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Run the program
run: $(TARGET)
	./$(TARGET)

# Clean up generated files
clean:
	rm -f $(TARGET) output.txt
