#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char * argv[]){
	int approach_id = -1;
        if(argc < 4){
                printf("Usage: %s <approach (simd_scan | bitweaving | m7) > <num_of_bits> <num_of_elements> <predicates (1 | 2) > \n", argv[0]);
                exit(1);
        }

	char fileName[50];
	strcpy(fileName, "../m7_microbench/data/");
	strcat(fileName, argv[2]);
	strcat(fileName, "bits_");
	strcat(fileName, argv[3]);
	strcat(fileName, ".txt");
	printf("filename : %s\n", fileName);

	char simdCase[20]; //Necessary for simd_scan, since we have different binaries;
	if(strcmp(argv[1], "simd_scan") == 0){
		strcpy(simdCase, "simd_scan/");
		strcat(simdCase, argv[2]);
		strcat(simdCase, "bits");	
	}

	printf("simd : %s\n", simdCase);

	char *args[6];
	if(strcmp(argv[1], "simd_scan") == 0){
		args[0] = simdCase;
	}
	else if(strcmp(argv[1], "bitweaving") == 0)
		args[0] = "bitweaving/bitweaving";
	else
		args[0] = "sparc_m7/m7";

	args[1] = fileName;
	args[2] = argv[2];
	args[3] = argv[3];	
	args[4] = argv[4];	
	args[5] = NULL;

/*	args[0] = "simd_scan/8bits";
	args[1] = "data/8bits_1000000.txt";
	args[2] = "8";
	args[3] = "1000000";
	args[4] = "100";
	args[5] = NULL;
*/
	printf("Running with parameters: %s %s %s %s %s\n", args[0], args[1], args[2], args[3], args[4]);
	pid_t pid = fork();
	if(pid == -1){
		perror("fork has failed!");
		exit(1);
	}
	else if(pid == 0){
		execvp(args[0], args);
		perror("execv shouldn't have returned");
		exit(EXIT_FAILURE);
	}
	else{
		printf("In the bencmark frontend\n");
	}

	#ifdef __gnu_linux__
		printf("This is a linux machine!\n");
	#elif	__sun
		printf("This is a sun/solaris machine!\n");
	#endif
}
