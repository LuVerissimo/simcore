CC      = gcc
CFLAGS  = -Wall -Wextra -Wpedantic -std=gnu11 -g
LDFLAGS = -pthread -lrt

MODULE = sharedqueue.c
BINS   = connection

.PHONY: all clean asan tsan

all: $(BINS)

connection: connection.c $(MODULE)
	$(CC) $(CFLAGS) connection.c $(MODULE) -o connection $(LDFLAGS)

asan: CFLAGS += -fsanitize=address,undefined
asan: connection_asan

connection_asan: connection.c $(MODULE)
	$(CC) $(CFLAGS) connection.c $(MODULE) -o connection_asan $(LDFLAGS)

tsan: CFLAGS += -fsanitize=thread
tsan: connection_tsan

connection_tsan: connection.c $(MODULE)
	$(CC) $(CFLAGS) connection.c $(MODULE) -o connection_tsan $(LDFLAGS)

clean:
	rm -f connection connection_asan connection_tsan