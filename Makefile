# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c11

# Source files
SRCS = main.c fc.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
EXEC = sfl

# Build rule
build: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Run rule
run_sfl: build
	./$(EXEC)

# Clean rule
clean:
	rm -f $(EXEC) $(OBJS)

