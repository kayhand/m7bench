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

#define error(a) do{\
	perror(a);\
	exit(-1);\
	while(0)

#define QLEN 30

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
		exit(-1); 
	}
	int size_indata = ftell(file);
	rewind(file);
	*stream = (uint64_t *) memalign(8192, num_of_elements * sizeof(uint64_t));
	if(*stream == NULL){
		perror("Load malloc : ");
		exit(-1);
	}

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


void count_select(dax_vec_t *src_v, dax_vec_t *dst_v, dax_int_t *min, dax_int_t *max, uint64_t ** data, int nb_cols){ 
	int i;
	uint64_t *new_data;
	void *p;
	dax_context_t *ctx;
	dax_vec_t *index,
		  *index_aux,
		  *mask,
		  *new, //Will contain the reduced version of data
		  *ones; //A vector full of '1', length = src.length


	index = memalign(8192 ,sizeof(dax_vec_t));
	memset(index, 0, sizeof(dax_vec_t));
	index_aux = memalign(8192 ,sizeof(dax_vec_t));
	memset(index_aux, 0, sizeof(dax_vec_t));
	new = memalign(8192 ,sizeof(dax_vec_t));
	memset(new, 0, sizeof(dax_vec_t));
	mask = memalign(8192 ,sizeof(dax_vec_t));
	memset(mask, 0, sizeof(dax_vec_t));
	ones = memalign(8192 ,sizeof(dax_vec_t));
	memset(mask, 0, sizeof(dax_vec_t));

	index->elements = src_v->elements;
	index->elem_width=src_v->elem_width;
	index->data = memalign(8192 ,DAX_OUTPUT_SIZE(index->elements, index->elem_width));
	if(index->data == NULL){
		perror("malloc");
		exit(-1);
	}

	new->elements = src_v->elements;
	new->elem_width=src_v->elem_width;
	new->data = memalign(8192 ,DAX_OUTPUT_SIZE(new->elements, new->elem_width));
	if(new->data == NULL){
		perror("malloc");
		exit(-1);
	}
	index_aux->elements = src_v->elements;
	index_aux->elem_width=src_v->elem_width;
	index_aux->data = memalign(8192 ,DAX_OUTPUT_SIZE(index_aux->elements, index_aux->elem_width));
	if(index_aux->data == NULL){
		perror("malloc");
		exit(-1);
	}
	mask->format = DAX_BITS;
	mask->elements = src_v->elements;
	mask->elem_width = 1;
	mask->data = memalign(8192 ,DAX_OUTPUT_SIZE(mask->elements, mask->elem_width));
	if(mask->data == NULL){
		perror("malloc");
		exit(-1);
	}
	ones->format = DAX_BITS;
	ones->elements = src_v->elements;
	ones->elem_width = 1;
	ones->data = memalign(8192 ,DAX_OUTPUT_SIZE(ones->elements, ones->elem_width));
	if(ones->data == NULL){
		perror("malloc");
		exit(-1);
	}

	dax_thread_init(1,1,0,NULL, &ctx);
	//dax_fill(ctx, 0, 1, mask->data, mask->elements, 1);
	memset(mask->data, 0, DAX_OUTPUT_SIZE(mask->elements, mask->elem_width));

	src_v->data = data[0];

	// predicate to index
	dax_scan_range(ctx, DAX_ONES_INDEX, src_v, index, DAX_GE_AND_LE, min, max);
	/*
	//Uses a vector full of ones to avoid doing a new comparison
	//This creates a boolean result mask
	dax_translate(ctx, 0, index, &mask, ones, 1);
	src_v->data = data[1];
	dax_select(ctx, 0, src_v, new, &mask);
	*/

	//dax_fill(ctx, 0, 1, ones->data, ones->elements, 1);
	memset(ones->data, 1, DAX_OUTPUT_SIZE(ones->elements, ones->elem_width));

	/**
	 * In the loop, each iteration starts with an index vector (I)  and ends with it.
	 *
	 * First, I is translated into a boolean vector B.
	 * Then, the input data (D)  is selected (culled) according to B and creates
	 * new data (nD).
	 * nD is scanned, creating the resulting boolean vector (R)
	 * I is selected against R, creating the input for the next iteration.
	 */
	for(i=1; i<nb_cols;i++){
		//dax_fill(ctx, 0, 1, mask->data, mask->elements, 1);
		dax_translate(ctx, 0, index, mask, ones, index->elem_width);
		src_v->data = data[i];
		dax_select(ctx, 0, src_v, new, mask);
		dax_scan_range(ctx, 0, new, mask, DAX_GE_AND_LE, min, max);
		dax_select(ctx, 0, index, index_aux, mask);

		p = index;
		index = index_aux;
		index_aux = p;
	}


	dax_thread_fini(ctx);
	free(index);
	free(index_aux);
	free(mask);
	free(new);
	free(ones);


}

