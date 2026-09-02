CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=gnu11 -g
LDFLAGS = -pthread

# Every .c file in this dir becomes its own binary of the same name
SRCS := $(wildcard *.c)
BINS := $(SRCS:.c=)

.PHONY: all clean asan tsan

all: $(BINS)

# Pattern rule: race.c -> race, server.c -> server, etc.
%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

# ASan+UBSan build -- separate binaries, suffixed, so 'make' output isn't overwritten
asan: CFLAGS += -fsanitize=address,undefined
asan: $(addsuffix _asan,$(BINS))

%_asan: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

# TSan build -- can't combine with ASan in one binary, needs its own target
tsan: CFLAGS += -fsanitize=thread
tsan: $(addsuffix _tsan,$(BINS))

%_tsan: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

clean:
	rm -f $(BINS) $(addsuffix _asan,$(BINS)) $(addsuffix _tsan,$(BINS))