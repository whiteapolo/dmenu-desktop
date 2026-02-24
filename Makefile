CFLAGS = -O3
DEV_CFLAGS = -I./include -O0 -Wall -Wextra -g -pedantic

all: release

release:
	dmd main.d -O -release -inline -boundscheck=off -of=dmenu-desktop

dev:
	dmd main.d -of=dmenu-desktop

clean:
	rm -rf dmenu-desktop

.PHONY: all clean release dev
