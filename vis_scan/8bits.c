#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vis.h>
#include <time.h>

#define error(a) do{ \
	perror(a);\
	exit(EXIT_FAILURE);\
	}while(0);

/*
 *Data: 8 bits
 *Buffer: 64 bits => values
 * An shuffle extracts 8 bytes
 * All in all, 16 values (0xF)
 * 
 * Predicate = an upper bound (<)
 *-
 * Dans la version originale, le pr?dicat est n, pour dire <n.
 * On r?cup?re donc le nombre de valeurs qui correspondent ? ce pr?dicat.
 *
 */

void count_query(uint64_t ** stream, int number_of_buffers, int predicate)
{

	/* Compare lt, with no pointers! */

	int i, aux = 0, result = 0;
	double constant, value;
	struct tms begin, end;

	for (i = 0; i < 4; i++) {
		aux <<= 8;
		aux += predicate;
	}
	constant = aux;

	times(&begin);

	for (i = 0; i < number_of_buffers; i++) {
		value = (*stream)[i] * 1.0;
		aux = vis_fucmplt8(value, constant);

		printf("aux = %d\n", aux);

		//Should aux be an immediate value:
		while (aux > 0) {
			result += aux % 2;
			aux >>= 1;
		}
	}

	times(&end);

	printf
	    ("In the end, %d values were found to be smaller than %d in approx %d clocks!\n",
	     result, predicate,
	     (end.tms_utime - begin.tms_utime) + (end.tms_stime -
						  begin.tms_stime)
	    );
}

int load_data(char *fname, int num_of_bits, int num_of_elements,
	      uint64_t ** stream)
{

	return 0;
}

int main()
{

	uint64_t *tab;
	int i;

	if (posix_memalign((void **)&tab, 8, sizeof(uint64_t) * 2))
		error("memalign tab");

	for (i = 0; i < 4; i++) {
		tab[0] <<= 8;
		tab[0] += i * 2;
		tab[1] <<= 8;
		tab[1] += i * 2;
		printf("Adding two %d\n", i * 2);
	}

	count_query((uint64_t **) & tab, 2, 3);

	free(tab);
	return EXIT_SUCCESS;
}
