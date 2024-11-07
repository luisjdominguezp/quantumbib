# Makefile

CC = gcc
CFLAGS = -O3 -march=native -Wall -Wextra
LDFLAGS = -lgmp -lssl -lcrypto

SRCDIRS = addition random

SRCS = $(wildcard $(addsuffix /*.c, $(SRCDIRS))) main.c

TARGET = main_program

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
