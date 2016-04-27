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
#include "immintrin.h"

static __inline__ unsigned long long tick(void)
{
	unsigned hi, lo;
	__asm__ __volatile__("rdtsc":"=a"(lo), "=d"(hi));
	return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
}

int load_data_8bits(FILE * f, __m256i ** stream)
{
	/*
	 * The strem is filled with three buffers at a time.
	 * If there are not enough values, zero padding.
	 */

	int nb_of_elements = 0, i, n = 0;
	int N = 0;
	char tab[96] = { 0 };
	char c;
	int values[768] = { 0 };

	while (!feof(f)) {
		if (fgetc(f) == '\n')
			nb_of_elements++;
	}
	rewind(f);

	nb_of_elements = ((int) ceil( nb_of_elements * 3 / 256));
	nb_of_elements += 3 - (nb_of_elements % 3);



	*stream = malloc(sizeof(__m256i) * nb_of_elements);

	if (*stream == 0) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	while (!feof(f)) {
		for (i = 0; i < 768; i++) {
			if (fscanf(f, "%d", &(values[i])) == EOF)
				break;
			values[i] &= 7;
		}
		for (; i < 768; i++)
			values[i] = 0;

		i = 0;
		while (i < 768 && n < 96) {
			tab[n++] = ((values[i] << 5)
				    | (values[i + 1] << 2)
				    | (values[i + 2] >> 1) & 0xFF);
			tab[n++] = ((values[i + 2] << 7)
				    | (values[i + 3] << 4)
				    | (values[i + 4] << 1)
				    | (values[i + 5] >> 2)) & 0xFF;
			tab[n++] = ((values[i + 5] << 6)
				    | (values[i + 6] << 3)
				    | values[i + 7]) & 0xFF;
			i += 8;
		}

		n = 0;
		for (i = 0; i < 3; i++) {
			*(stream[N++]) =
				_mm256_set_epi8(tab[n + 0], tab[n + 1], tab[n + 2],
					    tab[n + 3], tab[n + 4], tab[n + 5],
					    tab[n + 6], tab[n + 7], tab[n + 8],
					    tab[n + 9], tab[n + 10],
					    tab[n + 11], tab[n + 12],
					    tab[n + 13], tab[n + 14],
					    tab[n + 15], tab[n + 16],
					    tab[n + 17], tab[n + 18],
					    tab[n + 19], tab[n + 20],
					    tab[n + 21], tab[n + 22],
					    tab[n + 23], tab[n + 24],
					    tab[n + 25], tab[n + 26],
					    tab[n + 27], tab[n + 28],
					    tab[n + 29], tab[n + 30],
					    tab[n + 31]);
			n += 32;
		}

	}
	return nb_of_elements;
}

/*
 * Stream must be padded with zeroes to 
 */
