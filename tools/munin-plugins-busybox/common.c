#include <stdio.h>

int writeyes(void) {
	puts("yes");
	return 0;
}

int writeno(const char *s) {
	if(s)
		printf("no (%s)\n", s);
	else
		puts("no");
	return 1;
}
