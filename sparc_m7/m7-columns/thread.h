#include "/opt/dax/dax.h"
//#include "dax_query.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>

#include <sched.h>
#include <pthread.h>

#define ELEM_BITS(vec)  \
	((vec->format & DAX_BITS) ? vec->elem_width : (8 * vec->elem_width))

#define VEC_LEN(vec)    \
	((vec->elements * ELEM_BITS(vec) + vec->offset + 7) / 8)



typedef struct {
	int elements;
	int elem_width;
	int predicate_min, predicate_max;
	int nb_cols;
	uint64_t ** data;
	dax_vec_t **vecs;

	int num_of_threads;
	pthread_mutex_t *start_mutex;
	pthread_cond_t *start_cond;
	int *thread_count;

	void (*count_query)(dax_vec_t **  src, dax_vec_t *dst, dax_int_t *min, dax_int_t *max, int nb_cols);
} query_params;

void compress(query_params *args, dax_context_t *ctx){
	int i;
	dax_vec_t *aux;
	dax_zip_t *codec;
	dax_result_t res;
	size_t len;
	void *buf;

	len = VEC_LEN( (args->vecs[0]) );
	buf = memalign(8192, len);
	if(buf == NULL){
		perror("memalign compress");
		exit(-1);
	}

	for(i=0; i < args->nb_cols; i++){
		aux = args->vecs[i];
		res = dax_zip(ctx, DAX_ZIP_HIGH, aux, buf, len, &codec);
		if(res.status != DAX_SUCCESS){
			fprintf(stderr, "Compression error!, res: %d\n", res);
			exit(-1);
		}else{
			printf("Derived the codec for columns %d\n", i);
		}
		//res = dax_encode(ctx, aux, buf, len, codec);
		if(res.status != DAX_SUCCESS){
			fprintf(stderr, "Compression error!, res: %d\n", res);
			exit(-1);
		}
		free(aux->data); //Unnecessary now
		aux->data = realloc(buf, res.count); //Try to get back some space
		aux->format |= DAX_ZIP;
		aux->codec = codec;
	}


	dax_vec_t v = {0};
	v.format = DAX_BITS;
	v.elements = aux->elements;
	v.elem_width = 1;
	v.offset = 0;
	v.data = memalign(8192, DAX_OUTPUT_SIZE(v.elements, v.elem_width));
	//res = dax_extract(ctx, 0, aux, &v);
	dax_int_t bound1;
	bound1.format = DAX_BITS;
	bound1.elem_width = aux->elem_width;
	bound1.dword[2] = 2;
	res = dax_scan_value(ctx, 0, aux, &v, DAX_NE, &bound1);


	if(res.status){
		printf("Aux status: %d\n", res.status);
		exit(-1);
	}
}

void prepare_vecs(query_params *query_args){
	dax_vec_t *src;
	int i;

	query_args->vecs = malloc(sizeof(dax_vec_t *) * query_args->nb_cols);
	for(i=0;i<query_args->nb_cols;i++){
		src = memalign(8192,sizeof(dax_vec_t));
		if(src == NULL){
			perror("Memalign the src vector");
			exit(-1);
		}
		src->elements = query_args->elements;
		src->format = DAX_BITS;
		src->elem_width = query_args->elem_width;
		src->offset = 0;
		src->codec = NULL;
		src->data = query_args->data[i];

		query_args->vecs[i] = src;
	}
	src = query_args->vecs[0];


}

void *worker(void *args){
	int i;
	query_params *query_args = (query_params *) args;
	dax_int_t bound1 = {0}, bound2 = {0};
	dax_vec_t dst = {0}, *src;

	src = query_args->vecs[0];

	bound1.format = DAX_BITS;
	bound1.elem_width = src->elem_width;
	bound1.dword[2] = query_args->predicate_min;

	bound2.format = DAX_BITS;
	bound2.elem_width = src->elem_width;
	bound2.dword[2] = query_args->predicate_max;

	dst.elements = src->elements;
	dst.format = DAX_BITS;
	dst.elem_width = 1;
	dst.data = memalign(8192, DAX_OUTPUT_SIZE(dst.elements, dst.elem_width));
	dst.offset = 0;

	if(dst.data == NULL){
		perror("malloc dst");
		exit(-1);
	}

	pthread_mutex_lock(query_args->start_mutex);
	pthread_t cur_thread = pthread_self();
	int thread_count = *(query_args->thread_count);
	int cpu_id = 0;

	*(query_args->thread_count) += 1;

	pthread_cond_wait(query_args->start_cond, query_args->start_mutex);	
	pthread_mutex_unlock(query_args->start_mutex);

	for(i=0;i<1000;i++){
		query_args->count_query(query_args->vecs, &dst, &bound1, &bound2, query_args->nb_cols);
	}


	free(dst.data);
	pthread_exit(NULL);
}

void createThreads(pthread_t *threads, pthread_attr_t *attr, int num_of_threads, void* query_params){
	int i;
	for(i = 0; i < num_of_threads; i++){
		pthread_create(&threads[i], attr, worker, query_params);
	}
}

void joinThreads(pthread_t *threads, int num_of_threads){
	int i;
	for(i = 0; i < num_of_threads; i++){
		pthread_join(threads[i], NULL);
	}
}
