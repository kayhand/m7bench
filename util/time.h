#ifndef TIME_H_
#define TIME_H_

#include <time.h>

static __inline__ unsigned long long tick(void){
#ifdef __gnu_linux__
	unsigned hi, lo;
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
#else
	return 0;
#endif
}

static inline unsigned long long timestamp() {
#ifdef __gnu_linux__
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return ts.tv_sec * 1e9 + ts.tv_nsec;
#else
	return gethrtime();
#endif
}


#endif

