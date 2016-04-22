#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vis.h>
#include "SIMD_buffer.c"

#define FBUFFER_SIZE 16
#define DISPLACEMENT 4

#define error(a) do{ \
	perror(a);\
	exit(EXIT_FAILURE);\
}while(0);

static __inline__ unsigned long long tick(void)
{
	/*
	   unsigned hi, lo;
	   __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	   return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
	 */
	return clock();
}

/*
 * Since the smallest data size a VIS operation can work with is a byte,
 * 4 bits compression must be decoded to 8bits compression before any
 * operation. A simple shift and mask of the unecessary data does it all.
 */

void decode_4_to_8(unsigned long long *src, unsigned long long *dst)
{
	unsigned long long mask = 0x0F0F0F0F0F0F0F0F;

	*dst = *src >> 4;
	*dst &= mask;
	*src &= mask;
}

void count_query(uint64_t ** stream, int number_of_buffers, int predicate)
{

	/* Compare lt, with no pointers! */

	int i, k;
	uint64_t aux = 0, result = 0, v2 = 0;
	unsigned long long constant, buff[2];
	long long begin, end;

	for (i = 0; i < 8; i++) {
		aux <<= 8;
		//Just in case, a hard limit on the space taken by the predicated
		//in the mask
		aux |= (predicate & 0x0F);
	}

	constant = aux;

	print_buffer(aux);

	aux = 0;

	begin = tick();

	for (i = 0; i < number_of_buffers; i++) {
		buff[0] = (*stream)[i];

		decode_4_to_8(&(buff[0]), &buff[1]);

		for (k = 0; k < 2; k++) {

			//This is the MARK
			aux = vis_fucmplt8(vis_ll_to_double(buff[k]),
					   vis_ll_to_double(constant));

			//Should aux be an immediate value:
			while (aux > 0) {
				result += aux % 2;
				aux >>= 1;
			}
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
	int i, j=1;
	int vector[16] =
	    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

	if (posix_memalign((void **)&tab, 8, sizeof(uint64_t) * 100))
		error("memalign tab");

	for (j = 0; j < 100; j++)
		for (i = 0; i < FBUFFER_SIZE; i++) {
			tab[j] <<= DISPLACEMENT;
			tab[j] |= ( vector[i] & 0x0F);
		}

	count_query((uint64_t **) & tab, 100, 5);
	

	unsigned long long a, b;
	a = tab[0];
	printf("Before decoding: \n");
	print_buffer(a);
	decode_4_to_8(&a, &b);
	printf("-\n", tab[0]);
	print_buffer(a);
	print_buffer(b);



	printf("Expecting: five values per vector.\n");

	free(tab);
	return EXIT_SUCCESS;
}
