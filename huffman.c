#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>

int main(int argc, char *argv[]){
	// argc: counter of arguments
	// argv[0]: myprogram
	// argc[1]: myfile.png
	
	FILE *f = fopen(argv[1],"r");
	unsigned char png_head[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
	unsigned char head[8];
	fread(head, sizeof(head), 1, f);

	// if the file is PNG
	if (!memcmp(png_head, head, 8))
	{
		printf("Read file success\n");
	}else{
		fprintf(stderr,"%s is not a PNG file, error = %d\n", argv[1], errno);
	}

	fclose(f);
	return 0;
}
