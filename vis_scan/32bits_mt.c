#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vis.h>
#include <sys/time.h>
#include <pthread.h>
#include "SIMD_buffer.c"
#include "util.h"

//Should be 8, but it's not<
#define FBUFFER_SIZE 4

#define DISPLACEMENT 16

#define error(a) do{ \
	perror(a);\
	exit(EXIT_FAILURE);\
}while(0);

void count_query(uint64_t * stream, unsigned long long number_of_buffers, int predicate);

int decode_table[256];
int waiting_threads = 0;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

/*
 *Data: 8 bits
 * 
 * Predicate = an upper bound (<)
 *-
 *
 */


void *worker(void *arg){
	struct query_thread *q = arg;

	pthread_mutex_lock(&mut);
	waiting_threads++;
	printf("I'm thread %d, with %d elements!\n", q->id, q->nb_elements);
	pthread_cond_wait(&cond, &mut);
	pthread_mutex_unlock(&mut);

	if(q->nb_elements);
	count_query(q->stream, q->nb_elements, q->predicate);

}


void count_query(uint64_t * stream, unsigned long long number_of_buffers, int predicate)
{


	uint64_t p64;
	int i, j, aux;
	double d_predicate, d_original;
	unsigned long long begin, end, result=0;
	hrtime_t h_begin, h_end;

	/***************
	 * AAAAAAAA AAAAAAAA AAAAAAAA AAAAAAAA BBBBBBBB BBBBBBBB BBBBBBBB BBBBBBBB
	 * 0		1	2	3	4	 5	  6		7
	 *
	 */
	p64 = predicate & 0xffffffff;
	p64 <<= 32;
	p64 |= predicate;
	d_predicate = vis_ll_to_double(p64);

	h_begin = gethrtime();
	begin = tick();
	for(i=0;i<number_of_buffers;i++){
		d_original = vis_ll_to_double(stream[i]);

		aux = vis_fcmpgt32(d_original, d_predicate);
		result += decode_table[aux];

	}
	end = tick();
	h_end = gethrtime();

	//printf("result = %llu\n", result);
	printf("Query completed in %lluns, %lluns per buffer, %lluticks, %lluticks per buffer\n",
			h_end - h_begin,
			(h_end - h_begin) / number_of_buffers,
			end - begin,
			(end - begin)/number_of_buffers);

}


int main(int argc, char ** argv)
{

	uint64_t *t = NULL;
	unsigned long long nb_elements;
	int  predicate, nb_threads, i;
	int per_thread;
	pthread_t *threads;
	struct query_thread *qt;

	if(argc < 4){
		printf("Usage: %s filename predicate nb_threads\n", argv[0]);
		exit(EXIT_FAILURE);

	}
	predicate = atoi(argv[2]);
	nb_threads = atoi(argv[3]);

	printf("Loading data\n");
	load_data(argv[1], 32, &nb_elements, &t);
	printf("Loaded\n");


	fill_decode_table(decode_table);
	if((threads = malloc(nb_threads * sizeof(pthread_t))) == NULL)
		error("threads");
	if((qt = malloc(sizeof(struct query_thread) * nb_elements)) == NULL)
		error("qery_threads");

	per_thread = nb_elements / nb_threads;


	pthread_mutex_lock(&mut);

	for(i=0;i<nb_threads;i++){
		qt[i].id = i;
		qt[i].predicate = predicate;
		qt[i].nb_elements = per_thread;
		qt[i].stream = t + (per_thread * i);
	}	
	//The last threads gets the unattributed remains.
	qt[i-1].nb_elements += nb_elements % nb_threads;
	pthread_mutex_unlock(&mut);

	for(i=0;i<nb_threads;i++){
		pthread_create(&(threads[i]), NULL, worker, &(qt[i]));
	}

	/*
	 * Launches all threads at the same time
	 */
	pthread_mutex_lock(&mut);
	while(waiting_threads != nb_threads){
		pthread_mutex_unlock(&mut);
		pthread_mutex_lock(&mut);
	}//Loop untils all threads are waiting on the condition
	pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&mut);

	for(i=0;i<nb_threads;i++){
		pthread_join(threads[i], NULL);	
	}

	printf("DONE, %d threads\n", nb_threads);
	free(t);
	free(threads);
	free(qt);
	pthread_cond_destroy(&cond);
	return EXIT_SUCCESS;
}


