#include<stdlib.h>
int main(int argc, char *argv[]){
	// argc: counter of arguments
	// argv[0]: myprogram
	// argc[1]: myfile.png
	
	FILE *f = fopen(argv[1],"r");
	

	fclose(f);
	return 0;
}
