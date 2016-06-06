#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

int load_data(char* fname, unsigned int maxNumber, int size){
 	FILE *file;

	int curInd = 0;
	unsigned int curVal = 0;
	if((file = fopen(fname, "w")) == NULL){
		return(-1);
		perror("Could not open");
	}

	if (fseek(file, 0, SEEK_END) != 0)
	{
		perror("fseek");
		exit(1); 
	}

	srand(time(NULL));
	while(curInd < size){
		curVal = rand() % maxNumber;
		fprintf(file, "%d\n", curVal);
		curInd++;
	}

	if(file){
		fclose(file);
	}
	return(0);
}

int main(int argc, char * argv[]){
	if(argc < 3){
                printf("Usage: %s <file> <num_of_bits> <num_of_elements> \n", argv[0]);
                exit(1);
        }

	int errno;
	unsigned maxNumber = pow(2, atoi(argv[2]));
	errno = load_data(argv[1], maxNumber, atoi(argv[3]));
}
