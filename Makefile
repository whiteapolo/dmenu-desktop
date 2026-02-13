all:
	make -C ./zlib
	cc main.c ./zlib/libzatar.a -o exe -O3 -I./zlib/include
