#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>
#include <x86intrin.h>    
#include <tmmintrin.h>
#include <unistd.h>
#include <stdint.h>
#include <malloc.h>
#include <string.h>
#include <inttypes.h>

#include "../util/time.h"

void print128_num(__m128i var)
{
    uint8_t *val_8 = (uint8_t*) &var;
    printf("Numerical (8 bits): %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u\n",
           val_8[0], val_8[1], val_8[2], val_8[3], val_8[4], val_8[5], val_8[6], val_8[7],
           val_8[8], val_8[9], val_8[10], val_8[11], val_8[12], val_8[13], val_8[14], val_8[15]);

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
	mask[0] = 0x00;mask[1] = 0x01;
	mask[4] = 0x01;mask[5] = 0x02;
	mask[8] = 0x03;mask[9] = 0x04;
	mask[12] = 0x04;mask[13] = 0x05;

	mask_register = _mm_loadu_si128((__m128i *)mask); 

	mask[0] = 0x06;mask[1] = 0x07;
	mask[4] = 0x07;mask[5] = 0x08;
	mask[8] = 0x09;mask[9] = 0x0A;
	mask[12] = 0x0A;mask[13] = 0x0B;
	mask_register2 = _mm_loadu_si128((__m128i *)mask); 

	mask[0] = 0x0C;mask[1] = 0x0D;
	mask[4] = 0x0D;mask[5] = 0x0E;
	mask[8] = 0x80;mask[9] = 0x80;
	mask[12] = 0x80;mask[13] = 0x80;
	mask_register3 = _mm_loadu_si128((__m128i *)mask); 

	//				[3]	[2]	[1]	[0]
	clean_register = _mm_set_epi32(0xFFF0, 0x0FFF, 0xFFF0, 0x0FFF);
	
	//__m128i lower_bound = _mm_set_epi32(16384, 1024, 16384, 1024);
	__m128i upper_bound = _mm_set_epi32((predicate << 4), predicate, (predicate << 4), predicate);

	__m128i* input; 
	unsigned int cur_res;

	__m128i old_reg, cur_reg, aligned_reg;
	int elements_read = 0;
	int count = 0;

	long long res;
	res = tick();

	//1st load
	input = (__m128i*) *stream;
	old_reg = _mm_load_si128 (input);

	print128_num(*input);
	print128_num(old_reg);

	__m128i shuffled = _mm_shuffle_epi8 (old_reg, mask_register);
	__m128i cleaned = _mm_and_si128(shuffled, clean_register);
	__m128i result = _mm_cmpgt_epi32(cleaned, upper_bound);
	cur_res = _mm_movemask_ps((__m128) result);
	count += _popcnt32(cur_res);

	//2nd part
	shuffled = _mm_shuffle_epi8 (old_reg, mask_register2);
	cleaned = _mm_and_si128(shuffled, clean_register);
	result = _mm_cmplt_epi32(cleaned, upper_bound);
	cur_res = _mm_movemask_ps((__m128) result);
	count += _popcnt32(cur_res);

	//3rd part
	shuffled = _mm_shuffle_epi8 (old_reg, mask_register3);
	cleaned = _mm_and_si128(shuffled, clean_register);
	result = _mm_cmplt_epi32(cleaned, upper_bound);
	cur_res = _mm_movemask_ps((__m128) result);
	count += _popcnt32(cur_res);

	elements_read += 10;
	
	int i = 1;
	int shiftAmount = 15;
	while(elements_read < numberOfElements){  
		if(shiftAmount < 0)
			shiftAmount = 15;
		if(shiftAmount != 0){
			input = (__m128i*) *stream + i;
			cur_reg = _mm_load_si128 (input);
		}

		//Compiler needs a direct value.
		if(shiftAmount == 15)
			aligned_reg = _mm_alignr_epi8(cur_reg, old_reg, 15);
		else if(shiftAmount == 14)
			aligned_reg = _mm_alignr_epi8(cur_reg, old_reg, 14);
		else if(shiftAmount == 13)
			aligned_reg = _mm_alignr_epi8(cur_reg, old_reg, 13);
		else if(shiftAmount == 12)
			aligned_reg = _mm_alignr_epi8(cur_reg, old_reg, 12);
		else if(shiftAmount == 11)
			aligned_reg = _mm_alignr_epi8(cur_reg, old_reg, 11);
		else if(shiftAmount == 10)
			aligned_reg = _mm_alignr_epi8(cur_reg, old_reg, 10);
		else if(shiftAmount == 9)
			aligned_reg = _mm_alignr_epi8(cur_reg, old_reg, 9);
		else if(shiftAmount == 8)
			aligned_reg = _mm_alignr_epi8(cur_reg, old_reg, 8);
		else if(shiftAmount == 7)
			aligned_reg = _mm_alignr_epi8(cur_reg, old_reg, 7);
		else if(shiftAmount == 6)
			aligned_reg = _mm_alignr_epi8(cur_reg, old_reg, 6);
		else if(shiftAmount == 5)
			aligned_reg = _mm_alignr_epi8(cur_reg, old_reg, 5);
		else if(shiftAmount == 4)
			aligned_reg = _mm_alignr_epi8(cur_reg, old_reg, 4);
		else if(shiftAmount == 3)
			aligned_reg = _mm_alignr_epi8(cur_reg, old_reg, 3);
		else if(shiftAmount == 2)
			aligned_reg = _mm_alignr_epi8(cur_reg, old_reg, 2);
		else if(shiftAmount == 1)
			aligned_reg = _mm_alignr_epi8(cur_reg, old_reg, 1);
		else if(shiftAmount == 0)
			aligned_reg = old_reg;


			
		__m128i shuffled = _mm_shuffle_epi8 (aligned_reg, mask_register);
		__m128i cleaned = _mm_and_si128(shuffled, clean_register);
		__m128i result = _mm_cmplt_epi32(cleaned, upper_bound);

		cur_res = _mm_movemask_ps((__m128) result);
		count += _popcnt32(cur_res);

		//cur_res |= (_mm_movemask_ps((__m128) result)) << (elements_read % 30);

		//2nd part
		shuffled = _mm_shuffle_epi8 (aligned_reg, mask_register2);
		cleaned = _mm_and_si128(shuffled, clean_register);
		result = _mm_cmplt_epi32(cleaned, upper_bound);

		cur_res = _mm_movemask_ps((__m128) result);
		count += _popcnt32(cur_res);
		//cur_res |= (_mm_movemask_ps((__m128) result)) << (elements_read % 30);

		//3rd part
		shuffled = _mm_shuffle_epi8 (aligned_reg, mask_register3);
		cleaned = _mm_and_si128(shuffled, clean_register);
		result = _mm_cmplt_epi32(cleaned, upper_bound);

		cur_res = _mm_movemask_ps((__m128) result);
                count += _popcnt32(cur_res);       
		//cur_res |= (_mm_movemask_ps((__m128) result)) << (elements_read % 30);
		elements_read += 10;

		if(shiftAmount != 0){
			old_reg = cur_reg;
			i++;
		}
		shiftAmount--;

		//Bit vector case
		/*if(elements_read % 30 == 0){
			result_vector[elements_read/30 - 1] = cur_res;
			cur_res = 0;
		}*/
	}
	res = tick() - res;
	printf("Selected %d values in %lld cycles! \nIt took %lf cycles for each code. \n", count, res, res  / (numberOfElements * 1.0));
}

int load_data(char *fname, int num_of_bits, int num_of_elements, uint64_t **stream){
 	FILE *file;
	int newVal = 0, prevVal = 0;
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
	*stream = (uint64_t *) memalign(8, num_of_elements * sizeof(uint64_t));

	int modVal = 0;
	unsigned long curIndex = 0, prevIndex = 0;
	int8_t first_part, second_part;

	int bits_written = 0;
	int bits_remaining = 0; //Number of bits that roll over to next index from the last value written into the previous index

	int err;
	while(!feof(file)){
		err = fscanf(file, "%u", &newVal);
		curIndex = ((lineInd) * num_of_bits / 64); //Index of the buffer to gather 8 bit-access
		//curIndex = bits_written / 64;
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
