# Makefile

CC = gcc
CFLAGS = -O3 -march=native -Wall -Wextra
LDFLAGS = -lgmp -lssl -lcrypto

SRCDIRS = addition subtraction multiplication barret_reduction exponentiation montgomery mont_expo sqrt_tonelli_shanks mod_inv random check0s check1s hash

SRCS = $(wildcard $(addsuffix /*.c, $(SRCDIRS))) main.c

TARGET = main_program

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
