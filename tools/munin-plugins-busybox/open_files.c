#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"

int open_files(int argc, char **argv) {
	FILE *f;
	int alloc, freeh, avail;
	if(argc > 1) {
		if(!strcmp(argv[1], "config")) {
			if(!(f=fopen("/proc/sys/fs/file-nr", "r"))) {
				fprintf(stderr, "cannot open /proc/sys/fs/file-nr\n");
				return 1;
			}
			if(1 != fscanf(f, "%*d %*d %d", &avail)) {
				fclose(f);
				fprintf(stderr, "cannot read from /proc/sys/fs/file-nr\n");
				return 1;
			}
			fclose(f);
			puts("graph_title File table usage\n"
				"graph_args --base 1000 -l 0\n"
				"graph_vlabel number of open files\n"
				"graph_category system\n"
				"graph_info This graph monitors the Linux open files table.\n"
				"used.label open files\n"
				"used.info The number of currently open files.\n"
				"max.label max open files\n"
				"max.info The maximum supported number of open files. Tune by modifying /proc/sys/fs/file-max.");
			printf("used.warning %d\nused.critical %d\n", (int)(avail*0.92), (int)(avail*0.98));
			return 0;
		}
		if(!strcmp(argv[1], "autoconf")) {
			if(0 == access("/proc/sys/fs/file-nr", R_OK))
				return writeyes();
			else
				return writeno("/proc/sys/fs/file-nr not readable");
		}
	}
	if(!(f=fopen("/proc/sys/fs/file-nr", "r"))) {
		fputs("cannot open /proc/sys/fs/file-nr\n", stderr);
		return 1;
	}
	if(3 != fscanf(f, "%d %d %d", &alloc, &freeh, &avail)) {
		fclose(f);
		fputs("cannot read from /proc/sys/fs/file-nr\n", stderr);
		return 1;
	}
	fclose(f);
	printf("used.value %d\nmax.value %d\n", alloc-freeh, avail);
	return 0;
}