void count_queue(dax_vec_t **src_v, dax_vec_t *dst_v, dax_int_t *min, dax_int_t *max, int nb_cols){ 
	int i, j, nb_final;
	int wait_for = 0;
	dax_context_t *ctx;
	dax_queue_t *queue;
	dax_status_t status;
	dax_vec_t *dst;
	dax_vec_t aux = {0};
	dax_poll_t *poll_v;

	/*
	src = memalign(8192, sizeof(dax_vec_t) * nb_cols);
	if(src == NULL){
		perror("memalign");
		exit(-1);
	}
	*/
	dst = memalign(8192, sizeof(dax_vec_t) * nb_cols * 2);
	if(dst == NULL){
		perror("memalign");
		exit(-1);
	}
	poll_v = memalign(8192, sizeof(dax_poll_t) * nb_cols);
	if(poll_v == NULL){
		perror("Poll mem");
		exit(-1);
	}

	for(i=0 ; i < (nb_cols * 2) ; i++){
		dst[i].format = dst_v->format;
		dst[i].elements = dst_v->elements;
		dst[i].elem_width = dst_v->elem_width;
		dst[i].offset = 0;

		dst[i].data = memalign(8192, DAX_OUTPUT_SIZE(dst[i].elements, dst[i].elem_width));
		if(dst[i].data == NULL)
			perror("Memalign");
	}

	status = dax_thread_init(1,1,0,NULL, &ctx);
	if(dax_queue_create(ctx, QLEN, &queue) != DAX_SUCCESS){
		perror("queue");
		exit(-1);
	}


	for(i=0;i<nb_cols;i++){
		if(dax_scan_range_post(queue, 0, src_v[i], &(dst[i]), DAX_GE_AND_LE, min, max, (void *) i) != 0)
		{
			perror("scan");
			exit(-1);
		}
	}
	while(i>0){
		j = dax_poll(queue, poll_v, i, -1);
		if(j<0){
			perror("poll (queue)");
			fprintf(stderr, "Value j was: %d, i was %d, nb cols was: %d\n", j, i, nb_cols);
			exit(-1);
		}
		printf("Count: %d\n", poll_v->count);
		
		i-=j;
	}

	/*
	 * NO AND !
	 */

	/*
	for(i=0;i<nb_cols;i++){

		free(src[i].data);
		free(dst[i].data);
	}
	for(i=nb_cols ; i < (nb_cols * 2) ; i++)
		free(dst[i].data);

	free(dst);
	free(src);
	free(poll_v);

	dax_queue_destroy(queue);
	dax_thread_fini(ctx);

	return ;

	/*
	 * END NO AND!
	 */ 

	j = 0;
	int r;
	switch(nb_cols){

		case 1:
			free(dst_v->data);
			dst_v->data = dst[0].data;
			dst[0].data = NULL;
			break;
		case 2:
			r = dax_and_post(queue, 0, &(dst[0]), &(dst[1]), dst_v, (void *) 1);
			if(r != 0){
				fprintf(stderr, "Error: %d : %d\n", r, __LINE__);
				exit(-1);
			}
			wait_for = 1;
			break;
		case 3:
			r = dax_and_post(queue, DAX_SERIAL, &(dst[0]), &(dst[1]), &(dst[3]), NULL);
			if(r != 0){
				fprintf(stderr, "Error: %d : %d\n", r, __LINE__);
				exit(-1);
			}

			r = dax_and_post(queue, DAX_COND, &(dst[2]), &(dst[3]), dst_v, (void *) 1);
			if(r != 0){
				fprintf(stderr, "Error: %d : %d\n", r, __LINE__);
				exit(-1);
			}
			wait_for = 2;
			break;

		case 4:
			r = dax_and_post(queue, DAX_SERIAL, &(dst[0]), &(dst[1]), &(dst[4]), NULL);
			if(r != 0){
				fprintf(stderr, "Error: %d : %d\n", r, __LINE__);
				exit(-1);
			}

			r = dax_and_post(queue, DAX_COND | DAX_SERIAL, &(dst[2]), &(dst[3]), &(dst[5]), NULL);
			if(r != 0){
				fprintf(stderr, "Error: %d : %d\n", r, __LINE__);
				exit(-1);
			}
			r = dax_and_post(queue, DAX_COND, &(dst[4]), &(dst[5]), dst_v, (void *) 1);
			if(r != 0){
				fprintf(stderr, "Error: %d : %d\n", r, __LINE__);
				exit(-1);
			}
			wait_for = 3;
			break;

		case 8:

			r = dax_and_post(queue, DAX_SERIAL, &(dst[0]), &(dst[1]), &(dst[8]), NULL);
			if(r != 0){
				fprintf(stderr, "Error: %d : %d\n", r, __LINE__);
				exit(-1);
			}
			r = dax_and_post(queue, DAX_SERIAL, &(dst[2]), &(dst[3]), &(dst[9]), NULL);
			if(r != 0){
				fprintf(stderr, "Error: %d : %d\n", r, __LINE__);
				exit(-1);
			}
			r = dax_and_post(queue, DAX_SERIAL, &(dst[4]), &(dst[5]), &(dst[10]), NULL);
			if(r != 0){
				fprintf(stderr, "Error: %d : %d\n", r, __LINE__);
				exit(-1);
			}
			r = dax_and_post(queue, DAX_SERIAL, &(dst[6]), &(dst[7]), &(dst[11]), NULL);
			if(r != 0){
				fprintf(stderr, "Error: %d : %d\n", r, __LINE__);
				exit(-1);
			}
			r = dax_and_post(queue, DAX_SERIAL, &(dst[8]), &(dst[9]), &(dst[0]), NULL);
			if(r != 0){
				fprintf(stderr, "Error: %d : %d\n", r, __LINE__);
				exit(-1);
			}
			r = dax_and_post(queue, DAX_SERIAL, &(dst[10]), &(dst[11]), &(dst[1]), NULL);
			if(r != 0){
				fprintf(stderr, "Error: %d : %d\n", r, __LINE__);
				exit(-1);
			}
			r = dax_and_post(queue, DAX_SERIAL, &(dst[0]), &(dst[1]), dst_v, (void *) 1);
			if(r != 0){
				fprintf(stderr, "Error: %d : %d\n", r, __LINE__);
				exit(-1);
			}

			break;
		default:
			fprintf(stderr, "undefined behavior!\n");
			exit(-1);
	}

	while(wait_for){
		j =dax_poll(queue, poll_v, wait_for,  -1);
		if( j < 0){
			perror("poll");
			exit(-1);
		}
		wait_for -= j;
	}

	for(i=0 ; i < (nb_cols * 2) ; i++)
		free(dst[i].data);

	free(dst);
	free(poll_v);

	dax_queue_destroy(queue);
	dax_thread_fini(ctx);
}
 

