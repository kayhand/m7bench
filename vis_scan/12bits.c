#include <inttypes.h>
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

void count_query(uint64_t * stream, int number_of_buffers, int predicate)
{


	uint64_t p64, original_value;
	double d_predicate, d_original;
	double d_values[3];
	double d_clean;
	int i, j, aux; 
	unsigned long long result=0;
	hrtime_t h_begin, h_end;
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

	h_begin = gethrtime();
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
	unsigned long long nb_elements;
	int  predicate;

	if(argc < 3){
		printf("Usage: %s filename predicate\n", argv[0]);
		exit(EXIT_FAILURE);

	}

	predicate =  atoi(argv[2]);

	fill_decode_table(decode_table);

	load_data( argv[1], 12, &nb_elements, &t);

	if(nb_elements % 3){
		fprintf(stderr, "ERR number of values. the stream must posses n%%3 elements\n");
		exit(-1);
	}

	count_query(t, nb_elements, predicate);


	free(t);
	return EXIT_SUCCESS;
}
