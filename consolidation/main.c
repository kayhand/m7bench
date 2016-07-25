#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "util.h"

#define _GNU_SOURCE


extern uint64_t mask_eight;
int main(int argc, char ** argv){
	dax_vec_t *values;
	uint64_t *res;
	predicate_t pred;
	uint64_t r;

	if(argc < 2){
		fprintf(stderr, "Nope\n");
		exit(-1);
	}


	res = malloc( ceil(1000 / 8.0) * sizeof(char));
	if(res == NULL)
		error("malloc");


	values = load_data(argv[1]);


	pred = prepare_predicate(116, 8 );


	//r = simple_scan_eq(values, pred);
	bitweaving_scan_eq(values,pred, res);
	r = get_result_bitW(res, ceil(values->elements / values->elem_width));

	printf("Found %lu values\n", r);


	//free(values->data);
	//free(values);
	printf("Hello world\n");
	fcloseall();
	return 0;
}

