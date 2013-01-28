#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "common.h"

#define PROC_LOADAVG "/proc/loadavg"

int load(int argc, char **argv) {
	FILE *f;
	int warn, crit;
	float val;
	char *s;
	if(argc > 1) {
		if(!strcmp(argv[1], "config")) {
			s = getenv("load_warn");
			if(s)
				warn = atoi(s);
			else
				warn = 10;
			s = getenv("load_crit");
			if(s)
				crit = atoi(s);
			else
				crit = 120;
			puts("graph_title Load average\n"
				"graph_args --base 1000 -l 0\n"
				"graph_vlabel load\n"
				"graph_scale no\n"
				"graph_category system\n"
				"load.label load");
			printf("load.warning %d\nload.critical %d\n", warn, crit);
			return 0;
		}
		if(!strcmp(argv[1], "autoconf"))
			return writeyes();
	}
	if(!(f=fopen(PROC_LOADAVG, "r"))) {
		fputs("cannot open " PROC_LOADAVG "\n", stderr);
		return 1;
	}
	if(1 != fscanf(f, "%*f %f", &val)) {
		fputs("cannot read from " PROC_LOADAVG "\n", stderr);
		fclose(f);
		return 1;
	}
	fclose(f);
	printf("load.value %.2f\n", val);
	return 0;
}
