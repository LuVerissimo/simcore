<<<<<<< HEAD
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=gnu11 -g
LDFLAGS = -pthread -lrt

# Define the shared library logic module explicitly
MODULE = sharedqueue.c

# STRICTLY define your binaries. Do NOT use wildcards here.
BINS = shm_writer shm_reader
=======
CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=gnu11 -g
LDFLAGS = -pthread

# Every .c file in this dir becomes its own binary of the same name
SRCS := $(wildcard *.c)
BINS := $(SRCS:.c=)
>>>>>>> ec8c9f9d299c93c68f39a22106b54a0e9608f1a8

.PHONY: all clean asan tsan

all: $(BINS)

<<<<<<< HEAD
# Rule to compile the writer executable
shm_writer: shm_writer.c $(MODULE)
	$(CC) $(CFLAGS) shm_writer.c $(MODULE) -o shm_writer $(LDFLAGS)

# Rule to compile the reader executable
shm_reader: shm_reader.c $(MODULE)
	$(CC) $(CFLAGS) shm_reader.c $(MODULE) -o shm_reader $(LDFLAGS)

# Address Sanitizer target
asan: CFLAGS += -fsanitize=address,undefined
asan: clean shm_writer_asan shm_reader_asan

shm_writer_asan: shm_writer.c $(MODULE)
	$(CC) $(CFLAGS) shm_writer.c $(MODULE) -o shm_writer_asan $(LDFLAGS)

shm_reader_asan: shm_reader.c $(MODULE)
	$(CC) $(CFLAGS) shm_reader.c $(MODULE) -o shm_reader_asan $(LDFLAGS)

# Thread Sanitizer target
tsan: CFLAGS += -fsanitize=thread
tsan: clean shm_writer_tsan shm_reader_tsan

shm_writer_tsan: shm_writer.c $(MODULE)
	$(CC) $(CFLAGS) shm_writer.c $(MODULE) -o shm_writer_tsan $(LDFLAGS)

shm_reader_tsan: shm_reader.c $(MODULE)
	$(CC) $(CFLAGS) shm_reader.c $(MODULE) -o shm_reader_tsan $(LDFLAGS)

clean:
	rm -f $(BINS) shm_writer_asan shm_reader_asan shm_writer_tsan shm_reader_tsan
=======
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
>>>>>>> ec8c9f9d299c93c68f39a22106b54a0e9608f1a8
