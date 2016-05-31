#include <stdio.h>
#include <stdlib.h>

int main(int argc, char ** argv){
	FILE *f;
	int i;
	unsigned long long int res = 0;
	const int predicate = 5;

	f = fopen(argv[1], "r");

	while(fscanf(f, "%d", &i) != EOF){
		if(i>predicate)
			res++;

	}

	printf("%llu\n", res);
	fclose(f);
	return 0;
}
