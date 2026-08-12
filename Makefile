all:
	make dev -C ./zlib
	cc main.c ./zlib/libzatar.a -o exe -I./zlib/include -g -O3
