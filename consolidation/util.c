#ifndef UTIL_CFILE
#define UTIL_CFILE
#include "util.h"

uint64_t mask_eight = 9187201950435737471;

/*
 * Assumes that the file already contains dictionnary compressed data.
 * We only work on bitcases where the padding of BitWeaving is unnecessary.
 *
 */
dax_vec_t *load_data(char *fname){
	dax_vec_t *out;
	uint64_t nb_lines = 0, i;
	FILE * f;
	uint64_t value, max_value = 0;
	uint8_t *data8;
	uint16_t *data16;

	if((f = fopen(fname, "r") )== NULL)
		error("fopen");

	out = memalign(8192, sizeof(dax_vec_t));
	if(out == NULL)
		error("memalign out");


	//Trying to find the 
	while(fscanf(f, "%lu", &value) != EOF){
		nb_lines++;
		if(value > max_value)
			max_value = value;
	}
	rewind(f);
	if(ftell < 0)
		error("ftell");

	out->offset  = 0;
	out->elements = nb_lines;
	out->format = DAX_BITS;

	if(max_value <= pow(2,7) - 1){
		printf("Eight bits\n");
		out->elem_width = 8;
		out->data = memalign(8192, DAX_OUTPUT_SIZE(sizeof(uint8_t), nb_lines));
		//out->data = malloc( DAX_OUTPUT_SIZE(sizeof(uint8_t), nb_lines));
		if(out->data == NULL)
			error("memalign 8");

		data8 = out->data;


		i = 0;
		while(fscanf(f, "%lu", &value) > 0)
			data8[i++] = value & (127);

	}else if(max_value <= pow(2,15) - 1){
		printf("Sixteen bits\n");
		out->elem_width = 16;
		out->data = memalign(8192, DAX_OUTPUT_SIZE(sizeof(uint16_t), nb_lines));
		if(out->data == NULL)
			error("memalign 16");

		data16 = out->data;


		i = 0;
		while(fscanf(f, "%lu", &value) != EOF)
			data16[i++] = value & (32767);
	}else{
		fprintf(stderr, "The value %lu is too big and is unsupported.\n", max_value);
	}


	fclose(f);
	return out;
}
uint64_t simple_scan_eq(dax_vec_t *v, predicate_t predicate){
	uint64_t count = 0, i = 0;
	uint8_t *data8;
	uint16_t *data16;


	switch(v->elem_width){
		case 8:
			data8 = v->data;
			for(i=0; i < v->elements ; i++)
				if(data8[i] == predicate.value)
					count++;
			break;
		case 16:
			data16 = v->data;
			for(i=0; i < v->elements ; i++)
				if(data16[i] == predicate.value)
					count++;
			break;
		default:
			fprintf(stderr, "This is not supposed to happend!\n");
			exit(-1);

	}

	return count;
}

void bitweaving_scan_eq(dax_vec_t *v, predicate_t predicate, uint64_t *res){
	uint64_t i, nb_rows;
	uint64_t mask, e;
	uint64_t *data = v->data;

	nb_rows = ceil(v->elements / (v->elem_width * 1.0));

	switch(v->elem_width){
		case 8: //This is _very_ experimental
			mask = mask_eight;
			break;
		default:
			fprintf(stderr, "Let's try with eight first, if you d'ont mind\n");
			exit(-1);
	}

	for(i= 0; i < nb_rows; i++){
		e = ~((predicate.bitweave ^ data[i]) + mask ) & (~mask);
		res[i] = e;
	}

}
predicate_t prepare_predicate(uint64_t value, int nb_bits){
	uint64_t res = 0;
	int i;

	switch(nb_bits){
		case 8:
			value &= 255;
			for(i=0 ; i < 8; i++)
				((uint8_t *) &res)[i] =  value;
			break;
		default:
			fprintf(stderr, "Unsupported format\n");
			break;

	} 

	predicate_t p = {value, nb_bits, res};
	return p;

}

uint64_t get_result_bitW(uint64_t*array, uint64_t nb_lines){
	int i;
	uint64_t  count = 0, aux;

	for(i=0;i<nb_lines;i++){

		aux = array[i];
		while(aux > 0){
			count += (aux % 2);
			aux >>= 1;
		}

	}


	return count;
}
#endif
