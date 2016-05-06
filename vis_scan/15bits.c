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
	double d_predicate[4], d_original[16];
	double d_value[8][2];
	double value_a, value_b;
	int i, j, aux, k;
	unsigned long long result=0;
	double zero = 0, d_buffer;
	hrtime_t h_begin, h_end;
	unsigned int bmasks[] = {
		0xff01f123,
		0xf345f567,
		0xf789f9ab,
		0xfbcdffde
	};
	double d_clean[4];

	int clean = 0X7FFF;
	predicate &= 0x7FFF;


	d_predicate[0] = vis_ll_to_double( (((unsigned long long) (predicate<<1))<<32) |  ( predicate << 2 ));
	d_predicate[1] = vis_ll_to_double( (((unsigned long long) (predicate<<3))<<32) |  ( predicate << 4 ));
	d_predicate[2] = vis_ll_to_double( (((unsigned long long) (predicate<<5))<<32) |  ( predicate << 6 ));
	d_predicate[3] = vis_ll_to_double( (((unsigned long long) (predicate<<7))<<32) |  ( predicate));


	d_clean[0] = vis_ll_to_double( (((unsigned long long) (clean<<1))<<32) |  ( clean << 2 ));
	d_clean[1] = vis_ll_to_double( (((unsigned long long) (clean<<3))<<32) |  ( clean << 4 ));
	d_clean[2] = vis_ll_to_double( (((unsigned long long) (clean<<5))<<32) |  ( clean << 6 ));
	d_clean[3] = vis_ll_to_double( (((unsigned long long) (clean<<7))<<32) |  ( clean));

	/*
	for(i=0;i<4;i++){
		d_clean[i] = vis_ll_to_double(clean_mask[i]);
	}*/



	for(i=0;i<number_of_buffers;i+=16){
		for(k=0;k<16;k++){
			original_value = stream[i+k];
			d_original[k] = vis_ll_to_double(original_value);
		}

		for(k=0;k<8;k++){

			if(k==0){
				d_value[0][0] = d_original[0];
				d_value[0][1] = d_original[1];
				continue;
			}
			if(k==7){
				d_value[7][0] = d_original[14];
				d_value[7][1] = d_original[15];
				break;
			}

			d_value[k][0] = __vis_faligndatai(d_original[k*2-1],d_original[k*2], 8-k);
			d_value[k][1] = __vis_faligndatai(d_original[k*2],d_original[k*2+1], 8-k);
		}



		for(j=0; j<4; j++){
			//Set the mask for the following Byte shuffles
			__vis_write_bmask(bmasks[j], 0);
			for(k=0;k<8;k++){
				d_buffer = __vis_bshuffle(d_value[k][0], d_value[k][1]);
				d_buffer = __vis_fand(d_buffer,d_clean[j]);

				aux = vis_fcmpgt32(d_buffer, d_predicate[j]);
				result += decode_table[aux];
			}

		}
	}

	printf("Result: %llu\n", result);

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
	load_data( argv[1], 15, &nb_elements, &t);
	count_query(t, nb_elements, predicate);


	free(t);
	return EXIT_SUCCESS;
}
