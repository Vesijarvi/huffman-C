CC=gcc
CFLAGS=-fsanitize=address -g

huffman: huffman.c
	${CC} ${CFLAGS} -o huffman huffman.c

clean:
	rm huffman
