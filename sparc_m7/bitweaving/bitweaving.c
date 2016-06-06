#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

#include <sys/time.h>
#include <../util/thread.h>

#define WORD_SIZE 64 

pthread_mutex_t start_mutex;
pthread_cond_t start_cond;
int thread_count = 0;

int countSetBits(unsigned int n)
{
  unsigned int count = 0;
  while(n)
  {
      count += n & 1;
      n >>= 1;
  }
  return count;
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

void assignThreadParams(query_params *params){
	params->start_mutex = &start_mutex;
	params->start_cond = &start_cond;
	params->thread_count = &thread_count;
}

void printResultVector(uint64_t *result, int elements, int padded, int remainder){
	uint64_t cur_res;
	uint64_t cur_comp = pow(2, WORD_SIZE - 1 - padded);
	int bits = 63 - padded;
	int count = 0;
	int i;
	for(i = 0; i < elements - 1; i++){
		printf("Segment id : %d\n", i);
		cur_res = result[i];
                //count += __builtin_popcount(cur_res);
                //count += __builtin_popcount(cur_res >> 32);
		while (bits >= 0) {
		    if ((cur_res & cur_comp) >> bits)
			printf("1");
		    else
			printf("0");

		    cur_comp >>= 1;
		    bits--;
		}
		printf("\n");
		bits = 63 - padded;
		cur_comp = pow(2, WORD_SIZE - 1- padded);
	}
	//Last segment with partial
	cur_res = result[i];
	while (bits >= 63 - (padded + remainder - 1)){
	    if ((cur_res & cur_comp) >> bits)
		printf("1");
	    else
		printf("0");

	    cur_comp >>= 1;
	    bits--;
	}

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

void count_query(uint64_t *stream, int start, int end, uint64_t *bit_vector, int num_of_bits, int numberOfSegments, int numberOfElements, int predicate){
	uint64_t upper_bound = (0 << num_of_bits) | predicate; 
	uint64_t mask = (0 << num_of_bits) | (uint64_t) (pow(2, num_of_bits) - 1);
	
	int i, j;
	for(i = 0; i < WORD_SIZE/(num_of_bits + 1) - 1; i++){
		upper_bound |= (upper_bound << (num_of_bits + 1));
		mask |= (mask << (num_of_bits + 1));
	}

	uint64_t cur_result;
	int count = 0;
	int total_codes = num_of_bits + 1; //size of a code with the predicate payload
	uint64_t local_res = 0;
	//long long res;
	//res = tick();
	for(i = start; i < end; i++){
		for(j = 0; j < total_codes; j++){
			cur_result = stream[i * total_codes + j];
			cur_result = cur_result ^ mask;
			cur_result += upper_bound;
			cur_result = cur_result & ~mask;
			local_res = local_res | (cur_result >> j); 
		//	count += countSetBits(cur_result);
		//	count += countSetBits(cur_result >> 32);
		}
		bit_vector[i] = local_res;
		local_res = 0;
	}
//	printf("Count: %d\n", count);
	//int remainder = numberOfElements - (numberOfSegments - 1) * (numberOfElements / (numberOfSegments - 1));
	//printResultVector(global_res, numberOfSegments, WORD_SIZE - (num_of_bits + 1) * (WORD_SIZE / (num_of_bits + 1)), remainder);
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

	int num_of_threads = atoi(argv[5]);
	pthread_t threads[num_of_threads];
	assignThreadParams(params);
	params->num_of_threads = num_of_threads;

	pthread_mutex_init(&start_mutex, NULL);
	pthread_cond_init(&start_cond, NULL);
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	createThreads(threads, &attr, num_of_threads, params);

	int wait = 1;
	do{
		pthread_mutex_lock(&start_mutex);
		wait = (thread_count < num_of_threads);
		pthread_mutex_unlock(&start_mutex);
	} while(wait);

	hrtime_t start, end;
	start = gethrtime();
	pthread_cond_broadcast(&start_cond);
	printf("Threads starting ...\n\n");

	joinThreads(threads, num_of_threads);

	end = gethrtime();
	long long res = end - start;
	printf("It took %lld ns! in total and \n%lf ns for each code. \n\n", res, res / (num_of_elements * 100.0));
	printf("%lf codes were processed in each ns!\n",  (num_of_threads * 1.0) / (res / (num_of_elements * 100.0)));

	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&start_mutex);
	pthread_cond_destroy(&start_cond);

	//free resources
	free(params);
	free(data_stream);
}
