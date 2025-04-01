CC = gcc
CFLAGS = -Wall -Wextra
SOURCES = calc.c calculator.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = calc

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
