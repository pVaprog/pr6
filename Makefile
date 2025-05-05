# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -pthread

# Target executables
TARGET1 = task1
TARGET2 = task2
ALL_TARGETS = $(TARGET1) $(TARGET2)

# Source files
SRC1 = task1.c
SRC2 = task2.c

# Default target
all: $(ALL_TARGETS)

# Rule for Task 1
$(TARGET1): $(SRC1)
	$(CC) $(CFLAGS) -o $@ $<

# Rule for Task 2
$(TARGET2): $(SRC2)
	$(CC) $(CFLAGS) -o $@ $<

# Clean rule
clean:
	rm -f $(ALL_TARGETS)

# Phony targets
.PHONY: all clean
