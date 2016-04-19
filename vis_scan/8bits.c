#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vis.h>

#define error(a) do{ \
	perror(a);\
	exit(EXIT_FAILURE);\
	}while(0);

/*
 *Data: 8 bits
 *Buffer: 64 bits => values
 * An shuffle extracts 8 bytes
 * All in all, 16 values (0xF)
 * 
 * Predicate = an upper bound (<)
 *
 * Dans la version originale, le prédicat est n, pour dire <n.
 * On récupère donc le nombre de valeurs qui correspondent à ce prédicat.
 *
 */

//Syntastic
extern double vis_ld_d64(void *);
extern int vis_fucmplt8(double, double);

/**
 * The stream data is already formatted. (In theory)
 */
void count_query(uint64_t **stream, int number_of_elements, int predicate){

	/* Compare lt, with no pointers! */

	int i, constant, aux;
	int result = 0;
	for(i=0; i<4; i++){
		constant  <<=  8;
		constant += predicate;
	}


	for(i=0; i < number_of_elements; i++){
		aux = vis_fucmplt8(*stream[i] * 1.0, constant);

		printf("aux = %d\n", aux);

		//Should aux be an immediate value:
		while(aux > 0){
			result += aux % 2;
			aux >>= 1;
		}
	}

	printf("In the end, %d values were found!\n");



	/*
	int i, aux = 0;
	double *Constant, constant_register, var_register;
	double *Variable;
	
	if(posix_memalign((void *)&Constant, 8, sizeof(double)))
		error("memalign");
	if(posix_memalign((void *)&Variable, 8, sizeof(double)))
		error("memalign");

	for(i=0; i<4; i++){
		aux  <<=  8;
		aux += predicate;
	}

	*Constant = aux * 1.0;

	constant_register = vis_ld_d64(Constant);

	for(i=0; i<number_of_elements; i++){

		*Variable = *stream[i] * 1.0;
		constant_register = vis_ld_d64(Variable);



	}
	*/






}

int load_data(char *fname, int num_of_bits, int num_of_elements, uint64_t **stream){

	return 0;
}

int main(){

	uint64_t tab[2] = {0};
	int i;


	for(i=0;i<4;i++){
		tab[0] <<= 8;
		tab[0] += i*2;
		tab[1] <<= 8;
		tab[1] += i*2;
	}

	count_query((uint64_t **)&tab, 2, 3);


	return EXIT_SUCCESS;
}
