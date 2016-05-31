#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vis.h>
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


	uint64_t p64, original_value;
	double d_predicate, d_original;
	int i, j, aux;
	unsigned long long result = 0;
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
	h_begin = gethrtime();
	for(i=0;i<number_of_buffers;i++){
		original_value = stream[i];
		d_original = vis_ll_to_double(original_value);

		/*
		printf("%llu\n", stream[i]);
		for(j=0;j<8;j++){
			printf("%llu\t", (stream[i] >> (8 * j)) & 0xff);
		}*/

		for(j=0; j<4; j++){
			//Set the mask for the following Byte shuffles
			__vis_write_bmask(bmasks[j], 0);
			d_buffer = __vis_bshuffle(d_original, zero);
			aux = vis_fcmpgt32(d_buffer, d_predicate);
			result += decode_table[aux];
		}
	}
	h_end = gethrtime();
	//printf("result: %llu\n", result);

	printf("Query completed in %lluns, %lluns per stream element\n",
			h_end - h_begin,
			(h_end - h_begin) / number_of_buffers);
}

int main(int argc, char ** argv)
{
	uint64_t *t = NULL;
	unsigned long long nb_elements;
	int  predicate;

	if(argc < 3){
		printf("Usage: %s filename predicate\n", argv[0]);
		exit(EXIT_FAILURE);

	}
	predicate = atoi(argv[2]);

	fill_decode_table(decode_table);

	load_data( argv[1], 8, &nb_elements, &t);
	count_query(t, nb_elements, predicate);


	free(t);
	return EXIT_SUCCESS;
}
