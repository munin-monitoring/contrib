#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern char **environ;

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

const char *getenv_composed(const char *name1, const char *name2) {
	char **p;
	size_t len1 = strlen(name1), len2 = strlen(name2);
	for(p = environ; *p; ++p) {
		if(0 == strncmp(*p, name1, len1) &&
				0 == strncmp(len1 + *p, name2, len2) &&
				(*p)[len1 + len2] == '=')
			return len1 + len2 + 1 + *p;
	}
	return NULL;
}

void print_warning(const char *name) {
	const char *p;
	p = getenv_composed(name, "_warning");
	if(p == NULL)
		p = getenv("warning");
	if(p == NULL)
		return;

	printf("%s.warning %s\n", name, p);
}

void print_critical(const char *name) {
	const char *p;
	p = getenv_composed(name, "_critical");
	if(p == NULL)
		p = getenv("critical");
	if(p == NULL)
		return;

	printf("%s.critial %s\n", name, p);
}

void print_warncrit(const char *name) {
	print_warning(name);
	print_critical(name);
}
