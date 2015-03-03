/*
 * multicpu1sec C plugin
 */
#include <stdio.h>
#include <string.h>

#define PROC_STAT "/proc/stat"

int fail(char* msg) {
	perror(msg);

	return 1;
}

int config() {
	/* Get the number of CPU */
	FILE* f;
	if ( !(f=fopen(PROC_STAT, "r")) ) {
		return fail("cannot open " PROC_STAT);
	}

	// Starting with -1, since the first line is the "global cpu line"
	int ncpu = -1;
	while (! feof(f)) {
		char buffer[1024];
		if (fgets(buffer, 1024, f) == 0) {
			break;
		}

		if (! strncmp(buffer, "cpu", 3)) ncpu ++;
	}

	fclose(f);

	printf(
		"graph_title multicpu1sec\n"
		"graph_category system::1sec\n"
		"graph_vlabel average cpu use %\n"
		"graph_scale no\n"
		"graph_total All CPUs\n"
		"update_rate 1\n"
		"graph_data_size custom 1d, 10s for 1w, 1m for 1t, 5m for 1y\n"
	);

	int i;
	for (i = 0; i < ncpu; i++) {
		printf("cpu%d.label CPU %d\n", i, i);
		printf("cpu%d.draw %s\n", i, "AREASTACK");
		printf("cpu%d.min 0\n", i);
	}


}

int acquire() {
	printf("acquire()\n");
}

int fetch() {
	printf("fetch()\n");
}

int main(int argc, char **argv) {
	if (argc > 1) {
		char* first_arg = argv[1];
		if (! strcmp(first_arg, "config")) {
			return config();
		}

		if (! strcmp(first_arg, "acquire")) {
			return acquire();
		}
	}

	return fetch();
}
