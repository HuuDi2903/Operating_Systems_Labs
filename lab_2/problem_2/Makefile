CC = gcc
CFLAGS = -Wall -pthread

all: sum_serial sum_multi_thread

sum_serial: sum_serial.c
	$(CC) $(CFLAGS) -o $@ $<

sum_multi_thread: sum_multi_thread.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f sum_serial sum_multi_thread
