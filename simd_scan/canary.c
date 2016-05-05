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



void point_the_finger(unsigned int first_value);
int main(){

	int i, count;
	__m128i a = _mm_set_epi32(3985178624, 3985178624, 3985178624, 3985178624);
	__m128i c = _mm_set_epi32(31, 1000, 745, 43);
	__m128i b = _mm_set_epi32(30,30,30,30);
	__m128i d = _mm_set_epi32(0,0,0,0);

	__m128i *data, res;

	posix_memalign( (void **) &data, 8, 100);

	data[0] = a;
	data[1] = b;
	data[2] = c;

	

	printf("Is a bigger than b?\n");
	res = _mm_cmpgt_epi32(a, b);
	i = _mm_movemask_ps((__m128)res);
	count = _popcnt32(i);
	printf("I= %d\tCount=%d\n\n", i, count);

	printf("Is aligned a bigger than aligned b?\n");
	res = _mm_cmpgt_epi32(data[0], data[1]);
	i = _mm_movemask_ps((__m128)res);
	count = _popcnt32(i);
	printf("I= %d\tCount=%d\n\n", i, count);


	printf("Is c bigger than b?\n");
	res = _mm_cmpgt_epi32(c, b);
	i = _mm_movemask_ps((__m128)res);
	count = _popcnt32(i);
	printf("I= %d\tCount=%d\n\n", i, count);

	printf("Is aligned c bigger than aligned b?\n");
	res = _mm_cmpgt_epi32(data[2], data[1]);
	i = _mm_movemask_ps((__m128)res);
	count = _popcnt32(i);
	printf("I= %d\tCount=%d\n\n", i, count);


	printf("Is a bigger than d?\n");
	res = _mm_cmpgt_epi32(a, d);
	i = _mm_movemask_ps((__m128)res);
	count = _popcnt32(i);
	printf("I= %d\tCount=%d\n\n", i, count);

	printf("Is c bigger then d?\n");
	res = _mm_cmpgt_epi32(c, d);
	i = _mm_movemask_ps((__m128)res);
	count = _popcnt32(i);
	printf("I= %d\tCount=%d\n\n", i, count);

	
	/*
	 * AVX 2 test
	 */
	/* _mm_cmpgt_epi32_mask(a,b);*/

	free(data);
	point_the_finger( 3985178624);
	int e = 2147483647;
	printf("%d\n", e);
	e++;
	printf("%d\n", e);

	return 0;
}

void point_the_finger(unsigned int first_value){
	int count = 0;
	__m128i variable, result;
	__m128i zero = _mm_set_epi32(0,0,0,0);

	while(!count && first_value > 0){
		first_value--;
		variable = _mm_set_epi32(first_value, first_value, first_value, first_value);
		result = _mm_cmpgt_epi32(variable, zero);
		count = _popcnt32(_mm_movemask_ps((__m128)result));
	}

	printf("First value = %u\n", first_value);


}