void count_query(dax_vec_t ** src, dax_vec_t *dst, dax_int_t *min, dax_int_t *max, int nb_cols){ 
	int i;
	dax_context_t *ctx ;
	dax_result_t res;
	dax_vec_t final = {0}, aux = {0};
	dax_status_t status;

	aux.format = final.format = DAX_BITS;
	aux.elements = final.elements = src[0]->elements;
	aux.elem_width = final.elem_width = 1;
	aux.offset = final.offset = 0;

	aux.data = memalign(8192, DAX_OUTPUT_SIZE(dst->elements, 1));
	final.data = memalign(8192, DAX_OUTPUT_SIZE(dst->elements, 1));

	if(aux.data == NULL || final.data == NULL){
		fprintf(stderr, "Malloc query.\n");
		exit(-1);
	}

	status = dax_thread_init(1,1, 0, NULL, &ctx);


	for(i=0 ; i < nb_cols; i++){

		res = dax_scan_range(ctx, 0, src[i], dst, DAX_GE_AND_LE, min, max);
		//printf("Result: %d, Status: %d\t", res.count, res.status);
		//printf("Data format: %d\n", src[i]->format);

		if(i == 0)
			dax_copy(ctx, 0, dst->data, final.data, DAX_OUTPUT_SIZE(src[0]->elements, 1));
		else if(i % 2)
			dax_and(ctx, 0, dst->data, final.data, aux.data);
		else
			dax_and(ctx, 0, dst->data, aux.data, final.data);
	}

	if(i%2)
		dax_copy(ctx, 0, final.data, dst->data, DAX_OUTPUT_SIZE((src[0]->elements), 1));
	else
		dax_copy(ctx, 0, aux.data, dst->data, DAX_OUTPUT_SIZE(src[0]->elements, 1));



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

	if(argc < 7){
		printf("Usage: %s <num_bits> <num_elements> <min> <max> <nb_threads> [files]\n", argv[0]);
		//			1	2		3  4		5	6->

		exit(1);
	}

	status = dax_thread_init(1,1, 0, NULL, &ctx);

	criteria1 = atoi(argv[3]);
	criteria2 = atoi(argv[4]);

	col = memalign(8 ,sizeof(uint64_t *) * (argc - 5));
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
	//params->count_query = count_queue;

	int num_of_threads = atoi(argv[5]);
	pthread_t threads[num_of_threads];
	assignThreadParams(params);
	params->num_of_threads = num_of_threads;

	pthread_mutex_init(&start_mutex, NULL);
	pthread_cond_init(&start_cond, NULL);
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	//COMPRESSION
	prepare_vecs(params);
	//compress(params, ctx);

	createThreads(threads, &attr, num_of_threads, params);

	printf("Threads: %d\n", num_of_threads);
	int wait = 1;
	do{
		pthread_mutex_lock(&start_mutex);
		wait = (thread_count < num_of_threads);
		pthread_mutex_unlock(&start_mutex);
	} while(wait);


	hrtime_t start, end;

	start = gethrtime();
	pthread_cond_broadcast(&start_cond);
	joinThreads(threads, num_of_threads);
	end = gethrtime();
	long long res = end - start;
	printf("It took %lld ns! in total and \n%lf ns for each code. \n\n", res, res  / (params->elements * 1000.0));
	printf("In a single ns %f codes were processed. \n\n", num_of_threads * (1.0 / (res  / (params->elements * 1000.0))));

	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&start_mutex);
	pthread_cond_destroy(&start_cond);

	for(i=0;i<params->nb_cols;i++){
		if(params->vecs[i]->codec != NULL)
			dax_zip_free(ctx, params->vecs[i]->codec);
		free(params->vecs[i]->data);
		free(params->vecs[i]);
	}
	free(params->vecs);
	free(col);
	(void) dax_thread_fini(ctx);
}
