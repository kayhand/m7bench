#include "util.h"

int load_data(char *fname, int num_of_bits, unsigned long long *num_of_elements, uint64_t **stream){
	FILE *f;
	unsigned int nb_of_values = 0, value;
	int n, N=0;
	uint64_t writtenVal = 0, aux;
	uint64_t vol;
	unsigned long lineInd = 0;

	if((f = fopen(fname, "r")) == NULL)
		return(-1);

		if(!f){
		perror("fopen: ");
		exit(-1);
		}

	while (!feof(f)) {
		if (fgetc(f) == '\n')
			nb_of_values++;
	}
	rewind(f);
	*num_of_elements = ceil(nb_of_values / (64.0 / num_of_bits));
	

	/*
	 * Special cases
	 */
	switch(num_of_bits){
		case 12:
			if((*num_of_elements)%3)
				*num_of_elements += (3 - (*num_of_elements% 3));
			break;
		case 9:
			if(*num_of_elements%8)
				*num_of_elements += (8 - (*num_of_elements% 8));
			break;
		case 15:
			if(*num_of_elements%16)
				*num_of_elements += (16 - (*num_of_elements% 16));
			break;
		default:
			break;

	}

	n = 64;

	*stream = (uint64_t*) malloc(*num_of_elements * sizeof(uint64_t));
	if(! *stream){
		fprintf(stderr, "num elements: %llu\n", *num_of_elements);

		perror("malloc: ");
		exit(-1);
	}
	memset(*stream, 0, sizeof((*num_of_elements) * sizeof(uint64_t)));

	while(!feof(f)){
		if(fscanf(f, "%d", &value) == EOF ){
			aux <<= n;
			if(n=64)
				n=0;
			memcpy( *stream + N, &aux, sizeof(uint64_t));
			break;
		}


		if(n>num_of_bits){
			aux <<= num_of_bits;
			if(n==64){
			//printf("Should be zero %llu\n", aux);
				aux = 0;
			}
			aux |= value;
			n -= num_of_bits;
		}else{
			aux <<= n;
			aux |= (value) >> (num_of_bits - n);
			//printf("%llu ; %llu \n", aux >> 32, aux & 0xffffffff);
			memcpy( *stream + N, &aux, sizeof(uint64_t));
			aux = value;
			N++;
			n = 64 - (num_of_bits - n);
		}

	}

	fclose(f);

	return 0;
}

void fill_decode_table(int *decode_table){
	int i, aux, cpt;
	for(i=0;i<0xff;i++){
		aux = i;
		cpt = 0;

		while(aux>0){
			cpt += aux & 1;
			aux >>= 1;
		}

		decode_table[i] = cpt;

	}
}
