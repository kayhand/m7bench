#ifndef UTIL_DEF
#define UTIL_DEF

#include <stdio.h>
#include <malloc.h>
#include <math.h>
#include <stdlib.h>
#include <inttypes.h>

#define error(a) do{ \
	perror(a);\
	exit(-1); \
}while(0) 

/**
 * This will be deleted and replaced later with the original DAX values.
 * Meanwhile it's here for development purposes only
 */


#define DAX_OUTPUT_SIZE(elements, elem_width_bits)      \
      ((((elements) * (elem_width_bits) + 511) / 512 * 512 + 512) / 8)
#define DAX_BITS 1


typedef struct {
	uint64_t format, elements;
	void *data;
	uint32_t elem_width;
	uint8_t offset;
}dax_vec_t;

typedef struct {
	uint64_t value;
	int nb_bits;
	uint64_t bitweave;
	//dax_int_t
} predicate_t;

dax_vec_t *load_data(char *fname);
/*
 * Creates a predicate valid for all
 */
predicate_t prepare_predicate(uint64_t value, int nb_bits);
uint64_t simple_scan_eq(dax_vec_t *v, predicate_t predicate);
void bitweaving_scan_eq(dax_vec_t *v, predicate_t predicate, uint64_t *res);
uint64_t dax_scan(void* ctx, dax_vec_t *v, predicate_t predicate);

uint64_t get_result_bitW(uint64_t *array, uint64_t nb_lines);
#endif
