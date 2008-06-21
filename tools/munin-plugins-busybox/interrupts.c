#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"

int interrupts(int argc, char **argv) {
	FILE *f;
	char buff[256];
	if(argc > 1) {
		if(!strcmp(argv[1], "config")) {
			puts("graph_title Interrupts & context switches\n"
				"graph_args --base 1000 -l 0\n"
				"graph_vlabel interrupts & ctx switches / ${graph_period}\n"
				"graph_category system\n"
				"graph_info This graph shows the number of interrupts and context switches on the system. These are typically high on a busy system.\n"
				"intr.info Interrupts are events that alter sequence of instructions executed by a processor. They can come from either hardware (exceptions, NMI, IRQ) or software.");
			puts("ctx.info A context switch occurs when a multitasking operatings system suspends the currently running process, and starts executing another.\n"
				"intr.label interrupts\n"
				"ctx.label context switches\n"
				"intr.type DERIVE\n"
				"ctx.type DERIVE\n"
				"intr.max 100000\n"
				"ctx.max 100000\n"
				"intr.min 0\n"
				"ctx.min 0");
			return 0;
		}
		if(!strcmp(argv[1], "autoconf")) {
			if(0 == access("/proc/stat", R_OK))
				return writeyes();
			else
				return writeno("/proc/stat not readable");
		}
	}
	if(!(f=fopen("/proc/stat", "r"))) {
		fputs("cannot open /proc/stat\n", stderr);
		return 1;
	}
	while(fgets(buff, 256, f)) {
		if(!strncmp(buff, "intr ", 5))
			printf("intr.value %s", buff+5);
		else if(!strncmp(buff, "ctxt ", 5))
			printf("ctx.value %s", buff+5);
	}
	fclose(f);
	return 0;
}
