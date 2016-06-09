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

#include <sched.h>
#include <pthread.h>


typedef struct {
	int elements;
	int elem_width;
	int predicate_min, predicate_max;
	int nb_cols;
	uint64_t **data;

	int num_of_threads;
	pthread_mutex_t *start_mutex;
	pthread_cond_t *start_cond;
	int *thread_count;

	void (*count_query)(dax_vec_t *src, dax_vec_t *dst, dax_int_t *min, dax_int_t *max, uint64_t ** data, int nb_cols);
} query_params;


void *worker(void *args){
	int i;
	query_params *query_args = (query_params *) args;
	dax_int_t bound1 = {0}, bound2 = {0};
	dax_vec_t src = {0}, dst = {0};

	src.elements = query_args->elements;
	src.format = DAX_BITS;
	src.elem_width = query_args->elem_width;
	src.offset = 0;

	//src.data = malloc(src.elements * src.elem_width); //Was this necessary?
	src.data = query_args->data;


	bound1.format = DAX_BITS;
	bound1.elem_width = src.elem_width;
	bound1.dword[2] = query_args->predicate_min;

	bound2.format = DAX_BITS;
	bound2.elem_width = src.elem_width;
	bound2.dword[2] = query_args->predicate_max;

	dst.elements = src.elements;
	dst.format = DAX_BITS;
	dst.elem_width = 1;
	dst.data = memalign(8192, DAX_OUTPUT_SIZE(dst.elements, dst.elem_width));
	dst.offset = 0;

	pthread_mutex_lock(query_args->start_mutex);
	pthread_t cur_thread = pthread_self();
	int thread_count = *(query_args->thread_count);
	int cpu_id = 0;

	*(query_args->thread_count) += 1;

	pthread_cond_wait(query_args->start_cond, query_args->start_mutex);	
	pthread_mutex_unlock(query_args->start_mutex);

	query_args->count_query(&src, &dst, &bound1, &bound2, query_args->data, query_args->nb_cols);

	free(dst.data);
	pthread_exit(NULL);
}

void createThreads(pthread_t *threads, pthread_attr_t *attr, int num_of_threads, void* query_params){
	int i;
	for(i = 0; i < num_of_threads; i++){
		pthread_create(&threads[i], attr, worker, query_params);
	}
}

void joinThreads(pthread_t *threads, int num_of_threads){
	int i;
	for(i = 0; i < num_of_threads; i++){
		pthread_join(threads[i], NULL);
	}
}
