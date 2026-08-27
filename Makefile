CC = gcc
CFLAGS = -Wall -Wextra -std=c11
CPPFLAGS = -Isrc

SOURCES = $(wildcard src/*.c)
TARGET = micro

.PHONY: all clean force

all: $(TARGET)

$(TARGET): force
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SOURCES) -o $@

force:

clean:
	rm -f $(TARGET)