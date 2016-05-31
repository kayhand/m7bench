#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vis.h>
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

void count_query(uint64_t * stream, int number_of_buffers, int predicate)
{


	uint64_t p64, original_value;
	double d_predicate, d_original;
	double d_values[3];
	double d_clean;
	int i, j, aux; 
	unsigned long long result=0;
	double zero = 0, d_buffer;
	unsigned int bmasks[8] = {
		//Stream[1,2]
		0xff01ff12,
		0xff34ff45,
		0xff67ff78,

		//Stream[2,3]
		0xff12ff23,
		0xff45ff56,
		0xff78ff89,
		0xffabffbc,
		0xffdeffef
	};

	/***************
	 *
	 * Stream[i]
	 * AAAAAAAA AAAABBBB BBBBBBBB CCCCCCCC CCCCDDDD DDDDDDDD EEEEEEEE EEEEFFFF 
	 *
	 * Stream[i+1]
	 * FFFFFFFF GGGGGGGG GGGGHHHH HHHHHHHH IIIIIIII IIIIJJJJ JJJJJJJJ KKKKKKKK
	 *
	 * Stream[i+2]
	 * LLLLMMMM MMMMMMMM NNNNNNNN NNNNOOOO OOOOOOOO PPPPPPPP PPPPQQQQ QQQQQQQQ
	 *
	 *	0	1	2	3	4	5	  6		7
	 */
	p64 = predicate & 0xfff;
	p64 <<= 36;
	p64 |= predicate;
	d_predicate = vis_ll_to_double(p64);

	p64 = 0xfff0;
	p64 <<=32;
	p64 |= 0xfff;
	d_clean = vis_ll_to_double(p64);

	for(i=0;i<number_of_buffers;i+=3){

		//We need three buffers at a time
		for(j=0;j<3;j++){
			d_values[j] = vis_ll_to_double(stream[i+j]);
		}

		
		/**
		 * Six first values
		 */

		for(j=0; j<3; j++){
			//Set the mask for the following Byte shuffles
			__vis_write_bmask(bmasks[j], 0);
			d_buffer = __vis_bshuffle(d_values[0], d_values[1]);
			d_buffer = __vis_fand(d_buffer, d_clean);

			aux = vis_fcmpgt32(d_buffer, d_predicate);
			result += decode_table[aux];

		}

		/*
		 * Ten last values
		 */
		for(j=3; j<8; j++){
			//Set the mask for the following Byte shuffles
			__vis_write_bmask(bmasks[j], 0);
			d_buffer = __vis_bshuffle(d_values[1], d_values[2]);
			d_buffer = __vis_fand(d_buffer, d_clean);

			aux = vis_fcmplt32(d_buffer, d_predicate);
			result += decode_table[aux];

		}
	}



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

	load_data( argv[1], 12, &nb_elements, &t);

	if(nb_elements % 3){
		fprintf(stderr, "ERR number of values. the stream must posses n%%3 elements\n");
		exit(-1);
	}

	pthread_mutex_lock(&mut);

	for(i=0;i<nb_threads;i++){
		qt[i].id = i;
		qt[i].predicate = predicate;
		qt[i].nb_elements = nb_elements;
		qt[i].stream = t;
	}	
	//The last threads gets the unattributed remains.
	qt[i-1].nb_elements += (nb_elements) % nb_threads;
	pthread_mutex_unlock(&mut);

	//for(i=0;i<nb_threads;i++)
	//	qt[i].nb_elements *= 3;

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
	return EXIT_SUCCESS;
}

