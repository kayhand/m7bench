#ifndef TIME_H_
#define TIME_H_

#include <time.h>

static __inline__ unsigned long long tick(void){
	unsigned hi, lo;
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

static inline unsigned long long timestamp() {
        struct timespec ts;
//#ifdef LINUX 
        clock_gettime(CLOCK_MONOTONIC, &ts);
//#endif
        return ts.tv_sec * 1e9 + ts.tv_nsec;
}


#endif

