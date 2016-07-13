#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <vis.h>
#include "SIMD_buffer.c"
#include "util.h"

//Should be 8, but it's not<
#define FBUFFER_SIZE 8

#define DISPLACEMENT 8

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

void count_query(uint64_t * stream, unsigned long long int number_of_buffers, int predicate)
{

	/* Compare lt, with no pointers! */


	uint64_t p64; 
	double d_predicate[4], d_originals[8][2];
	double d_values;
	int i, j, k, aux;
	unsigned long long result = 0;
	unsigned long long int pred[4];
	double zero = 0.0, d_buffer;
	//Once correctly aligned, all buffers should conform to 
	//these cleaning masks
	double d_cleaning_masks[4] = {
		__vis_ll_to_double(0x0000ff8000007fc0),
		__vis_ll_to_double(0x00003fe000001ff0),
		__vis_ll_to_double(0x00000ff8000007fc),
		__vis_ll_to_double(0x000003fe000001ff),
	};

	/*
	 * We apply this shuffle mask on a correctly shifted buffer.
	 */

	unsigned int bmasks[4] =  {
		0xff01ff12,
		0xff23ff34,
		0xff45ff56,
		0xff67ff78
	};

	predicate &= 0x1ff;
	pred[0] = (((long long int )predicate) << 39) | (predicate << 6);
	pred[1] = (((long long int )predicate) << 37) | (predicate << 4);
	pred[2] = (((long long int )predicate) << 35) | (predicate << 2);
	pred[3] = (((long long int )predicate) << 33) | (predicate) ;

	for(i=0;i<4;i++){
		d_predicate[i] = vis_ll_to_double(pred[i]);
	}


	for(i=0;i<number_of_buffers;i+=8){

		//Iterates on eight buffers

		for(j=0;j<7;j++){
			d_originals[j][0] = __vis_faligndatai(__vis_ll_to_double(stream[i+j]),__vis_ll_to_double(stream[i+j+1]),j);
			d_originals[j][1] = __vis_faligndatai(__vis_ll_to_double(stream[i+j+1]),zero, j);

			/*
			   int a, b, c, d;
			   a = __vis_fucmpeq8( __vis_ll_to_double(stream[i+j]), zero);
			   b = __vis_fucmpeq8( __vis_ll_to_double(stream[i+j+1]), zero);
			   c = __vis_fucmpeq8(d_originals[j][0], zero);
			   d = __vis_fucmpeq8(d_originals[j][1], zero);
			   printf("%llu + %llu => ", stream[i+j], stream[j+j+1]);
			   printf("%d + %d %d>> %d + %d\n", a, b, j, c, d);
			   */

		}
		for(k=0; k<4; k++){
			__vis_write_bmask(bmasks[k], 0);
			for(j=0;j<7;j++){
				d_values = __vis_bshuffle(d_originals[j][0], d_originals[j][1]);
				d_values = __vis_fand(d_values, d_cleaning_masks[k]);
				aux = vis_fcmplt32(d_values, d_predicate[k]);

				result += decode_table[aux];


			}
		}

	}




	/***********************
	 *
	 * Stream Element x 9

	 __0_____1_______2_______3_______4_______5_______6______7___
	 8 + 1 ; 7 + 2 ; 6 + 3 ; 5 + 4 ; 4 + 5 ; 3 + 6 ; 2 + 7 ; 1 
	 + 8 ; 8 + 1 ; 7 + 2 ; 6 + 3 ; 5 + 4 ; 4 + 5 ; 3 + 6 ; 2 
	 + 7 ; 1 + 8 ; 8 + 1 ; 7 + 2 ; 6 + 3 ; 5 + 4 ; 4 + 5 ; 3 
	 + 6 ; 2 + 7 ; 1 + 8 ; 8 + 1 ; 7 + 2 ; 6 + 3 ; 5 + 4 ; 4 
	 + 5 ; 3 + 6 ; 2 + 7 ; 1 + 8 ; 8 + 1 ; 7 + 2 ; 6 + 3 ; 5 
	 + 4 ; 4 + 5 ; 3 + 6 ; 2 + 7 ; 1 + 8 ; 8 + 1 ; 7 + 2 ; 6 
	 + 3 ; 5 + 4 ; 4 + 5 ; 3 + 6 ; 2 + 7 ; 1 + 8  ; 8 + 1 ; 7 
	 + 2 ; 6 + 3 ; 5 + 4 ; 4 + 5 ; 3 + 6 ; 2 + 7 ; 1 + 8 ; 8 
	 + 1 ; 7 + 2 ; 6 + 3 ; 5 + 4 ; 4 + 5 ; 3 + 6 ; 2 + 7 ; 1 + 8

	 *
	 * *********************/

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
	int per_thread;
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

	load_data( argv[1], 9, &nb_elements, &t);

	if(nb_elements%8 != 0){
		fprintf(stderr, "Error nb elements %llu\n", nb_elements);
		exit(-1);
	}

	/*
	 * Since everything must be seen as blocks of eight elements:
	 */

	per_thread = (nb_elements/8) / nb_threads;

	pthread_mutex_lock(&mut);

	for(i=0;i<nb_threads;i++){
		qt[i].id = i;
		qt[i].predicate = predicate;
		qt[i].nb_elements = nb_elements;
		qt[i].stream = t;
	}	
	//The last threads gets the unattributed remains.
	qt[i-1].nb_elements += (nb_elements) % nb_threads;

	//for(i=0;i<nb_threads;i++)
	//	qt[i].nb_elements *= 8;

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
	return EXIT_SUCCESS;
}


