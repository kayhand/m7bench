#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int load_data(char *fname, int maxNumber, long int n){
	int curVal = 0;
	FILE *file;
	long int size = 1;

	if((file = fopen(fname, "w")) == NULL){
		perror("Could not open");
		exit(EXIT_FAILURE);
	}


	srand(time(NULL));
	while(size > 0 && size <= n){
		curVal = rand() % maxNumber;
		fprintf(file, "%d\n", curVal);
		size++;
		if(! (size % 10000))
		fflush(file);
	}

	if(fclose(file)){
		perror("fclose");
		exit(EXIT_FAILURE);
	}
	return(EXIT_SUCCESS);
}

int main(int argc, char * argv[]){
	long int n = 0;
	if(argc < 4){
		printf("Usage: %s <file> <num_of_bits> <num_of_elements> \n", argv[0]);
		exit(1);
	}

	sscanf(argv[3], "%ld", &n);
	fprintf(stderr, "%ld\n", n);

	return load_data(argv[1], pow(2, atoi(argv[2])), n);
}
