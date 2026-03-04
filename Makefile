# Makefile

CC = gcc
CFLAGS = -O3 -march=native -Wall -Wextra
LDFLAGS = -lgmp -lssl -lcrypto

SRCDIRS = addition subtraction multiplication barrett_reduction exponentiation montgomery mont_expo sqrt_tonelli_shanks mod_inv random check0s check1s hash dilithium

SRCS     = $(wildcard $(addsuffix /*.c, $(SRCDIRS))) main.c
LIB_SRCS = $(wildcard $(addsuffix /*.c, $(SRCDIRS)))

TARGET      = main_program
TEST_TARGET = test_runner

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

$(TEST_TARGET): $(LIB_SRCS) tests/unitTests.c
	$(CC) $(CFLAGS) $(LIB_SRCS) tests/unitTests.c -o $(TEST_TARGET) $(LDFLAGS) -lcriterion

test: $(TEST_TARGET)
	./$(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET)

.PHONY: all clean test
