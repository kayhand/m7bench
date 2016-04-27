#include <immintrin.h>
#include <inttypes.h>
#include <malloc.h>
#include <math.h>
#include <mmintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <x86intrin.h>    

static __inline__ unsigned long long tick(void)
{
	unsigned hi, lo;
	__asm__ __volatile__("rdtsc":"=a"(lo), "=d"(hi));
	return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
}

int load_data_8bits(FILE * f, __m128i ** stream)
{
	/*
	 * The cycle is 9Bytes long, 
	 * The greater cycle is 9 codes long (=> 128 values)
	 */

	int nb_of_elements = 0, n = 0;
	int N = 0;
	int value, aux;
	unsigned __int128 buff;

	while (!feof(f)) {
		if (fgetc(f) == '\n')
			nb_of_elements++;
	}
	rewind(f);

	nb_of_elements = ceil( nb_of_elements * 9 / 128);
	*stream = malloc(sizeof(__m128i) * nb_of_elements);

	if (*stream == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	n = 128;
	buff = 0;
	while(!feof(f)){

		fscanf(f, "%d", &value);
		value &= 0x1ff;
		if(n>=9){
			buff = (buff<<9) & value;
			n -= 9;
			if(n==0){
				*(stream[N++]) = _mm_set_epi64x( buff & ((1<<64) - 1) , buff>>64);
				n=128;
			}
		}else{
			aux = (value>>(9-n));
			buff = (buff<<n) & aux;
			*(stream[N++]) = _mm_set_epi64x( buff & ((1<<64) - 1) , buff>>64);
			buff = value;
			n = 128 - (9-n);
		}
	}
	if(n != 0 && n != 128){

		buff <<= n;
		*(stream[N++]) = _mm_set_epi64x( buff & ((1<<64) - 1) , buff>>64);

	}



	return N;
}

/*
 * Stream must be padded with zeroes to 
 */
void count_query(__m128i * stream, int numberOfElements, int predicate)
{
/**** 9 bits case, 128  bits buffer *
 *
 * ***/

	//9 bits
	__m128i cleaning_mask, predicate_value;
	unsigned long long res;
	unsigned char mask[16] = {0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8};
	__m128i mask_register, shifted, shuffled;
	__m128i result;
	int n_result = 0, m_result, i;
	//n_result = result number
	//m_result = result mask

	predicate = (predicate << 7) & 0xFF80;

	//Masks creation
	mask_register = _mm_loadu_si128((__m128i *) mask);

	cleaning_mask =
	    _mm_set_epi16(0xffe, (0xffe >> 1), (0xffe >> 2), (0xffe >> 3),
			  (0xffe >> 4), (0xffe >> 5), (0xffe >> 6),
			  (0xffe >> 7));
	
	predicate_value =
	    _mm_set_epi16(predicate, (predicate >> 1), (predicate >> 2), (predicate >> 3),
			  (predicate >> 4), (predicate >> 5), (predicate >> 6),
			  (predicate >> 7));
	
	shifted = stream[0];

	res = tick();

	for(i=1; i<numberOfElements; i++){

		shuffled = _mm_shuffle_epi8(shifted, mask_register);
		shuffled = _mm_and_si128(shuffled, cleaning_mask);
		result = _mm_cmplt_epi16(shuffled, predicate_value);
		m_result = _mm_movemask_epi8(result);

		while(m_result > 0){
			n_result += m_result & 1;
			m_result >>= 1;
		}

		// (old + new) >> 56
		shifted = _mm_alignr_epi8(stream[i-1], stream[i], 56);
		//TODO: check last case
	}

	res = tick() - res;
	printf
	    ("Selected %d values in %lld cycles! \nIt took %lf cycles for each code. \n",
	     n_result, res, res / (numberOfElements * 1.0));

}

int main(int argc, char *argv[])
{
	__m128i *stream;
	int nb_of_elements;
	FILE *f = NULL;

	if (argc < 3)
		return EXIT_FAILURE;

	if ((f = fopen(argv[0], "r")) == NULL) {
		perror("fopen");
		return EXIT_FAILURE;
	}

	load_data_8bits(f, &stream);
	count_query(stream, nb_of_elements, 5);

	return 0;
}
