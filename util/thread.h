#include <pthread.h>
#include <sched.h>

#ifndef __gnu_linux__
#include <sys/lgrp_user.h>
#include <sys/types.h>
#include <sys/mman.h>
#endif

typedef struct {
	//When querying several columns
        uint64_t ** stream;
        uint64_t *bit_vector;
        int num_of_bits;
        int num_of_segments;
        int num_of_elements;
        int predicate, predicate_max;
	int num_of_threads;

	int nb_streams, count_stream;

	pthread_mutex_t *start_mutex;
	pthread_cond_t *start_cond;
	int *thread_count;

	void (*count_query)(uint64_t *stream, int start, int end, uint64_t *bit_vector, int num_of_bits, int numberOfSegments, int numberOfElements, int predicate, int predicate2);
} query_params;

#ifndef __gnu_linux__
//Set the process afinity with this memory
void set_affinity(uint64_t *e){
	uint_t req = MEMINFO_VLGRP; //Get local group
	uint64_t lgrp;
	uint_t valid;

	if(meminfo((const unsigned long long *)&e, 1, &req, 1, &lgrp, &valid) != 0){
		perror("meminfo");
		exit(-1);
	}

	if(lgrp_affinity_set(P_LWPID, P_MYID, lgrp, LGRP_AFF_STRONG) != 0){
		perror("affinity ");
		exit(-1);
	}
}
#endif

int returnCpuId(int thread_count){
	int cpu_id = 0;
	if(thread_count < 20){
		if(thread_count % 2 == 0)
			cpu_id = thread_count / 2;
		else
			cpu_id = 10 + thread_count / 2;
	}		
	else{
		if(thread_count % 2 == 0)
			cpu_id = 20 + (thread_count / 2) % 10;
		else
			cpu_id = 30 + (thread_count / 2) % 10;
	}	
	return cpu_id;	
}

void *worker(void *args){
	int i;
	query_params *query_args = (query_params *) args;
	
	uint64_t *bit_vector = (uint64_t *) malloc(sizeof(uint64_t) * (query_args->num_of_segments));
	uint64_t *aux_vector = NULL;

	memset(bit_vector, 255, sizeof(uint64_t) * (query_args->num_of_segments));

	pthread_mutex_lock(query_args->start_mutex);
	pthread_t cur_thread = pthread_self();
	int thread_count = *(query_args->thread_count);
	int cpu_id = 0;

#ifdef __gnu_linux__
		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		cpu_id = returnCpuId(thread_count);
		CPU_SET(cpu_id , &cpuset);
		pthread_setaffinity_np(cur_thread, sizeof(cpu_set_t), &cpuset);
#else
		set_affinity(query_args->stream[0]);
#endif

	int block_size = query_args->num_of_segments / query_args->num_of_threads;
	int start = thread_count * block_size;
	int end = start + block_size;

	*(query_args->thread_count) += 1;
	if(*(query_args->thread_count) == query_args->num_of_threads)
		end = query_args->num_of_segments;

	pthread_cond_wait(query_args->start_cond, query_args->start_mutex);	
	pthread_mutex_unlock(query_args->start_mutex);

	int loop;
	for(loop = 0; loop < 1000; loop++){ //Loops a thousand times and then
		i=0;
		//on each column.
		//First, the last argument of the query functions tells whether aux_vector holds any meaningful values

		//printf("query nb: %d\n", query_args->nb_streams);

		for(i=0 ; i < query_args->nb_streams; i++){
				query_args->count_query(query_args->stream[i], start, end, bit_vector, 
						query_args->num_of_bits, query_args->num_of_segments, 
						query_args->num_of_elements, query_args->predicate, query_args->predicate_max);


		}
	}

	query_args->bit_vector = bit_vector;

	free(aux_vector);
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
