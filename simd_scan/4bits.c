#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>
#include <x86intrin.h>    
#include <mmintrin.h>
#include <unistd.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <inttypes.h>

#include "../util/time.h"

void print128_num(__m128i var)
{
    uint16_t *val = (uint16_t*) &var;
    printf("Numerical (16 bits): %u %u %u %u %u %u %u %u\n",
           val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7]);

//    uint32_t *val_32 = (uint32_t*) &var;
  //  printf("Numerical (32 bits): %i %i %i %i\n\n",
    //       val_32[0], val_32[1], val_32[2], val_32[3]);
}

void count_query(uint64_t **stream, int numberOfElements, int predicate){
	__m128i mask_register, mask_register2, mask_register3, mask_register4; 
	__m128i mask_register5, mask_register6, mask_register7, mask_register8; 
	__m128i clean_register;

	unsigned char mask[16];
	memset(mask, 0x80, sizeof(mask));
	mask[0] = 0x00;
	mask[4] = 0x00;
	mask[8] = 0x01;
	mask[12] = 0x01;

	mask_register = _mm_loadu_si128((__m128i *)mask); 
	
	mask[0] = 0x02;mask[4] = 0x02;mask[8] = 0x03;mask[12] = 0x03;
	mask_register2 = _mm_loadu_si128((__m128i *)mask); 

	mask[0] = 0x04;mask[4] = 0x04;mask[8] = 0x05;mask[12] = 0x05;
	mask_register3 = _mm_loadu_si128((__m128i *)mask); 

	mask[0] = 0x06;mask[4] = 0x06;mask[8] = 0x07;mask[12] = 0x07;
	mask_register4 = _mm_loadu_si128((__m128i *)mask); 

	mask[0] = 0x08;mask[4] = 0x08;mask[8] = 0x09;mask[12] = 0x09;
	mask_register5 = _mm_loadu_si128((__m128i *)mask); 

	mask[0] = 0x0A;mask[4] = 0x0A;mask[8] = 0x0B;mask[12] = 0x0B;
	mask_register6 = _mm_loadu_si128((__m128i *)mask); 

	mask[0] = 0x0C;mask[4] = 0x0C;mask[8] = 0x0D;mask[12] = 0x0D;
	mask_register7 = _mm_loadu_si128((__m128i *)mask); 

	mask[0] = 0x0E;mask[4] = 0x0E;mask[8] = 0x0F;mask[12] = 0x0F;
	mask_register8 = _mm_loadu_si128((__m128i *)mask); 

	clean_register = _mm_set_epi32(240, 15, 240, 15);
	__m128i upper_bound = _mm_set_epi32(predicate << 4, predicate, predicate << 4, predicate);

	__m128i* input; 

	unsigned int volatile cur_res;
	int i = 0;
	int elements_read = 0;
	unsigned int count = 0;

	unsigned long long ts1, ts2;
	long long res;
	res = tick();
	ts1 = timestamp();
	while(elements_read < numberOfElements){
		input = (__m128i*) *stream + i;
		__m128i cur_reg = _mm_load_si128 (input);
			
		__m128i shuffled = _mm_shuffle_epi8 (cur_reg, mask_register);
		__m128i cleaned = _mm_and_si128(shuffled, clean_register);
		__m128i lt = _mm_cmplt_epi32(cleaned, upper_bound);
		cur_res = _mm_movemask_ps((__m128) lt);
		//count += _popcnt32(cur_res);

		shuffled = _mm_shuffle_epi8 (cur_reg, mask_register2);
		cleaned = _mm_and_si128(shuffled, clean_register);
		lt = _mm_cmplt_epi32(cleaned, upper_bound);
		cur_res = _mm_movemask_ps((__m128) lt);
		//count += _popcnt32(cur_res);

		shuffled = _mm_shuffle_epi8 (cur_reg, mask_register3);
		cleaned = _mm_and_si128(shuffled, clean_register);
		lt = _mm_cmplt_epi32(cleaned, upper_bound);
		cur_res = _mm_movemask_ps((__m128) lt);
		//count += _popcnt32(cur_res);

		shuffled = _mm_shuffle_epi8 (cur_reg, mask_register4);
		cleaned = _mm_and_si128(shuffled, clean_register);
		lt = _mm_cmplt_epi32(cleaned, upper_bound);
		cur_res = _mm_movemask_ps((__m128) lt);
		//count += _popcnt32(cur_res);

		shuffled = _mm_shuffle_epi8 (cur_reg, mask_register5);
		cleaned = _mm_and_si128(shuffled, clean_register);
		lt = _mm_cmplt_epi32(cleaned, upper_bound);
		cur_res = _mm_movemask_ps((__m128) lt);
		//count += _popcnt32(cur_res);

		shuffled = _mm_shuffle_epi8 (cur_reg, mask_register6);
		cleaned = _mm_and_si128(shuffled, clean_register);
		lt = _mm_cmplt_epi32(cleaned, upper_bound);
		cur_res = _mm_movemask_ps((__m128) lt);
		//count += _popcnt32(cur_res);

		shuffled = _mm_shuffle_epi8 (cur_reg, mask_register7);
		cleaned = _mm_and_si128(shuffled, clean_register);
		lt = _mm_cmplt_epi32(cleaned, upper_bound);
		cur_res = _mm_movemask_ps((__m128) lt);
		//count += _popcnt32(cur_res);

		shuffled = _mm_shuffle_epi8 (cur_reg, mask_register8);
		cleaned = _mm_and_si128(shuffled, clean_register);
		lt = _mm_cmplt_epi32(cleaned, upper_bound);
		cur_res = _mm_movemask_ps((__m128) lt);
		//count += _popcnt32(cur_res);
		
		i++;
		count++;
		elements_read += 32;
	}
	ts2 = timestamp();
        res = tick() - res;
        unsigned long long elapsed = (ts2 - ts1);
        printf("It took %lf ns for each code. \n", (elapsed / (numberOfElements * 1.0)));
        printf("Selected %d values in %lld cycles! \nIt took %lf cycles for each code. \n", count, res, res / (numberOfElements * 1.0));
}

