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

static __inline__ unsigned long long tick(void){
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

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
	unsigned char mask[16];

	memset(mask, 0x80, sizeof(mask));

	mask[0] = 0x00;
	mask[4] = 0x01;
	mask[8] = 0x02;
	mask[12] = 0x03;

	mask_register = _mm_loadu_si128((__m128i *)mask);
 
	
	mask[0] = 0x04;mask[4] = 0x05;mask[8] = 0x06;mask[12] = 0x07;
	mask_register2 = _mm_loadu_si128((__m128i *)mask); 

	mask[0] = 0x08;mask[4] = 0x09;mask[8] = 0x0A;mask[12] = 0x0B;
	mask_register3 = _mm_loadu_si128((__m128i *)mask); 

	mask[0] = 0x0C;mask[4] = 0x0D;mask[8] = 0x0E;mask[12] = 0x0F;
	mask_register4 = _mm_loadu_si128((__m128i *)mask); 
	
	__m128i upper_bound = _mm_set1_epi32(predicate);
	__m128i* input; 

	unsigned int cur_res;
        int elements_read = 0;
        unsigned int count = 0;
	int i = 0;

        long long res;
        res = tick();

	while(elements_read < numberOfElements){
		input = (__m128i*) *stream + i;

		__m128i cur_reg = _mm_load_si128 (input);

		__m128i shuffled = _mm_shuffle_epi8 (cur_reg, mask_register);
		__m128i result = _mm_cmplt_epi32(shuffled, upper_bound);
		cur_res = _mm_movemask_ps((__m128) result);
		count += _popcnt32(cur_res);	
	
		shuffled = _mm_shuffle_epi8 (cur_reg, mask_register2);
		result = _mm_cmplt_epi32(shuffled, upper_bound);
		cur_res = _mm_movemask_ps((__m128) result);
		count += _popcnt32(cur_res);	

		shuffled = _mm_shuffle_epi8 (cur_reg, mask_register3);
		result = _mm_cmplt_epi32(shuffled, upper_bound);
		cur_res = _mm_movemask_ps((__m128) result);
		count += _popcnt32(cur_res);	

		shuffled = _mm_shuffle_epi8 (cur_reg, mask_register4);
		result = _mm_cmplt_epi32(shuffled, upper_bound);
		cur_res = _mm_movemask_ps((__m128) result);
		count += _popcnt32(cur_res);	

		elements_read += 16;
		i++;
	}
        res = tick() - res;
        printf("Selected %d values in %lld cycles! \nIt took %lf cycles for each code. \n", count, res, res  / (numberOfElements * 1.0));
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
