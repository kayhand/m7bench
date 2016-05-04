#ifndef THREAD_H_
#define THREAD_H_

#include <pthread.h>

typedef struct {
        uint64_t *stream;
        uint64_t bit_vector;
        int num_of_bits;
        int num_of_segments;
        int num_of_elements;
        int predicate;

	pthread_mutex_t *start_mutex;
	pthread_cond_t *start_cond;
	int *thread_count;

	void (*count_query)(uint64_t*, uint64_t*, int, int, int, int);
} query_params;

#ifdef LINUX
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
#endif //linux

void *worker(void *args){
	query_params *query_args = (query_params *) args;
	uint64_t *bit_vector = (uint64_t *) malloc(sizeof(uint64_t) * (query_args->num_of_segments));

	pthread_mutex_lock(query_args->start_mutex);
	pthread_t cur_thread = pthread_self();
	#ifdef LINUX
		CPU_SET(i, &cpuset);
		pthread_setaffinity_np(cur_thread, sizeof(cpu_set_t), &cpuset);
	#endif 	
	*(query_args->thread_count) += 1;
	printf("Current thread count: %d\n", *(query_args->thread_count));
	pthread_cond_wait(query_args->start_cond, query_args->start_mutex);	
	pthread_mutex_unlock(query_args->start_mutex);

	for(int loop = 0; loop < 100; loop++)
		query_args->count_query(query_args->stream, bit_vector, query_args->num_of_bits, query_args->num_of_segments, query_args->num_of_elements, query_args->predicate);

	free(bit_vector);
	pthread_exit(NULL);
}

void createThreads(pthread_t *threads, pthread_attr_t *attr, int num_of_threads, void* query_params){
	for(int i = 0; i < num_of_threads; i++){
		pthread_create(&threads[i], attr, worker, query_params);
	}
}

void joinThreads(pthread_t *threads, int num_of_threads){
	for(int i = 0; i < num_of_threads; i++){
		pthread_join(threads[i], NULL);
	}
}

#endif

