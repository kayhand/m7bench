#ifndef THREAD_H_
#define THREAD_H_

#include <pthread.h>

typedef struct {
        uint64_t *stream;
        int num_of_bits;
        int num_of_segments;
        int num_of_elements;
        int predicate;

	void (*count_query)(uint64_t*, int, int, int, int);
} query_params;

void *worker(void *args){
	query_params *query_args = (query_params *) args;
	query_args->count_query(query_args->stream, query_args->num_of_bits, query_args->num_of_segments, query_args->num_of_elements, query_args->predicate);
	return NULL;
}

void createThreads(pthread_t **threads, int number_of_threads){
	*threads = (pthread_t *) malloc(number_of_threads * sizeof(pthread_t));
}

void runThreads(pthread_t *threads, int num_of_threads, void* query_params){
	#ifdef LINUX
		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
	#endif //linux
	for(int i = 0; i < num_of_threads; i++){
		pthread_create(&threads[i], NULL, worker, query_params);
		#ifdef LINUX
			CPU_SET(i, &cpuset);
			pthread_setaffinity_np(&threads[i], sizeof(cpu_set_t), &cpuset);
		#endif 
	}
}

void joinThreads(pthread_t *threads, int num_of_threads){
	for(int i = 0; i < num_of_threads; i++){
		pthread_join(threads[i], NULL);
	}
}

#endif

