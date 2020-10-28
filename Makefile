CC=gcc
CFLAGS=-fsanitize=address -g

huffman:huffman.c
	${cc} -o huffman huffman.c

saint: huffman.c
	${CC} ${CFLAGS} -o huff-sanit huffman.c
clean:
	rm huffman
