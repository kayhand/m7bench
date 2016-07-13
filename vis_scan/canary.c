#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <vis.h>

/*
 *  If this file compiles, then VIS should work.
 */

void do_stuff(int (*f) (double, double))
{

	f(1.0, 2.0);

}

#ifdef __VIS
int main()
{
	void *p;

	printf("Vis version = %d\n", __VIS);

	// Test symbols
	p = (void *)vis_read_bmask;
	p = (void *)vis_addxc;

	p = (void *)__vis_fpcmpule16;

	printf("How long is it to be long long? It's %d !\n",
	       sizeof(unsigned long long));
	printf("But then, how long is it to be a uint64_t ? %d\n",
	       sizeof(uint64_t));

	return EXIT_SUCCESS;
}

#endif				/* __VIS */
