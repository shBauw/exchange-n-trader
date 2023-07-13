CC=gcc
CFLAGS=-Wall -Werror -Wvla -O0 -std=c11 -g -fsanitize=address
LDFLAGS=-lm
BINARIES=pe_exchange pe_trader tests/trader_test1 tests/trader_test2

all: $(BINARIES)

.PHONY: clean
clean:
	rm -f $(BINARIES)

