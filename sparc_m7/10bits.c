#include "dax.h"
#include "dax_query.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int load_data(char *fname, int num_of_bits, int num_of_elements, uint8_t **stream){
	FILE *file;
	unsigned int newVal = 0, prevVal = 0;
	uint8_t writtenVal = 0;
	unsigned long lineInd = 0;

	if((file = fopen(fname, "r")) == NULL)
		return(-1);
    
	*stream = (uint8_t *) memalign(8, num_of_elements * sizeof(uint8_t *));

	int modVal = 0;
	int curIndex = 0, prevIndex = 0;

	int bits_written = 0;
	int bits_remaining = 0; //Number of bits that roll over to next index from the last value written into the previous index

	int total_bits_read = 0;
	while(total_bits_read < num_of_bits * num_of_elements){
		fscanf(file, "%u", &newVal);

		//printf("Bits remaining: %d\n", bits_remaining);
		writtenVal = (uint8_t)(prevVal << (8 - bits_remaining));
		writtenVal |= (uint8_t)(newVal >> ((num_of_bits - 8) + bits_remaining));
		(*stream)[lineInd] = writtenVal;

		//printf("Written to index %d : %llu \n", lineInd, (*stream)[lineInd]);
		//printf("%d %d %d \n\n", lineInd, writtenVal, newVal, bits_remaining);

		bits_remaining += (num_of_bits - 8);
		prevVal = newVal;
		lineInd++;

		if(bits_remaining == 8){
			writtenVal = (uint8_t) (newVal);
			(*stream)[lineInd] = writtenVal;
			bits_remaining = 0;
			prevVal = 0;
			lineInd++;
		}
		total_bits_read += num_of_bits;
	}
	if(file){
		fclose(file);
	}
	return(0);
}

int main(int argc, char * argv[])
{
	if(argc < 4){
		printf("Usage: %s <file> <num_of_bits> <num_of_elements> <predicate>\n", argv[0]);
		exit(1);
	}

	uint8_t *criteria1 = (uint8_t *) memalign(8, 10 * sizeof(uint8_t));
	(criteria1)[2] = (uint8_t) (((uint16_t) atoi(argv[4])) >> 8);
	(criteria1)[3] = (uint8_t) atoi(argv[4]) ;
	(criteria1)[1] = 0; 
	(criteria1)[0] = 0; 

	uint8_t *col1;
	int errno = load_data(argv[1], atoi(argv[2]), atoi(argv[3]), &col1);

	dax_query_api_1_0_t *my_dax_api = (dax_query_api_1_0_t *) dax_query_init(1, 0);
	my_dax_api->dax_query_set_log_level(4);

	dax_enc_ctx_t *src_ctx = (dax_enc_ctx_t *) malloc(sizeof(dax_enc_ctx_t));
	src_ctx->enc_flags = 0x080;
	src_ctx->data_width = atoi(argv[2]);
	src_ctx->dict_data = NULL;
	src_ctx->dict_offsets = NULL;
	src_ctx->rle_vec = NULL;

	dax_vec_t *source = (dax_vec_t *) memalign(64, sizeof(dax_vec_t));
	source->data_stream = (uint8_t *) col1;
	source->enc_ctx = src_ctx;
	source->nrows = atoi(argv[3]); 

	//Initialize bound1
	dax_vec_t *bound1 = (dax_vec_t *) memalign(64, sizeof(dax_vec_t));
	dax_vec_t *bound2 = NULL;

	bound1->data_stream = ((uint8_t *) criteria1);

	dax_scan_op_t op1 = DAX_SCAN_LT;
	dax_scan_op_t op2 = DAX_SCAN_NOP;

	dax_vec_t *result = (dax_vec_t *) memalign(64, sizeof(dax_vec_t));
	result->nrows = source->nrows;
	result->output_bv = (uint64_t *) memalign(64, atoi(argv[3]) * 64 * sizeof(uint64_t));

	errno = my_dax_api->dax_scan(result, source, op1, bound1, op2, bound2, NULL);
	//print_binary(result->output_bv[0]);

	printf("DAX SCAN Return Code: %s\n\n", dax_strerror(errno));
	printf("Total number of selected: %d\n", result->output_bv_popcnt);

	free(col1);
	free(src_ctx);
	free(source);
	free(bound1);
	free(result);
	free(result->output_bv);
}

