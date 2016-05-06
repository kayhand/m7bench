#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vis.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>

#include "SIMD_buffer.c"
#include "util.h"

//Should be 8, but it's not<
#define FBUFFER_SIZE 8

#define DISPLACEMENT 8

#define error(a) do{ \
	perror(a);\
	exit(EXIT_FAILURE);\
}while(0);

//For a more efficient replacement of
//pop32
int decode_table[256];
int waiting_threads = 0;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;


void count_query(uint64_t * stream, unsigned long long int number_of_buffers, int predicate);

/*
 *Data: 8 bits
 * 
 * Predicate = an upper bound (<)
 *-
 *
 */

void *worker(void *arg){
	struct query_thread *q = arg;
	int i = 1000;

	pthread_mutex_lock(&mut);
	waiting_threads++;
	pthread_cond_wait(&cond, &mut);
	pthread_mutex_unlock(&mut);

	while(i--)
		count_query(q->stream, q->nb_elements, q->predicate);

	pthread_exit(NULL);

}

void count_query(uint64_t * stream, unsigned long long int number_of_buffers, int predicate)
{

	/* Compare lt, with no pointers! */


	uint64_t p64, original_value;
	double d_predicate, d_original;
	int i, j, aux;
	unsigned long long result = 0;
	long long begin, end;
	hrtime_t h_begin, h_end;
	double zero = 0, d_buffer;
	unsigned int bmasks[4] = {
		0xfff0fff1,
		0xfff2fff3,
		0xfff4fff5,
		0xfff6fff7
	};

	p64 = predicate & 0xff;
	p64 <<= 32;
	p64 |= predicate;
	d_predicate = vis_ll_to_double(p64);


	/***********************
	 *
	 * Stream Element
	 * AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD EEEEEEEE FFFFFFFF GGGGGGGG HHHHHHHHH
	 *	0	1	2	3	4	5	  6		7
	 *
	 * *********************/
	for(i=0;i<number_of_buffers;i++){
		original_value = stream[i];
		//22-25 without ll-to-double
		d_original = vis_ll_to_double(original_value);

		for(j=0; j<4; j++){
			//Set the mask for the following Byte shuffles
			//2ns per buffer without_
			__vis_write_bmask(bmasks[j], 0);
			//4ns without
			d_buffer = __vis_bshuffle(d_original, zero);
			//14-17ns without
			aux = vis_fcmplt32(d_buffer, d_predicate);
			//7-20ns without
			result += decode_table[aux];
		}
	}
}

int main(int argc, char ** argv)
{
	uint64_t *t = NULL;
	unsigned long long nb_elements;
	int  predicate, nb_threads, i;
	pthread_t *threads;
	hrtime_t begin, end;
	struct query_thread *qt;

	if(argc < 4){
		printf("Usage: %s filename predicate threads\n", argv[0]);
		exit(EXIT_FAILURE);

	}
	predicate = atoi(argv[2]);
	nb_threads = atoi(argv[3]);


	fill_decode_table(decode_table);
	if((threads = malloc(nb_threads * sizeof(pthread_t))) == NULL)
		error("threads");
	if((qt = malloc(sizeof(struct query_thread) * nb_elements)) == NULL)
		error("qery_threads");

	load_data( argv[1], 8, &nb_elements, &t);


	pthread_mutex_lock(&mut);

	for(i=0;i<nb_threads;i++){
		qt[i].id = i;
		qt[i].predicate = predicate;
		qt[i].nb_elements = nb_elements;
		qt[i].stream = t;
	}	
	//The last threads gets the unattributed remains.
	//qt[i-1].nb_elements += nb_elements % nb_threads;
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
	}
	pthread_mutex_unlock(&mut);

	begin = gethrtime();
	pthread_cond_broadcast(&cond);
	for(i=0;i<nb_threads;i++){
		pthread_join(threads[i], NULL);	
	}
	end = gethrtime();

	printf("DONE, %d threads\n", nb_threads);
	printf("Time elapsed: %llu\n", end - begin);
	printf("For each element it took; %f\n", (end - begin) / (100000000.0 * 1000) );
	printf("In a single ns:%f\n", 1.0 / ((end - begin) / (100000000.0 * 1000)) );
	free(t);	
	free(threads);
	free(qt);
	pthread_cond_destroy(&cond);
	return EXIT_SUCCESS;
}
