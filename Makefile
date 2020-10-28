CC=gcc
CFLAGS=-fsanitize=address -g

huffman: huffman.c
	${CC} ${CFLAGS} -o huff-sanit huffman.c

huffman1: huffman1.c
	${CC} ${CFLAGS} -o huff-sanit huffman1.c

clean:
	rm huffman
