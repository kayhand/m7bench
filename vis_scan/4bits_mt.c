#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vis.h>
#include <pthread.h>
#include "SIMD_buffer.c"
#include "util.h"

#define FBUFFER_SIZE 16
#define DISPLACEMENT 4

#define error(a) do{ \
	perror(a);\
	exit(EXIT_FAILURE);\
}while(0);


int decode_table[256];
int waiting_threads = 0;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

void count_query(uint64_t * stream, int number_of_buffers, int predicate)
{

	/* Compare lt, with no pointers! */

	double values[2], d_original[2];
	double zero = 0, d_predicate;
	uint64_t original_values[2], p64;
	int i, j, aux;
	double clean;
	unsigned long long int result = 0;
	unsigned int bmasks[4] = {
		0xfff0fff1,
		0xfff2fff3,
		0xfff4fff5,
		0xfff6fff7,
	};

	p64 = predicate & 0x0f;
	p64 <<= 32;
	p64 |= predicate;
	d_predicate = vis_ll_to_double(p64);

	clean = vis_ll_to_double(0x0F0F0F0F0F0F0F0F);



	/* *
	 * Stream Element:
	 * AAAABBBB CCCCDDDD EEEEFFFF GGGGHHHH IIIIJJJJ KKKKLLLL MMMMNNNN OOOOPPPP
	 *
	 * src:
	 * 0000BBBB 0000DDDD 0000FFFF 0000HHHH 0000JJJJ 0000LLLL 0000NNNN 0000PPPP
	 * dst
	 * 0000AAAA 0000CCCC 0000EEEE 0000GGGG 0000IIII 0000KKKK 0000MMMM 0000OOOO
	 * 0		1	2	3	4	5	 6	  7
	 *
	 * SO each mask will be applied twice, once on dest, then on src.
	 *
	 * To specify the bmask, vis_write_bmask(MASK, 0);
	 *
	 */

	for(i=0;i<number_of_buffers;i++){
		original_values[0] = stream[i];
		d_original[0] = vis_ll_to_double(original_values[0]);
		d_original[1] = vis_ll_to_double(original_values[0]/16);

	d_original[0] = vis_fand(d_original[0], clean);
	d_original[1] = vis_fand(d_original[1], clean);

		for(j=0; j<4; j++){
			//Set the mask for the following Byte shuffles
			__vis_write_bmask(bmasks[j], 0);
			values[0] = __vis_bshuffle(d_original[0], zero);
			values[1] = __vis_bshuffle(d_original[1], zero);

			aux = vis_fcmpgt32(values[0], d_predicate);
			result += decode_table[aux];

			aux = vis_fcmpgt32(values[1], d_predicate);
			result += decode_table[aux];
		}




	}
	printf("result %llu\n", result);
}

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

	load_data( argv[1], 4, &nb_elements, &t);


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
	printf("In a single ns:%f\t", 1.0 / ((end - begin) / (100000000.0 * 1000)) );
	printf(":%f\n", nb_threads * ( 1.0 / ((end - begin) / (100000000.0 * 1000)) ));
	free(t);
	free(threads);
	free(qt);
	pthread_cond_destroy(&cond);
	return EXIT_SUCCESS;}

