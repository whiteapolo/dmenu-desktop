CC = cc
BASE_CFLAGS = \
	-I./include \
	-std=c2x \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Werror \
	-Wshadow \
	-Wconversion \
	-Wsign-conversion \
	-Wformat=2 \
	-Wformat-overflow=2 \
	-Wformat-truncation=2 \
	-Wnull-dereference \
	-Wundef \
	-Wcast-align \
	-Wcast-qual \
	-Wpointer-arith \
	-Wwrite-strings \
	-Wvla \
	-Wswitch \
	-Wswitch-enum \
	-Wstrict-prototypes \
	-Wold-style-definition \
	-Wmissing-prototypes \
	-Wmissing-declarations \
	-Wstrict-aliasing=3 \
	-Wlogical-op \
	-Wduplicated-cond \
	-Wduplicated-branches \
	-Wbad-function-cast \
	-Wnested-externs

PROD_CFLAGS = $(BASE_CFLAGS) $(WARN_CFLAGS) -O3
DEV_CFLAGS = $(BASE_CFLAGS) $(WARN_CFLAGS) -O0 -g

all: release

release:
	@mkdir -p obj
	$(CC) $(PROD_CFLAGS) -c ./src/all.c -o obj/libzatar.o
	ar rcs libzatar.a ./obj/libzatar.o

dev:
	@mkdir -p obj
	$(CC) $(DEV_CFLAGS) -c ./src/all.c -o obj/libzatar.o
	ar rcs libzatar.a ./obj/libzatar.o

clean:
	rm -rf obj libzatar.a

.PHONY: all clean release dev
