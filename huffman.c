#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>

#define MAX_BUFFER_SIZE 256
#define INVALID_BIT_READ -1
#define INVALID_BIT_WRITE -1

#define GRAY_SCALE 256

#define FILE_OPEN_FAIL -1
#define END_OF_FILE -1
#define MEM_ALLOC_FAIL -1


/* singature for PNG */
// unsigned char signature[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

int *frequency = NULL;
int rgb_cnt = 0;	
int num_active = 0;
/* the number of bytes in the original input data stream */ 
unsigned int original_size = 0;	

typedef struct {
	int index;	/* positive index means internal node; 
				   negative index means a leaf (alphabet) node. */
	unsigned int weight;
} node_t;

/* data structure to */
node_t *nodes = NULL;
int num_nodes = 0;
int *leaf_index = NULL;
int *parent_index = NULL;

int free_index = 1;

int *stack;
int stack_top;

unsigned char buffer[MAX_BUFFER_SIZE];
int bits_in_buffer = 0;
int current_bit = 0;

int eof_input = 0;

int read_header(FILE *f);
int write_header(FILE *f);
int read_bit(FILE *f);
int write_bit(FILE *f, int bit);
int flush_buffer(FILE *f);

void init(){
	frequency = (int *)calloc(2*GRAY_SCALE, sizeof(int));
	leaf_index = frequency + GRAY_SCALE - 1;
}

void determine_frequency(FILE *f) {
    int c;
    // ensure rgb cnt not overflow
	int i=0;
	while ((c=fgetc(f))!=EOF) {
		if(rgb_cnt<0){
			perror("File is like your mom! Too Fat!\n");
			exit(1);
		}
        frequency[c]++;
        rgb_cnt++;
		printf("0x%02X ",c);
		if( !(++i % 16) ) putc('\n', stdout);
    }
    for (int i = 0; i < (GRAY_SCALE-1); i++){
        if (frequency[i] > 0)
            num_active++;
	}
	// printf("\nFrequency: %d\n",num_active);
}

/* Allocate space for the coding tree nodes 
	and parent indexlook-up table */
void allocate_tree(){
	nodes = (node_t *)
		calloc(2*num_active,sizeof(node_t));
	parent_index = (int *)
		calloc(num_active, sizeof(int));
}

void finalise() {
    printf("\n----- Debugg message -----\n");
    free(parent_index); //munmap_chunk()
    free(frequency);
    free(nodes); //munmap_chunk()
}

int add_node(int index, int weight) {
    int i = num_nodes++;
	/* move existing nodes with larger weights to the right */
    while (i > 0 && nodes[i].weight > weight) {
        memcpy(&nodes[i + 1], &nodes[i], sizeof(node_t));
        if (nodes[i].index < 0)
            ++leaf_index[-nodes[i].index];
        else
            ++parent_index[nodes[i].index];
        --i;
    }
	/* add new node to its rightful place */
    ++i;
    nodes[i].index = index;
    nodes[i].weight = weight;
    if (index < 0)
        leaf_index[-index] = i;
    else
        parent_index[index] = i;
    return i;
}

void add_leaves() {
    int i, freq;
    for (i = 0; i < GRAY_SCALE; ++i) {
        freq = frequency[i];
        if (freq > 0)
            add_node(-(i + 1), freq);
    }
}

void build_tree() {
    int a, b, index;
    while (free_index < num_nodes) {
        a = free_index++;
        b = free_index++;
        // index = add_node(b/2,
        //     nodes[a].weight + nodes[b].weight);
        parent_index[b/2] = index;
    }
}

void encode_alphabet(FILE *fout, int character) {
    int node_index;
    stack_top = 0;
    node_index = leaf_index[character + 1];
    while (node_index < num_nodes) {
        stack[stack_top++] = node_index % 2;
        node_index = parent_index[(node_index + 1) / 2];
    }
    while (--stack_top > -1)
        write_bit(fout, stack[stack_top]);
}

int encode(const char *in_file, const char *out_file){
	FILE *fin, *fout;
	if ((fin = fopen(in_file, "rb")) == NULL) {
        perror("Failed to open input file");
        return FILE_OPEN_FAIL;
    }
	if ((fout = fopen(out_file, "wb")) == NULL) {
		perror("Failed to open output file");
		fclose(fin);
		return FILE_OPEN_FAIL;
	}
	/* 
	If we want better compression, 
	it is better we not check file type 
	(i.e. the signature fo the PNG file)
	read PNG signature 	
	*/	
	// unsigned char head[8];
	// fread(head, sizeof(head), 1, fin);
	// if (memcmp(signature, head, 8)){
	// 	perror("Input file is not PNG format\n");
    // 	return 1;	/* bad signature */
	// }
	determine_frequency(fin);
	stack = (int*) calloc(num_active-1, sizeof(int));

	/* Allocate space for coding tree and bit stack */
	allocate_tree();

    add_leaves();
    write_header(fout);
    build_tree();
    fseek(fin, 0, SEEK_SET);
    int c;
    while ((c = fgetc(fin)) != EOF)
        encode_alphabet(fout, c);
    flush_buffer(fout);

	fclose(fin);
	fclose(fout);
	return 0;
}

