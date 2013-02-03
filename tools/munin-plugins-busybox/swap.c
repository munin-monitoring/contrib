#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"

int swap(int argc, char **argv) {
	FILE *f;
	char buff[256];
	int in, out;
	if(argc > 1) {
		if(!strcmp(argv[1], "config")) {
			puts("graph_title Swap in/out\n"
				"graph_args -l 0 --base 1000\n"
				"graph_vlabel pages per ${graph_period} in (-) / out (+)\n"
				"graph_category system\n"
				"swap_in.label swap\n"
				"swap_in.type DERIVE\n"
				"swap_in.max 100000\n"
				"swap_in.min 0\n"
				"swap_in.graph no\n"
				"swap_out.label swap\n"
				"swap_out.type DERIVE\n"
				"swap_out.max 100000\n"
				"swap_out.min 0\n"
				"swap_out.negative swap_in");
			print_warncrit("swap_in");
			print_warncrit("swap_out");
			return 0;
		}
		if(!strcmp(argv[1], "autoconf")) {
			if(0 == access(PROC_STAT, R_OK))
				return writeyes();
			else
				return writeno(PROC_STAT " not readable");
		}
	}
	if(!access("/proc/vmstat", F_OK)) {
		in=out=0;
		if(!(f=fopen("/proc/vmstat", "r"))) {
			fputs("cannot open /proc/vmstat\n", stderr);
			return 1;
		}
		while(fgets(buff, 256, f)) {
			if(!in && !strncmp(buff, "pswpin ", 7)) {
				++in;
				printf("swap_in.value %s", buff+7);
			}
			else if(!out && !strncmp(buff, "pswpout ", 8)) {
				++out;
				printf("swap_out.value %s", buff+8);
			}
		}
		fclose(f);
		if(!(in*out)) {
			fputs("no usable data on /proc/vmstat\n", stderr);
			return 1;
		}
		return 0;
	} else {
		if(!(f=fopen(PROC_STAT, "r"))) {
			fputs("cannot open " PROC_STAT "\n", stderr);
			return 1;
		}
		while(fgets(buff, 256, f)) {
			if(!strncmp(buff, "swap ", 5)) {
				fclose(f);
				if(2 != sscanf(buff+5, "%d %d", &in, &out)) {
					fputs("bad data on " PROC_STAT "\n",
							stderr);
					return 1;
				}
				printf("swap_in.value %d\nswap_out.value %d\n", in, out);
				return 0;
			}
		}
		fclose(f);
		fputs("no swap line found in " PROC_STAT "\n", stderr);
		return 1;
	}
}
