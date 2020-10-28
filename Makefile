CC=gcc
CFLAGS=-fsanitize=address -g

saint: huffman.c
	${CC} ${CFLAGS} -o huff-sanit huffman.c
clean:
	rm huffman
