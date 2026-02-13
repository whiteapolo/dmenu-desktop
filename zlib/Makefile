CC = cc
CFLAGS = -I./include -O3
DEV_CFLAGS = -I./include -O0 -Wall -Wextra -g -pedantic

all: release

release:
	@mkdir -p obj
	$(CC) $(CFLAGS) -c ./src/all.c -o obj/libzatar.o
	ar rcs libzatar.a ./obj/libzatar.o

dev:
	@mkdir -p obj
	$(CC) $(DEV_CFLAGS) -c ./src/all.c -o obj/libzatar.o
	ar rcs libzatar.a ./obj/libzatar.o

clean:
	rm -rf obj libzatar.a

.PHONY: all clean release dev
