#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <vis.h>

/*
 *  If this file compiles, then VIS should work.
 */

#ifdef __VIS
int main()
{
	void *p;

	printf("Vis version = %d\n", __VIS);

	p = (void *)vis_read_bmask;
	p = (void *)vis_addxc;

	printf("How long is it to be long long? It's %d !\n",
	       sizeof(unsigned long long));
	printf("But then, how long is it to be a uint64_t ? %d\n",
	       sizeof(uint64_t));

	return EXIT_SUCCESS;
}

#endif	/* __VIS */
