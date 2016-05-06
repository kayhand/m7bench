#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
	hrtime_t h_begin, h_end;
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


	h_begin = gethrtime();
	for(i=0;i<number_of_buffers;i+=8){

		//Iterates on eight buffers

		for(j=0;j<7;j++){
			d_originals[j][0] = __vis_faligndatai(
					__vis_ll_to_double(stream[i+j]), 
					__vis_ll_to_double(stream[i+j+1]),
					j);
			d_originals[j][1] = __vis_faligndatai(
					__vis_ll_to_double(stream[i+j+1]),
					zero, j);

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
				aux = vis_fcmpgt32(d_values, d_predicate[k]);

				result += decode_table[aux];


			}
		}

	}

	h_end = gethrtime();

	printf("Query completed in %lluns, %lluns per stream element\n",
			h_end - h_begin,
			(h_end - h_begin) / number_of_buffers);




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

int main(int argc, char ** argv)
{
	uint64_t *t = NULL;
	unsigned long long int nb_elements;
	int  predicate;


	if(argc < 3){
		printf("Usage: %s filename predicate\n", argv[0]);
		exit(EXIT_FAILURE);

	}

	fill_decode_table(decode_table);

	load_data( argv[1], 9, &nb_elements, &t);
	if(nb_elements%8)
		fprintf(stderr, "Error nb_elements\n");

	predicate = atoi(argv[2]);

	count_query(t, nb_elements, predicate);


	free(t);
	return EXIT_SUCCESS;
}

