#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vis.h>
#include "SIMD_buffer.c"
#include "util.h"

#define FBUFFER_SIZE 16
#define DISPLACEMENT 4

#define error(a) do{ \
	perror(a);\
	exit(EXIT_FAILURE);\
}while(0);


int decode_table[256];


/*
 * Since the smallest data size a VIS operation can work with is a byte,
 * 4 bits compression must be decoded to 8bits compression before any
 * operation. A simple shift and mask of the unecessary data does it all.
 */

void decode_4_to_8(uint64_t *src, uint64_t *dst)
{
	uint64_t mask = 0x0F0F0F0F0F0F0F0F;

	*dst = *src >> 4;
	*dst &= mask;
	*src &= mask;
}

void count_query(uint64_t * stream, int number_of_buffers, int predicate)
{

	/* Compare lt, with no pointers! */

	double values[2], d_original[2];
	double zero = 0, d_predicate;
	uint64_t original_values[2], p64;
	int i, j, aux;
	hrtime_t h_begin, h_end;
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



	/* *
	 * Stream Element:
	 * AAAABBBB CCCCDDDD EEEEFFFF GGGGHHHH IIIIJJJJ KKKKLLLL MMMMNNNN OOOOPPPP
	 *
	 * After decode_4_to_8
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

	h_begin = gethrtime();
	for(i=0;i<number_of_buffers;i++){
		original_values[0] = stream[i];
		decode_4_to_8(original_values, original_values + 1);
		d_original[0] = vis_ll_to_double(original_values[0]);
		d_original[1] = vis_ll_to_double(original_values[1]);

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

	load_data(argv[1], 4, &nb_elements, &t);
	count_query(t, nb_elements, predicate);


	free(t);
	return EXIT_SUCCESS;
}
