#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include "common.h"

/* TODO: The upstream plugin does way more nowawdays. */

int processes(int argc, char **argv) {
	DIR *d;
	struct dirent *e;
	char *s;
	int n=0;
	if(argc > 1) {
		if(!strcmp(argv[1], "config")) {
			puts("graph_title Number of Processes\n"
				"graph_args --base 1000 -l 0 \n"
				"graph_vlabel number of processes\n"
				"graph_category processes\n"
				"graph_info This graph shows the number of processes in the system.\n"
				"processes.label processes\n"
				"processes.draw LINE2\n"
				"processes.info The current number of processes.");
			return 0;
		}
		if(!strcmp(argv[1], "autoconf"))
			return writeyes();
	}
	if(!(d = opendir("/proc")))
		return fail("cannot open /proc");
	while((e = readdir(d))) {
		for(s=e->d_name;*s;++s)
			if(!isdigit(*s))
				break;
		if(!*s)
			++n;
	}
	closedir(d);
	printf("processes.value %d\n", n);
	return 0;
}