int load_data(char *fname, int num_of_bits, int num_of_elements, uint64_t **stream){
 	FILE *file;
	unsigned int newVal = 0, prevVal = 0;
	uint64_t writtenVal = 0;
	unsigned long lineInd = 0;
	if((file = fopen(fname, "r")) == NULL)
		return(-1);

	if (fseek(file, 0, SEEK_END) != 0)
	{
		perror("fseek");
		exit(1); 
	}
   	int size_indata = ftell(file);
    	rewind(file);
	*stream = (uint64_t *) memalign(16, num_of_elements * sizeof(uint64_t));

	int modVal = 0;
	uint64_t curIndex = 0, prevIndex = 0;
	int8_t first_part, second_part;

	int bits_written = 0;
	int bits_remaining = 0; //Number of bits that roll over to next index from the last value written into the previous index

	int err;
	while(!feof(file)){
		err = fscanf(file, "%u", &newVal);
		curIndex = ((lineInd) * num_of_bits / 64); //Index of the buffer to gather 8 bit-access
		if(curIndex != prevIndex){
			(*stream)[prevIndex] = writtenVal;
			//printf("Written to index %d : %llu \n", prevIndex, (*stream)[prevIndex]);
			bits_remaining = bits_written - 64;
			bits_written = 0;

			//printf("Bits remaining: %d\n", bits_remaining);
			if(bits_remaining > 0){
				writtenVal = ((uint64_t) prevVal) >> (num_of_bits - bits_remaining);
				bits_written = bits_remaining;
				writtenVal |= ((uint64_t) newVal) << bits_written;
				bits_written += num_of_bits;
			}
			else{
				writtenVal = (uint64_t) newVal;
				bits_written += num_of_bits;
			}
		}
		else{
			writtenVal |= ((uint64_t) newVal) << bits_written;
			bits_written += num_of_bits;
		}

		//printf("%d %d %llu %llu %d \n\n", lineInd, curIndex, writtenVal, newVal, bits_written);
		prevIndex = curIndex;
		prevVal = newVal;
		lineInd++;
	}

	if(file){
		fclose(file);
	}
	return(0);
}

void print_binary(unsigned int n){
	while (n) {
    		if (n & 1)
        		printf("1");
    		else
        		printf("0");
    		n >>= 1;
	}
	printf("\n");
}

int main(int argc, char * argv[]){
        if(argc < 4){
                printf("Usage: %s <file> <num_of_bits> <num_of_elements> <predicate>\n", argv[0]);
                exit(1);
        }

	uint64_t *data_stream;
	int errno;

	errno = load_data(argv[1], atoi(argv[2]), atoi(argv[3]), &data_stream);
	count_query(&data_stream, atoi(argv[3]), atoi(argv[4]));

	free(data_stream);
}
