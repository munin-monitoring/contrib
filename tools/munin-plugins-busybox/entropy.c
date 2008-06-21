#include <string.h>
#include <stdio.h>
#include "common.h"

int entropy(int argc, char **argv) {
	FILE *f;
	int entropy;
	if(argc > 1) {
		if(!strcmp(argv[1], "config")) {
			puts("graph_title Available entropy\n"
				"graph_args --base 1000 -l 0\n"
				"graph_vlabel entropy (bytes)\n"
				"graph_scale no\n"
				"graph_category system\n"
				"graph_info This graph shows the amount of entropy available in the system.\n"
				"entropy.label entropy\n"
				"entropy.info The number of random bytes available. This is typically used by cryptographic applications.");
			return 0;
		}
		if(!strcmp(argv[1], "autoconf"))
			return writeyes();
	}
	if(!(f=fopen("/proc/sys/kernel/random/entropy_avail", "r"))) {
		fputs("cannot open /proc/sys/kernel/random/entropy_avail\n", stderr);
		return 1;
	}
	if(1 != fscanf(f, "%d", &entropy)) {
		fputs("cannot read from /proc/sys/kernel/random/entropy_avail\n", stderr);
		fclose(f);
		return 1;
	}
	fclose(f);
	printf("entropy.value %d\n", entropy);
	return 0;
}
