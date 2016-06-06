#include <pthread.h>
#include <sched.h>

typedef struct {
	int elements;
	int elem_width;
	int predicate;
	uint64_t *data;

	int num_of_threads;
	pthread_mutex_t *start_mutex;
	pthread_cond_t *start_cond;
	int *thread_count;


	void (*count_query)(dax_vec_t*, dax_vec_t*, dax_int_t*);
} query_params;


void *worker(void *args){
	query_params *query_args = (query_params *) args;

	dax_vec_t src = {0};
	src.elements = query_args->elements;
	src.format = DAX_BITS;
	src.elem_width = query_args->elem_width;
	src.offset = 0;
	src.data = malloc(src.elements * src.elem_width);
	src.data = query_args->data;

	dax_int_t bound1 = {0};
	bound1.format = DAX_BITS;
	bound1.elem_width = src.elem_width;
	bound1.dword[2] = query_args->predicate;

	dax_vec_t dst = {0};
	dst.elements = src.elements;
	dst.format = DAX_BITS;
	dst.elem_width = 1;
	dst.data = memalign(8192, DAX_OUTPUT_SIZE(dst.elements, dst.elem_width));
	dst.offset = 0;

	pthread_mutex_lock(query_args->start_mutex);
	pthread_t cur_thread = pthread_self();
	int thread_count = *(query_args->thread_count);
	int cpu_id = 0;

	*(query_args->thread_count) += 1;

	//printf("Current thread count: %d\n", *(query_args->thread_count));
	pthread_cond_wait(query_args->start_cond, query_args->start_mutex);	
	pthread_mutex_unlock(query_args->start_mutex);

	dax_result_t res;
		query_args->count_query(&src, &dst, &bound1);
		//res = dax_scan_value(ctx, 0, &src, &dst, DAX_LT, &bound1);

	free(src.data);
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
