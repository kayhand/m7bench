#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int load_data(char *fname, int maxNumber, int size){
	int curInd = 0, curVal = 0;
 	FILE *file;

	if((file = fopen(fname, "w")) == NULL){
		perror("Could not open");
		exit(EXIT_FAILURE);
	}


	srand(time(NULL));
	while(curInd < size){
		curVal = rand() % maxNumber;
		fprintf(file, "%d\n", curVal);
		curInd++;
	}

	if(fclose(file)){
		perror("fclose");
		exit(EXIT_FAILURE);
	}
	return(EXIT_SUCCESS);
}

int main(int argc, char * argv[]){
	if(argc < 3){
                printf("Usage: %s <file> <num_of_bits> <num_of_elements> \n", argv[0]);
                exit(1);
        }

	return load_data(argv[1], pow(2, atoi(argv[2])), atoi(argv[3]));
}
