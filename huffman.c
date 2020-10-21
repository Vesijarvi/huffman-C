#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>

#define MAX_BUFFER_SIZE 256
#define INVALID_BIT_READ -1
#define INVALID_BIT_WRITE -1

unsigned char png_head[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

int encode(const char *ifile, const char *ofile){
	FILE *fin, *fout;
	if ((fin = fopen(ifile, "rb")) == NULL) {
        	perror("Failed to open input file");
        	return 1;
    	}
    	if ((fout = fopen(ofile, "wb")) == NULL) {
        	perror("Failed to open output file");
        	fclose(fin);
        	return 1;
    	}
		
	unsigned char head[8];
	fread(head, sizeof(head), 1, fin);
	if (memcmp(png_head, head, 8)){
		perror("Input file is not PNG format\n");
		return 1;
	}
	


	return 0;
}
int decode(const char *ifile, const char *ofile){
	FILE *fin, *fout;
	if ((fin = fopen(ifile, "rb")) == NULL) {
        	perror("Failed to open input file");
        	return 1;
    	}
	if ((fout = fopen(ofile, "wb")) == NULL) {
		perror("Failed to open output file");
		fclose(fin);
		return 1;
	}

	unsigned char head[8];
	fread(head, sizeof(head), 1, fin);
	if (memcmp(png_head, head, 8)){
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

	/*
	unsigned char head[8];
	fread(head, sizeof(head), 1, f);
	*/

	
	if(strcmp(argv[1], "encode") == 0)
		encode(argv[2], argv[3]);
	else if(strcmp(argv[1], "decode") == 0)
		encode(argv[2], argv[3]);
	else
		print_help();

	finalise();
	return 0;
}
