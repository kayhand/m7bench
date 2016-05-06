#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vis.h>
#include <sys/time.h>
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
		printf("aux = %d => %d\n", aux, decode_table[aux]);
		result += decode_table[aux];

	}
	end = tick();
	h_end = gethrtime();

printf("result = %llu\n", result);
	printf("Query completed in %lluns, %lluns per buffer, %lluticks, %lluticks per buffer\n",
			h_end - h_begin,
			(h_end - h_begin) / number_of_buffers);

}


int main(int argc, char ** argv)
{

	uint64_t *t = NULL;
	unsigned long long int nb_elements;
	int predicate;

	if(argc < 3){
		printf("Usage: %s filename predicate\n", argv[0]);
		exit(EXIT_FAILURE);

	}
	predicate = atoi(argv[2]);

	fill_decode_table(decode_table);
	load_data(argv[1], 32, &nb_elements, &t);
	count_query(t, nb_elements, predicate);


	free(t);
	return EXIT_SUCCESS;
}

