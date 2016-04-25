#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

#define WORD_SIZE 64 

void reserveMemory(uint64_t **data_stream, int number_of_segments, int num_of_bits){
	*data_stream = (uint64_t *) malloc(sizeof(uint64_t) * number_of_segments * (num_of_bits + 1) * 2);
}

int load_data(char *fname, int num_of_bits, int codes_per_segment, int numberOfSegments, int predicate, uint64_t *stream){
 	FILE *file;
	if((file = fopen(fname, "r")) == NULL){
		return(-1);
	}

	int curSegment = 0, prevSegment = 0;
	int values_written = 0, codes_written = 0;
	unsigned int newVal;
	uint64_t curVal;
	int rowId = 0;

	int rows = num_of_bits + 1;
	while(fscanf(file, "%u", &newVal) == 1){
		//printf("Val: %u\n", newVal);
		curSegment = values_written / codes_per_segment;
		if(curSegment >= numberOfSegments)
			break;
		if(curSegment != prevSegment){
			codes_written = 0;
			rowId = 0;
		}

		rowId = codes_written % (num_of_bits + 1);
		//Check if we need to append to an existing word
		if(codes_written < (num_of_bits + 1)){
			stream[curSegment * rows + codes_written] = (0 << num_of_bits) | newVal;
			//stream[curSegment][codes_written] = (0 << num_of_bits) | newVal; 
		}
		else{
			curVal = stream[curSegment * rows + rowId];
			curVal = curVal << rows;
			stream[curSegment * rows + rowId] = curVal | ((0 << num_of_bits) | newVal);
		}
		values_written++;
		codes_written++;
		prevSegment = curSegment;
	}

	fclose(file);

	int remainder = codes_per_segment  - (codes_written % codes_per_segment);
	remainder %= codes_per_segment;

	int curSlot = 0;
	while(remainder > 0){
		rowId = codes_written % (num_of_bits + 1);
		curSlot = curSegment * rows + rowId;
		curVal = stream[curSlot];
		curVal = curVal << rows;
		stream[curSlot] = curVal | ((0 << num_of_bits) | predicate);
		codes_written++;
		remainder--;
	}

	printf("In the end %d codes are written!\n", codes_written);
	/*
	for(int i = 0; i < numberOfSegments; i++){
		printf("Segment %d: \n", i);
		for(int j = 0; j < num_of_bits +1; j++){
			printf("%lu\n", stream[i * rows + j]);
		}
	}
	*/
	return(0);
}

void count_query(uint64_t *stream, int num_of_bits, int numberOfSegments, int predicate){
	uint64_t upper_bound = (0 << num_of_bits) | predicate; 
	uint64_t mask = (0 << num_of_bits) | (uint64_t) (pow(2, num_of_bits) - 1);
	
	for(int i = 0; i < WORD_SIZE/(num_of_bits + 1) - 1; i++){
		upper_bound |= (upper_bound << (num_of_bits + 1));
		mask |= (mask << (num_of_bits + 1));
	} 

	uint64_t result_vector, cur_result, lower;
	int count = 0;

	for(int i = 0; i < numberOfSegments; i++){
		for(int j = 0; j < num_of_bits + 1; j++){
			cur_result = stream[i * (num_of_bits + 1) + j];
			cur_result = cur_result ^ mask;
			cur_result += upper_bound;
			cur_result = cur_result & ~mask;
			count += __builtin_popcount(cur_result);
			count += __builtin_popcount(cur_result >> 32);
		}
	}
	printf("Total number of selected values: %d\n", count);	
}

int main(int argc, char * argv[]){
        if(argc < 4){
                printf("Usage: %s <file> <num_of_bits> <num_of_elements> <predicate>\n", argv[0]);
                exit(1);
        }

	int num_of_bits = atoi(argv[2]);
	int num_of_elements = atoi(argv[3]);

	int num_of_codes = WORD_SIZE / (num_of_bits + 1); //Number of codes in each processor word
	printf("%d \n", num_of_codes);
	int codes_per_segment = num_of_codes * (num_of_bits + 1); //Number of codes that fit in a single segment
	printf("%d \n", codes_per_segment);
	int number_of_segments =  ceil(num_of_elements * 1.0 / codes_per_segment); //Total number of segments needed to keep the data
	printf("%d \n", number_of_segments);

	uint64_t *data_stream;
	reserveMemory(&data_stream, number_of_segments, num_of_bits);

	int errno = load_data(argv[1], num_of_bits, codes_per_segment, number_of_segments, atoi(argv[4]), data_stream);
	count_query(data_stream, num_of_bits, number_of_segments, atoi(argv[4]));

	//free resources
	free(data_stream);
}
