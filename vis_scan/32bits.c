#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vis.h>
#include <time.h>
#include "SIMD_buffer.c"

//Should be 8, but it's not<
#define FBUFFER_SIZE 2
#define DISPLACEMENT 32

#define error(a) do{ \
	perror(a);\
	exit(EXIT_FAILURE);\
}while(0);

static __inline__ unsigned long long tick(void)
{
	return clock();
}

/*
 *Data: 8 bits
 * 
 * Predicate = an upper bound (<)
 *-
 *
 */

void count_query(uint64_t ** stream, int number_of_buffers, int predicate)
{

	/* Compare lt, with no pointers! */

	int i, k;
	uint64_t aux = 0, result = 0, v2 = 0;
	unsigned long long constant;
	long long begin, end;

	for (i = 0; i < FBUFFER_SIZE; i++) {
		aux <<= DISPLACEMENT;
		//Just in case, a hard limit on the space taken by the predicated
		//in the mask
		aux |= (predicate & 0xFFFFFFFF);
	}

	constant = aux;

	aux = 0;

	begin = tick();

	for (i = 0; i < number_of_buffers; i++) {

		//This is the MARK
		aux =
		    __vis_fcmple32(vis_ll_to_double((*stream)[i]),
				   vis_ll_to_double(constant));

		//Should aux be an immediate value:
		while (aux > 0) {
			result += aux % 2;
			aux >>= 1;
		}
	}

	end = tick();

	printf
	    ("In the end, %d values were found to be smaller than %d in approx %d clocks!\n",
	     result, predicate, end - begin);
}

int load_data(char *fname, int num_of_bits, int num_of_elements,
	      uint64_t ** stream)
{

	return 0;
}

int main()
{

	uint64_t *tab;
	//Only FBUFFER_SIZE values will be used
	int i, vector[8] = { 32, 12 };
	int j;

	if (posix_memalign((void **)&tab, 8, sizeof(uint64_t) * 100))
		error("memalign tab");

	for (j = 0; j < 100; j++)
		for (i = 0; i < FBUFFER_SIZE; i++) {
			tab[j] <<= DISPLACEMENT;
			tab[j] |= vector[i];
		}

	count_query((uint64_t **) & tab, 100, 15);

	printf("I was expecting one positive result per query\n");

	free(tab);
	return EXIT_SUCCESS;
}
