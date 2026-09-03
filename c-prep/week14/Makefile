CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=gnu11 -g
LDFLAGS = -pthread -lrt

# Define the shared library logic module explicitly
MODULE = sharedqueue.c

# STRICTLY define your binaries. Do NOT use wildcards here.
BINS = shm_writer shm_reader

.PHONY: all clean asan tsan

all: $(BINS)

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
