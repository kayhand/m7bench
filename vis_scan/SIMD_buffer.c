/*
 * TODO: Do this properly,
 * buffer.o & include of .h
 */

#define SIZE_BITS 64
#define BUFF_SIZE 100

void print_buffer(uint64_t value)
{
	int i;
	char buff[BUFF_SIZE] = "", aux[BUFF_SIZE] = "";

	printf(" ");

	for (i = 0; i < SIZE_BITS; i++) {
		if (value % 2) {
			sprintf(aux, "1%s", buff);
		} else {
			sprintf(aux, "0%s", buff);
		}
		strncpy(buff, aux, BUFF_SIZE);

		value = value >> 1;
	}
	printf("%s\n", buff);
}
