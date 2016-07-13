#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vis.h>
/** Test of the VIS api on oracle M7
 *
 * Load an unsigned value to a register and get it back.
 *
 * Since there is no way of storing a big endian  value, 
 * the storing process ;ust be done in little endian.
 *
 * Also, all loading and storing must be done exclusively on 
 * aligned memory>
 *
 * Also, all loading and storing must be done exclusively on 8 bits
 * aligned memory.
 *
 */

int main()
{

	unsigned int *n, *m;
	int i;

	if (posix_memalign((void **)&n, 8, sizeof(int)))
		return -1;
	if (posix_memalign((void **)&m, 8, sizeof(int)))
		return -1;

	*n = 45;
	*m = 0;

	// memory -> register
	i = vis_ld_u64_nf_le(n);

	printf("Put value %d, got %d\n", *n, i);

	// register -> memory
	__vis_st_uint64_le(i, m);
	printf("Done! the new value is %d\n", *m);

	return 0;
}
