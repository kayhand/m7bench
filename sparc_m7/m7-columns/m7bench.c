#include "/opt/dax/dax.h"
//#include "dax_query.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

#include "thread.h"

pthread_mutex_t start_mutex;
pthread_cond_t start_cond;
int thread_count = 0;

void assignThreadParams(query_params *params){
	params->start_mutex = &start_mutex;
	params->start_cond = &start_cond;
	params->thread_count = &thread_count;
}

int load_data(char *fname, int num_of_bits, int num_of_elements, uint64_t **stream){
	FILE *file;
	int newVal = 0, prevVal = 0;
	uint64_t writtenVal = 0;
	unsigned long lineInd = 0;
	if((file = fopen(fname, "r")) == NULL)
		return(-1);

	if (fseek(file, 0, SEEK_END) != 0)
	{
		perror("fseek");
		exit(1); 
	}
	int size_indata = ftell(file);
	rewind(file);
	*stream = (uint64_t *) memalign(8, num_of_elements * sizeof(uint64_t));

	int modVal = 0;
	unsigned long curIndex = 0, prevIndex = 0;
	int8_t first_part, second_part;

	int bits_written = 0;
	int bits_remaining = 0; //Number of bits that roll over to next index from the last value written into the previous index

	int err;
	while(!feof(file)){
		err = fscanf(file, "%u", &newVal);
		curIndex = ((lineInd) * num_of_bits / 64); //Index of the buffer to gather 8 bit-access
		//curIndex = bits_written / 64;
		if(curIndex != prevIndex){
			(*stream)[prevIndex] = writtenVal;
			//printf("Written to index %d : %llu \n", prevIndex, (*stream)[prevIndex]);
			bits_remaining = bits_written - 64;
			bits_written = 0;

			//printf("Bits remaining: %d\n", bits_remaining);
			if(bits_remaining > 0){
				writtenVal = ((uint64_t) prevVal) >> (num_of_bits - bits_remaining);
				bits_written = bits_remaining;
				writtenVal |= ((uint64_t) newVal) << bits_written;
				bits_written += num_of_bits;
			}
			else{
				writtenVal = (uint64_t) newVal;
				bits_written += num_of_bits;
			}
		}
		else{
			writtenVal |= ((uint64_t) newVal) << bits_written;
			bits_written += num_of_bits;
		}

		//printf("%d %d %llu %llu %d \n\n", lineInd, curIndex, writtenVal, newVal, bits_written);
		prevIndex = curIndex;
		prevVal = newVal;
		lineInd++;
	}

	if(file){
		fclose(file);
	}
	return(0);
}

void count_query(dax_vec_t *src, dax_vec_t *dst, dax_int_t *min, dax_int_t *max, uint64_t ** data, int nb_cols){ 
	int i;
	dax_context_t *ctx ;
	dax_result_t res;
	dax_vec_t final = {0}, aux = {0};
	dax_status_t status;

	aux.format = final.format = DAX_BITS;
	aux.elements = final.elements = src->elements;
	aux.elem_width = final.elem_width = 1;
	aux.offset = final.offset = 0;

	aux.data = memalign(8192, DAX_OUTPUT_SIZE(src->elements, src->elem_width));
	final.data = memalign(8192, DAX_OUTPUT_SIZE(src->elements, src->elem_width));

	if(aux.data == NULL || final.data == NULL){
		fprintf(stderr, "Malloc query.\n");
		exit(-1);
	}

	status = dax_thread_init(1,1, 0, NULL, &ctx);

	for(int loop = 0; loop < 1000; loop++){

		for(i=0 ; i < nb_cols; i++){
			src->data = data[i];

			res = dax_scan_range(ctx, 0, src, dst, DAX_GE_AND_LE, min, max);

			if(i == 0)
				dax_copy(ctx, 0, dst->data, final.data, DAX_OUTPUT_SIZE(src->elements, src->elem_width));
			else if(i % 2)
				dax_and(ctx, 0, dst->data, final.data, aux.data);
			else
				dax_and(ctx, 0, dst->data, aux.data, final.data);
		}

		if(i%2)
			dax_copy(ctx, 0, final.data, dst->data, DAX_OUTPUT_SIZE(src->elements, src->elem_width));
		else
			dax_copy(ctx, 0, aux.data, dst->data, DAX_OUTPUT_SIZE(src->elements, src->elem_width));

	}

	//printf("Count : %d\n", res.count);
	(void) dax_thread_fini(ctx);
	free(aux.data);
	free(final.data);
}

int main(int argc, char * argv[])
{
	int nb_files, i;
	dax_context_t *ctx;
	dax_status_t status;
	uint64_t criteria1, criteria2;
	uint64_t **col;
	int count = 6;

	if(argc < 6){
		printf("Usage: %s <num_bits> <num_elements> <min> <max> <nb_threads> [files]\n", argv[0]);
		//			1	2		3  4		5	6->

		exit(1);
	}

	status = dax_thread_init(1,1, 0, NULL, &ctx);

	criteria1 = atoi(argv[3]);
	criteria2 = atoi(argv[4]);

	col = malloc(sizeof(uint64_t *) * (argc - 5));
	if(col == NULL){
		perror("malloc col");
		exit(-1);
	}
	memset(col, NULL, sizeof(uint64_t *) * (argc - 5));

	while(argv[count] != NULL){
		
		if(load_data(argv[count], atoi(argv[1]), atoi(argv[2]), &(col[count - 6])))
		{
			perror("load_data ");
			exit(-1);
		}
		count++;
	}

	query_params f;
	query_params *params = &f;
	params->elements = atoi(argv[2]);
	params->elem_width = atoi(argv[1]);
	params->data = col;
	params->nb_cols = count - 6;
	params->predicate_min = criteria1;
	params->predicate_max = criteria2;
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
	pthread_cond_broadcast(&start_cond);
	printf("Threads starting ...\n\n");

	start = gethrtime();
	joinThreads(threads, num_of_threads);
	end = gethrtime();
	long long res = end - start;
	printf("It took %lld ns! in total and \n%lf ns for each code. \n\n", res, res  / (params->elements * 1000.0));
	printf("In a single ns %lf codes were processed. \n\n", num_of_threads * (1.0 / (res  / (params->elements * 1000.0))));

	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&start_mutex);
	pthread_cond_destroy(&start_cond);

	for(i=0;i<argc-6;i++)
		free(col[i]);
	free(col);
	(void) dax_thread_fini(ctx);
}

