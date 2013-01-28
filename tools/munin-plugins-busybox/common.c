#include <stdio.h>
#include <stdlib.h>

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

int getenvint(const char *name, int defvalue) {
	const char *value;
	value = getenv(name);
	if(value == NULL)
		return defvalue;
	return atoi(value);
}
