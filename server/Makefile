# Compiler and flags

# Check cross compile

CROSS_COMPILE?=

ifeq ($(CROSS_COMPILE),)
	CC=gcc
else
	CC=$(CROSS_COMPILE)gcc
endif

CFLAGS=-Wall -Werror

# Target executable
TARGET=aesdsocket

# Source files
SOURCES=aesdsocket.c

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
