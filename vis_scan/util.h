#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <vis.h>

struct query_thread{
	int id;
	int nb_elements;
	int predicate;
	uint64_t *stream;
};


long long tick(void);
int load_data(char *fname, int num_of_bits, unsigned long long int *num_of_elements, uint64_t **stream);
void fill_decode_table(int *decode_table);
