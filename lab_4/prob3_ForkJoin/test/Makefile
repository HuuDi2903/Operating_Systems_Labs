CC       = gcc
CFLAGS   = -pthread -fcommon -Wall -g

TARGET   = mypool
SRCS     = main.c mytpool.c mytask.c
OBJS     = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)