void count_query(__m256i * stream, int numberOfElements, int predicate)
{
/**** 3 bits case, 256 bits buffer *
 *
 * There may be no bit-sized shuffle, so a predicate and a cleaning mask
 * must be generated for each value.
 *
 * ***/
	unsigned char masks[8][32];
	long long res;
	int result = 0;
	int i, j, k, l, n = 0;
	const int shuffle[8] = { 0, 3, 6, 1, 4, 7, 2, 5 };

	__mmask32 aux_result;	//unsigned int
	__m256i mask_registers[8], buffers[0];
	__m256i cleaning_mask[8], predicate_value[8];
	__m256i *first_input = stream, shifted[4];

	__m256i aux1, aux2;
	//first input out of three

	memset(masks, 0x80, sizeof(masks));

	masks[0][0] = 0;
	masks[0][4] = 3;
	masks[0][8] = 6;
	masks[0][12] = 9;
	masks[0][16] = 12;
	masks[0][20] = 15;
	masks[0][24] = 18;
	masks[0][28] = 21;
	// data & 0xE0

	masks[1][0] = 0;
	masks[1][4] = 3;
	masks[1][8] = 6;
	masks[1][12] = 9;
	masks[1][16] = 12;
	masks[1][20] = 15;
	masks[1][24] = 18;
	masks[1][28] = 21;
	//  (data << 3) & 0xE0

	masks[2][0] = 0;
	masks[2][1] = 1;
	masks[2][4] = 3;
	masks[2][5] = 4;
	masks[2][8] = 6;
	masks[2][9] = 7;
	masks[2][12] = 9;
	masks[2][13] = 10;
	masks[2][16] = 12;
	masks[2][17] = 13;
	masks[2][20] = 15;
	masks[2][21] = 16;
	masks[2][24] = 18;
	masks[2][25] = 19;
	masks[2][28] = 21;
	masks[2][29] = 22;
	// (data << 6) & 0xE0

	masks[3][0] = 1;
	masks[3][4] = 4;
	masks[3][8] = 7;
	masks[3][12] = 10;
	masks[3][16] = 13;
	masks[3][20] = 16;
	masks[3][24] = 19;
	masks[3][28] = 22;
	// data << 1 ) &

	masks[4][0] = 1;
	masks[4][4] = 4;
	masks[4][8] = 7;
	masks[4][12] = 10;
	masks[4][16] = 13;
	masks[4][20] = 16;
	masks[4][24] = 19;
	masks[4][28] = 22;
	//data << 4

	masks[5][0] = 1;
	masks[5][1] = 2;
	masks[5][4] = 4;
	masks[5][5] = 5;
	masks[5][8] = 7;
	masks[5][9] = 8;
	masks[5][12] = 10;
	masks[5][13] = 11;
	masks[5][16] = 13;
	masks[5][17] = 14;
	masks[5][20] = 16;
	masks[5][21] = 17;
	masks[5][24] = 19;
	masks[5][25] = 20;
	masks[5][28] = 22;
	masks[5][29] = 23;
	//data << 7

	masks[6][0] = 2;
	masks[6][4] = 5;
	masks[6][8] = 8;
	masks[6][12] = 11;
	masks[6][16] = 14;
	masks[6][20] = 17;
	masks[6][24] = 20;
	masks[6][28] = 23;
	// data << 2

	masks[7][0] = 2;
	masks[7][4] = 5;
	masks[7][8] = 8;
	masks[7][12] = 11;
	masks[7][16] = 14;
	masks[7][20] = 17;
	masks[7][24] = 20;
	masks[7][28] = 23;
	// data << 5

	//Masks creation
	for (i = 0; i < 8; i++) {
		mask_registers[i] = _mm256_loadu_si256((__m256i *) masks[i]);

		cleaning_mask[i] =
		    _mm256_set_epi32(7 << (29 - shuffle[i]),
				     7 << (29 - shuffle[i]),
				     7 << (29 - shuffle[i]),
				     7 << (29 - shuffle[i]),
				     7 << (29 - shuffle[i]),
				     7 << (29 - shuffle[i]),
				     7 << (29 - shuffle[i]),
				     7 << (29 - shuffle[i]));
		predicate_value[i] =
		    _mm256_set_epi32(predicate << (29 - shuffle[i]),
				     predicate << (29 - shuffle[i]),
				     predicate << (29 - shuffle[i]),
				     predicate << (29 - shuffle[i]),
				     predicate << (29 - shuffle[i]),
				     predicate << (29 - shuffle[i]),
				     predicate << (29 - shuffle[i]),
				     predicate << (29 - shuffle[i]));
	}

	res = tick();
	for (i = 0; i < (numberOfElements / 3); i++) {

		shifted[0] = *first_input;

		// (a+b) >> 64
		shifted[1] =
		    _mm256_alignr_epi32(*first_input, *(first_input + 1), 2);
		first_input++;

		// (b+c) >> 128
		shifted[2] =
		    _mm256_alignr_epi32(*first_input, *(first_input + 1), 4);
		first_input++;

		// c << 64
		shifted[3] = _mm256_slli_epi64(*first_input, 1);
		first_input++;

		for (k = 0; k < 4; k++) {
			for (j = 0; j < 8; j++) {

				buffers[j] =
				    _mm256_shuffle_epi8(shifted[k],
							mask_registers[j]);
				buffers[j] =
				    _mm256_and_si256(buffers[j],
						     cleaning_mask[j]);

				aux_result =
				    _mm256_cmplt_epi8_mask(buffers[j],
							   predicate_value[j]);

				while (aux_result > 0) {
					if (aux_result % 2)
						result++;
					aux_result >>= 1;
				}
			}
		}
	}

	res = tick() - res;
	printf
	    ("Selected %d values in %lld cycles! \nIt took %lf cycles for each code. \n",
	     aux_result, res, res / (numberOfElements * 1.0));
}

int main(int argc, char *argv[])
{
	__m256i *stream;
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
