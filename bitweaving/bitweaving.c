#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <../util/thread.h>

#define WORD_SIZE 64 

static __inline__ unsigned long long tick(void){
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

void reserveMemory(uint64_t **data_stream, int number_of_segments, int num_of_bits){
	*data_stream = (uint64_t *) malloc(sizeof(uint64_t) * number_of_segments * (num_of_bits + 1) * 2);
	//*data_stream = (uint64_t *) memalign(16, number_of_segments * (num_of_bits + 1) * 2 * sizeof(uint64_t));
}

void assignParams(query_params **params, uint64_t *data_stream, int num_of_bits, int num_of_segments, int num_of_elements, int predicate){
	*params = (query_params *) malloc(sizeof(query_params));

	(*params)->stream = data_stream;
	(*params)->num_of_bits = num_of_bits;
	(*params)->num_of_segments = num_of_segments;
	(*params)->num_of_elements = num_of_elements;
	(*params)->predicate = predicate;
}

int load_data(char *fname, int codes_per_segment, query_params *params){
 	FILE *file;
	if((file = fopen(fname, "r")) == NULL){
		return(-1);
	}

	int curSegment = 0, prevSegment = 0;
	int values_written = 0, codes_written = 0;
	unsigned int newVal;
	uint64_t curVal;
	int rowId = 0;

	int rows = params->num_of_bits + 1;
	while(fscanf(file, "%u", &newVal) == 1){
		//printf("Val: %u\n", newVal);
		curSegment = values_written / codes_per_segment;
		if(curSegment >= params->num_of_segments)
			break;
		if(curSegment != prevSegment){
			codes_written = 0;
			rowId = 0;
		}

		rowId = codes_written % (params->num_of_bits + 1);
		//Check if we need to append to an existing word
		if(codes_written < (params->num_of_bits + 1)){
			params->stream[curSegment * rows + codes_written] = (0 << params->num_of_bits) | newVal;
			//params->stream[curSegment][codes_written] = (0 << params->num_of_bits) | newVal; 
		}
		else{
			curVal = params->stream[curSegment * rows + rowId];
			curVal = curVal << rows;
			params->stream[curSegment * rows + rowId] = curVal | ((0 << params->num_of_bits) | newVal);
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
		rowId = codes_written % (params->num_of_bits + 1);
		curSlot = curSegment * rows + rowId;
		curVal = params->stream[curSlot];
		curVal = curVal << rows;
		params->stream[curSlot] = curVal | ((0 << params->num_of_bits) | params->predicate);
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

void count_query(uint64_t *stream, int num_of_bits, int numberOfSegments, int numberOfElements, int predicate){
	uint64_t upper_bound = (0 << num_of_bits) | predicate; 
	uint64_t mask = (0 << num_of_bits) | (uint64_t) (pow(2, num_of_bits) - 1);
	
	for(int i = 0; i < WORD_SIZE/(num_of_bits + 1) - 1; i++){
		upper_bound |= (upper_bound << (num_of_bits + 1));
		mask |= (mask << (num_of_bits + 1));
	} 

	uint64_t result_vector, cur_result, lower;
	int count = 0;

	long long res;
	res = tick();

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
	res = tick() - res;

	printf("Selected %d values in %lld cycles! \nIt took %lf cycles for each code. \n\n", count, res, res  / (numberOfElements * 1.0));
}

int main(int argc, char * argv[]){

        if(argc < 5){
                printf("Usage: %s <file> <num_of_bits> <num_of_elements> <predicate> <num_of_threads>\n", argv[0]);
                exit(1);
        }

	int num_of_bits = atoi(argv[2]);
	int num_of_elements = atoi(argv[3]);
	int predicate = atoi(argv[4]);

	int num_of_codes = WORD_SIZE / (num_of_bits + 1); //Number of codes in each processor word
	int codes_per_segment = num_of_codes * (num_of_bits + 1); //Number of codes that fit in a single segment
	int num_of_segments =  ceil(num_of_elements * 1.0 / codes_per_segment); //Total number of segments needed to keep the data

	uint64_t *data_stream;
	reserveMemory(&data_stream, num_of_segments, num_of_bits);

	query_params *params;
	assignParams(&params, data_stream, num_of_bits, num_of_segments, num_of_elements, predicate); 

	int errno = load_data(argv[1], codes_per_segment, params);
	params->count_query = count_query;

	pthread_t *threads;
	int num_of_threads;

	if(atoi(argv[5]) < 2)
		count_query(data_stream, num_of_bits, num_of_segments, num_of_elements, atoi(argv[4]));
	else{
		num_of_threads = atoi(argv[5]);
		createThreads(&threads, num_of_threads);
		runThreads(threads, num_of_threads, params);
		joinThreads(threads, num_of_threads);
	}

	//free resources
	free(params);
	free(data_stream);
	if(atoi(argv[5]) > 1)
		free(threads);
}