void decode_bit_stream(FILE *fin, FILE *fout) {
    int i = 0, bit, node_index = nodes[num_nodes].index;
    while (1) {
        bit = read_bit(fin);
        if (bit == -1)
            break;
        node_index = nodes[node_index * 2 - bit].index;
        if (node_index < 0) {
            char c = -node_index - 1;
            fwrite(&c, 1, 1, fout);
            if (++i == original_size)
                break;
            node_index = nodes[num_nodes].index;
        }
    }
}

int decode(const char *in_file, const char *out_file){
	FILE *fin, *fout;
	if ((fin = fopen(in_file, "rb")) == NULL) {
        perror("Failed to open input file");
        return FILE_OPEN_FAIL;
    }
	if ((fout = fopen(out_file, "wb")) == NULL) {
		perror("Failed to open output file");
		fclose(fin);
		return FILE_OPEN_FAIL;
	}
	// If we want better compression, 
	// it is better we not check file type 
	// (i.e. the signature fo the PNG file)

	// unsigned char head[8];
	// fread(head, sizeof(head), 1, fin);
	// if (memcmp(signature, head, 8)){
	// 	perror("Input file is not PNG format\n");
	// 	return 1;
	// }

	if (read_header(fin) == 0) {
    	build_tree();
    	decode_bit_stream(fin, fout);
    }
    fclose(fin);
    fclose(fout);

    return 0;
}

int read_header(FILE *f) {
     int i, j, byte = 0, size;
     size_t bytes_read;
     unsigned char buff[4];

     bytes_read = fread(&buff, 1, sizeof(int), f);
     if (bytes_read < 1)
         return END_OF_FILE;
     byte = 0;
     original_size = buff[byte++];
     while (byte < sizeof(int))
         original_size =
             (original_size << (1 << 3)) | buff[byte++];

     bytes_read = fread(&num_active, 1, 1, f);
     if (bytes_read < 1)
         return END_OF_FILE;

     allocate_tree();

     size = num_active * (1 + sizeof(int));
     unsigned int weight;
     char *buffer = (char *) calloc(size, 1);
     if (buffer == NULL)
         return MEM_ALLOC_FAIL;
     fread(buffer, 1, size, f);
     byte = 0;
     for (i = 1; i <= num_active; ++i) {
         nodes[i].index = -(buffer[byte++] + 1);
         j = 0;
         weight = (unsigned char) buffer[byte++];
         while (++j < sizeof(int)) {
             weight = (weight << (1 << 3)) |
                 (unsigned char) buffer[byte++];
         }
         nodes[i].weight = weight;
     }
     num_nodes = (int) num_active;
     free(buffer);
     return 0;
}

int write_header(FILE *f) {
     int i, j, byte = 0,
         size = sizeof(unsigned int) + 1 +
              num_active * (1 + sizeof(int));
     unsigned int weight;
     char *buffer = (char *) calloc(size, 1);
     if (buffer == NULL)
         return MEM_ALLOC_FAIL;

     j = sizeof(int);
     while (j--)
         buffer[byte++] =
             (original_size >> (j << 3)) & 0xff;
     buffer[byte++] = (char) num_active;
     for (i = 1; i <= num_active; ++i) {
         weight = nodes[i].weight;
         buffer[byte++] =
             (char) (-nodes[i].index - 1);
         j = sizeof(int);
         while (j--)
             buffer[byte++] =
                 (weight >> (j << 3)) & 0xff;
     }
     fwrite(buffer, 1, size, f);
     free(buffer);
     return 0;
}

int read_bit(FILE *f) {
    if (current_bit == bits_in_buffer) {
        if (eof_input)
            return END_OF_FILE;
        else {
            size_t bytes_read =
                fread(buffer, 1, MAX_BUFFER_SIZE, f);
            if (bytes_read < MAX_BUFFER_SIZE) {
                if (feof(f))
                    eof_input = 1;
            }
            bits_in_buffer = bytes_read << 3;
            current_bit = 0;
        }
    }

    if (bits_in_buffer == 0)
        return END_OF_FILE;
    int bit = (buffer[current_bit >> 3] >>
        (7 - current_bit % 8)) & 0x1;
    ++current_bit;
    return bit;
}

int write_bit(FILE *f, int bit) {
    if (bits_in_buffer == MAX_BUFFER_SIZE << 3) {
        size_t bytes_written =
            fwrite(buffer, 1, MAX_BUFFER_SIZE, f);
        if (bytes_written < MAX_BUFFER_SIZE && ferror(f))
            return INVALID_BIT_WRITE;
        bits_in_buffer = 0;
        memset(buffer, 0, MAX_BUFFER_SIZE);
    }
    if (bit)
        buffer[bits_in_buffer >> 3] |=
            (0x1 << (7 - bits_in_buffer % 8));
    ++bits_in_buffer;
    return 0;
}

int flush_buffer(FILE *f) {
    if (bits_in_buffer) {
        size_t bytes_written =
            fwrite(buffer, 1,
                (bits_in_buffer + 7) >> 3, f);
        if (bytes_written < MAX_BUFFER_SIZE && ferror(f))
            return -1;
        bits_in_buffer = 0;
    }
    return 0;
}

void print_help(){
	fprintf(stderr, "USAGE: ./huffman [encode|decode] <input file> <output file>");
}

int main(int argc, char **argv){
	if(argc != 4){
		print_help();
		return 1;
	}
	init();
	if(strcmp(argv[1], "encode") == 0)
		encode(argv[2], argv[3]);
	else if(strcmp(argv[1], "decode") == 0)
		decode(argv[2], argv[3]);
	else
		print_help();
	finalise();
	return 0;
}
