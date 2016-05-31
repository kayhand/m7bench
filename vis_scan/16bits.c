#include <inttypes.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vis.h>
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
/*
 *Data: 8 bits
 * 
 * Predicate = an upper bound (<)
 *-
 *
 */

void count_query(uint64_t * stream, unsigned long int number_of_buffers, int predicate)
{


	uint64_t p64, original_value;
	double d_predicate, d_original;
	int i, j, aux;
	unsigned long long result=0;
	double zero = 0, d_buffer;
	hrtime_t h_begin, h_end;
	unsigned int bmasks[2] = {
		0xff01ff23,
		0xff45ff67
	};

	/***************
	 * AAAAAAAA AAAAAAAA BBBBBBBB BBBBBBBB CCCCCCCC CCCCCCCC DDDDDDDD DDDDDDDD
	 * 0		1	2	3	4	 5	  6		7
	 *
	 */
	p64 = predicate & 0xffff;
	p64 <<= 32;
	p64 |= predicate;
	d_predicate = vis_ll_to_double(p64);

	h_begin = gethrtime();
	for(i=0;i<number_of_buffers;i++){
		original_value = stream[i];
		d_original = vis_ll_to_double(original_value);

		for(j=0; j<2; j++){
			//Set the mask for the following Byte shuffles
			__vis_write_bmask(bmasks[j], 0);
			d_buffer = __vis_bshuffle(d_original, zero);

			aux = vis_fcmpgt32(d_buffer, d_predicate);
			result += decode_table[aux];

		}
	}
	h_end = gethrtime();

	printf("Query completed in %lluns, %lluns per stream element\n",
			h_end - h_begin,
			(h_end - h_begin) / number_of_buffers);

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
	predicate = atoi(argv[2]);

	fill_decode_table(decode_table);
	load_data( argv[1], 16, &nb_elements, &t);
	count_query(t, nb_elements, predicate);


	free(t);
	return EXIT_SUCCESS;
}
