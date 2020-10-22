#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>

#define MAX_BUFFER_SIZE 256
#define INVALID_BIT_READ -1
#define INVALID_BIT_WRITE -1

#define GRAY_SCALE 256

unsigned char signature[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

/* global codeword related */
int *frequency = NULL;
int rgb_cnt = 0;	
int active_codeword_cnt = 0;

int *stack;
int stack_top;

typedef struct {
	int index;
	unsigned int weight;
} node_t;

void determine_frequency(FILE *f) {
    int c;
    // ensure rgb cnt not overflow
	int i=0;
	while ((c=fgetc(f))!=EOF) {
		if(rgb_cnt<0){
			perror("File is like your mom! Too Fat!\n");
			exit(1);
		}
        frequency[(int)c]++;
        rgb_cnt++;
		printf("0x%02X ",c);
		if( !(++i % 16) ) putc('\n', stdout);
    }
    for (int i = 0; i < (GRAY_SCALE-1); i++){
        if (frequency[i] > 0)
            active_codeword_cnt++;
	}
}

void init() {
    frequency = (int *)
		calloc(2 * GRAY_SCALE, sizeof(int));
}

int encode(const char *in_file, const char *out_file){
	FILE *fin, *fout;
	if ((fin = fopen(in_file, "rb")) == NULL) {
        	perror("Failed to open input file");
        	return 1;
    }
	if ((fout = fopen(out_file, "wb")) == NULL) {
		perror("Failed to open output file");
		fclose(fin);
		return 1;
	}
	// read PNG signature 		
	unsigned char head[8];
	fread(head, sizeof(head), 1, fin);
	if (memcmp(signature, head, 8)){
		perror("Input file is not PNG format\n");
		return 1;	/* bad signature */
	}
	determine_frequency(fin);
	// stack = (int*)calloc(active_codeword_cnt-1, sizeof(int));

	// allocate_tree();

    // add_leaves();
    // write_header(fout);
    // build_tree();
    // fseek(fin, 0, SEEK_SET);
    // int c;
    // while ((c = fgetc(fin)) != EOF)
    // encode_alphabet(fout, c);
    // flush_buffer(fout);

	fclose(fin);
	fclose(fout);
	return 0;
}
int decode(const char *in_file, const char *out_file){
	FILE *fin, *fout;
	if ((fin = fopen(in_file, "rb")) == NULL) {
        	perror("Failed to open input file");
        	return 1;
    	}
	if ((fout = fopen(out_file, "wb")) == NULL) {
		perror("Failed to open output file");
		fclose(fin);
		return 1;
	}

	unsigned char head[8];
	fread(head, sizeof(head), 1, fin);
	if (memcmp(signature, head, 8)){
		perror("Input file is not PNG format\n");
		return 1;
	}

	return 0;
}
int finalise(){
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
		encode(argv[2], argv[3]);
	else
		print_help();
	finalise();
	return 0;
}
