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


int predicate_global;

static __inline__ unsigned long long tick(void)
{
	unsigned hi, lo;
	__asm__ __volatile__("rdtsc":"=a"(lo), "=d"(hi));
	return ((unsigned long long)lo) | (((unsigned long long)hi) << 32);
}

void print128_num(__m128i var, int binary)
{
	uint8_t *val = (uint8_t *) & var;
	int i,j;
	char buff[20] = "";
	
	if(!binary){
	printf(" %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u \n",
	       val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7],
	       val[8], val[9], val[10], val[11], val[12], val[13], val[14],
	       val[15]
	    );
	}else{

	for(i=0;i<16;i++){
	//for(i=15;i>=0;i--){
		for(j=0;j<8;j++){
			if(val[i]&1)
				buff[j] = 49;
			else
				buff[j] = 48;
			val[i] >>= 1;
		}
		buff[8] = '\0';
		printf("%s ", buff);
	}
	printf("\n");
	}
}

int load_data_17bits(FILE * f, int *nb_values, __m128i ** st)
{
	int count = 0;

	int nb_of_elements = 0, n = 0;
	int nb_of_values = 0;
	int N = 0;
	int value;
	unsigned __int128 aux;
	__m128i *stream;

	while (!feof(f)) {
		if (fgetc(f) == '\n')
			nb_of_values++;
	}
	rewind(f);
	*nb_values = nb_of_values;

	N = nb_of_elements = ceil(nb_of_values * 17.0 / 128);

	stream = malloc(sizeof(__m128i) * nb_of_elements);

	memset(stream, 0, sizeof(__m128i) * nb_of_elements);
	//N points to the last element of stream,
	//then goes backwards to fill in the data
	N--;

	if (stream == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	n = 128;
	aux = 0;
	*st = &(stream[N]);
			printf("N = %d\n", N);
			printf("nbValues %d\n", nb_of_values);
	while (!feof(f)) {

		if(fscanf(f, "%d", &value) == EOF ){
			aux <<= n;
			memcpy(&(stream[N]), &aux, sizeof(__m128i));

			print128_num(stream[N+1], 1);
			print128_num(stream[N], 1);
			printf("N = %d\n", N);
			break;
		}
		value &= 0x1ffff;

		//Debug value, to be shown.
		if (value > predicate_global)
			count++;

		if (n > 17) {
			aux <<= 17;
			aux |= value;
			n -= 17;

		} else {
			aux <<= n;
			aux |= (value >> (17 - n));

			memcpy(&(stream[N]), &aux, sizeof(__m128i));

			//Going backwards up the array
			// (big endian)
			N--;
			aux = value;
			n = 128 - (17 - n);
		}

	}

	printf("CHEAT = %d!\n", count);

	return nb_of_elements;
}
/*
 * Stream must be padded with zeroes to 
 */
void count_query(uint8_t *stream, int numberOfElements,int nb_values, int predicate)
{
	/**** 17 bits case, 128  bits buffer *
	 *
	 *
	 * Schema = 881 - ... .... - 188 Exery 8 value
	 * ***/

	__m128i predicate_value[2];
	__m128i loaded_value, cmp_result;
	__m128i clean_registers[2];
	__m128i shuffle_register;	//Shuffle mask

	//The stack is aligned, the heap isn't. So we
	//need a buffer for the SIMD operations
	__m128i c;


	uint8_t predicate_buffer[32]; 
	__m128i *p_buffer = (__m128i*) predicate_buffer;
	unsigned long long res;
	unsigned __int128 pred_aux = 0;

	int final_result = 0, m_result, i;
	int stream_element = 0, n;
	//final_result = result number
	//m_result = result mask


	unsigned char shuffle_mask[16] = {
		0x80, 7, 8, 9,
		0x80, 9, 10, 11,
		0x80, 11, 12, 13,
		0x80, 13, 14, 15
	};



	/*
	 * Creating the prediacte in a roundabout way,
	 * May be useful to recheck the masks values.
	 */
	predicate &= 0x1FFFF;
	pred_aux = predicate;
	n = 128;

	for(i=0;i<2;i++){
		while(n>=17){
			pred_aux <<= 17;
			n -= 17;
			pred_aux |= predicate;
		}
		pred_aux <<= n;
		pred_aux |= (predicate >> (17-n));
		if(i){
			memcpy(p_buffer, &pred_aux, sizeof(uint8_t)* 16);
		}else{
			memcpy(p_buffer + 1, &pred_aux, sizeof(uint8_t) * 16);
		}
		//print128_num(c, 0);
		pred_aux = predicate; 
		n = 128 - (17-n);
	}




	clean_registers[0] = _mm_set_epi32(0xffff8000,
					   0x7fffc000, 0x3fffe000, 0x1ffff000);
	clean_registers[1] = _mm_set_epi32(0xffff800,
					   0x7fffc00, 0x3fffe00, 0x1ffff00);

	shuffle_register = _mm_loadu_si128((__m128i *) shuffle_mask);

	memcpy(&c, &(predicate_buffer[16]), sizeof(uint8_t) * 16);
	printf("Predicate buffer 1\n");
	print128_num(c, 1);
	predicate_value[0] = _mm_shuffle_epi8(c, shuffle_register);
	memcpy(&c, &(predicate_buffer[8]), sizeof(uint8_t) * 16);
	printf("Predicate buffer 2\n");
	print128_num(c, 1);
	predicate_value[1] = _mm_shuffle_epi8(c, shuffle_register);

	predicate_value[0] = _mm_and_si128(predicate_value[0], clean_registers[0]);
	predicate_value[1] = _mm_and_si128(predicate_value[1], clean_registers[1]);

	printf("Cleaned predicate codes, with buffers:\n");
	printf("Value 0\n");
	print128_num(predicate_value[0], 1);
	print128_num(clean_registers[0], 1);

	printf("Value 1\n");
	print128_num(predicate_value[1], 1);
	print128_num(clean_registers[1], 1);

	printf("\n");

	res = tick();



	
	i = 1;
	while (stream_element < nb_values) {
		i = i ? 0 : 1;

		memcpy(&c, stream, sizeof(uint8_t) * 16);

		loaded_value = _mm_shuffle_epi8(c, shuffle_register);

		memset(&cmp_result, 0, sizeof(__m128i));

		if (i) {
			loaded_value =
			    _mm_and_si128(loaded_value, clean_registers[1]);
			cmp_result =
			    _mm_cmpgt_epi32(loaded_value, predicate_value[1]);

			stream -= 9;
		} else {
			loaded_value =
			    _mm_and_si128(loaded_value, clean_registers[0]);
			cmp_result =
			    _mm_cmpgt_epi32(loaded_value, predicate_value[0]);
			stream -= 8;
		}


		//Vith 32 bits comparison, four values are read at a time
		stream_element += 4;
		m_result = _mm_movemask_ps((__m128) cmp_result);

		if(_popcnt32(m_result != 15)){
			printf("Found a result, result = %d\n", m_result);
			printf("Value:\t\t");
			print128_num(loaded_value, 1);
			printf("Clean mask\t");
			print128_num(clean_registers[i], 1);
			printf("Predicate:\t");
			print128_num(predicate_value[i], 1);
			printf("cmp_result\t");
			print128_num(cmp_result, 1);
		}
		

		final_result += _popcnt32(m_result);

	}

	res = tick() - res;
	printf
	    ("Selected %d values in %lld cycles! \nIt took %lf cycles for each code. \n",
	     final_result, res, res / (numberOfElements * 1.0));


	//Checking the alignment of the predicate values
	print128_num(predicate_value[0], 1);
	print128_num(clean_registers[0], 1);
	print128_num(predicate_value[1], 1);
	print128_num(clean_registers[1], 1);

}

int main(int argc, char *argv[])
{
	__m128i *stream;
	FILE *f = NULL;
	int elements_nb;
	int nb_values;
	predicate_global = 30;

	if (argc < 2) {
		printf("Usage: %s <filename>\n", argv[0]);
		return EXIT_FAILURE;
	}

	if ((f = fopen(argv[1], "r")) == NULL) {
		perror("fopen");
		return EXIT_FAILURE;
	}

	elements_nb = load_data_17bits(f, &nb_values, &stream);
 
	count_query((uint8_t*)stream, elements_nb, nb_values, predicate_global);

	//Just checking to see where the least and the 
	__m128i number = _mm_set_epi32(0xE, 0xE, 0xE, 4294967294);
	printf("Test number!\n");
	print128_num(number, 1);
	
	//free(stream);
	return 0;
}
