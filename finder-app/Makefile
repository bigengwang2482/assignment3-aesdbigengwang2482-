# Compiler and flags
CC=gcc
CFLAGS=-Wall

# Target executable
TARGET=writer

# Source files
SOURCES=writer.c

# Object files
OBJECTS=$(SOURCES:.c=.o)

# Default target
all: $(TARGET)

# Basic compile and link

$(TARGET): $(OBJECTS)
	$(CC) $(CLAGS) -o $@ $^


# Compile
%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(TARGET)
