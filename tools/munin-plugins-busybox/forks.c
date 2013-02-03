#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"

int forks(int argc, char **argv) {
	FILE *f;
	char buff[256];
	if(argc > 1) {
		if(!strcmp(argv[1], "config")) {
			puts("graph_title Fork rate\n"
				"graph_args --base 1000 -l 0 \n"
				"graph_vlabel forks / ${graph_period}\n"
				"graph_category processes\n"
				"graph_info This graph shows the forking rate (new processes started).\n"
				"forks.label forks\n"
				"forks.type DERIVE\n"
				"forks.min 0\n"
				"forks.max 100000\n"
				"forks.info The number of forks per second.");
			print_warncrit("forks");
			return 0;
		}
		if(!strcmp(argv[1], "autoconf")) {
			if(0 == access(PROC_STAT, R_OK))
				return writeyes();
			else
				return writeno(PROC_STAT " not readable");
		}
	}
	if(!(f=fopen(PROC_STAT, "r"))) {
		fputs("cannot open " PROC_STAT "\n", stderr);
		return 1;
	}
	while(fgets(buff, 256, f)) {
		if(!strncmp(buff, "processes ", 10)) {
			fclose(f);
			printf("forks.value %s", buff+10);
			return 0;
		}
	}
	fclose(f);
	fputs("no processes line found in " PROC_STAT "\n", stderr);
	return 1;
}
