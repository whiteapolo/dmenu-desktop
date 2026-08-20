CC = gcc
RELEASE_CFLAGS = -Wall -Wextra -O3 -I./zlib/include
DEV_CFLAGS = -Wall -Wextra -O0 -g -I./zlib/include
TARGET = dmenu-desktop
PREFIX = ~/.local/bin

all: release

release:
	make -C zlib
	$(CC) $(RELEASE_CFLAGS) -o $(TARGET) main.c ./zlib/libzatar.a

dev:
	make dev -C zlib
	$(CC) $(DEV_CFLAGS) -o $(TARGET) main.c ./zlib/libzatar.a

clean:
	rm -f $(TARGET)

install:
	mkdir -p $(PREFIX)
	cp $(TARGET) $(PREFIX)/$(TARGET)

uninstall:
	rm -f $(PREFIX)/$(TARGET)

.PHONY: all clean install uninstall
