#include <stdio.h>
#include <string.h>
#include "common.h"

int uptime(int argc, char **argv) {
	FILE *f;
	float uptime;
	if(argc > 1) {
		if(!strcmp(argv[1], "config")) {
			puts("graph_title Uptime\n"
				"graph_args --base 1000 -l 0 \n"
				"graph_vlabel uptime in days\n"
				"uptime.label uptime\n"
				"uptime.draw AREA");
			return 0;
		}
		if(!strcmp(argv[1], "autoconf"))
			return writeyes();
	}
	if(!(f=fopen("/proc/uptime", "r"))) {
		fputs("cannot open /proc/uptime\n", stderr);
		return 1;
	}
	if(1 != fscanf(f, "%f", &uptime)) {
		fputs("cannot read from /proc/uptime\n", stderr);
		fclose(f);
		return 1;
	}
	fclose(f);
	printf("uptime.value %.2f\n", uptime/86400);
	return 0;
}
