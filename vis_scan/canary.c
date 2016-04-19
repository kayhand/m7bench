#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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

	return EXIT_SUCCESS;
}

#endif				/* __VIS */
