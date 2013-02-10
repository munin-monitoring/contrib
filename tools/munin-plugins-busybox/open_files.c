#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"

#define FS_FILE_NR "/proc/sys/fs/file-nr"

/* TODO: support env.warning and friends after the upstream plugin is fixed */

int open_files(int argc, char **argv) {
	FILE *f;
	int alloc, freeh, avail;
	if(argc > 1) {
		if(!strcmp(argv[1], "config")) {
			if(!(f=fopen(FS_FILE_NR, "r")))
				return fail("cannot open " FS_FILE_NR);
			if(1 != fscanf(f, "%*d %*d %d", &avail)) {
				fclose(f);
				return fail("cannot read from " FS_FILE_NR);
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
				"max.info The maximum supported number of open "
					"files. Tune by modifying " FS_FILE_NR
					".");
			printf("used.warning %d\nused.critical %d\n",
					(int)(avail*0.92), (int)(avail*0.98));
			return 0;
		}
		if(!strcmp(argv[1], "autoconf"))
			return autoconf_check_readable(FS_FILE_NR);
	}
	if(!(f=fopen(FS_FILE_NR, "r")))
		return fail("cannot open " FS_FILE_NR);
	if(3 != fscanf(f, "%d %d %d", &alloc, &freeh, &avail)) {
		fclose(f);
		return fail("cannot read from " FS_FILE_NR);
	}
	fclose(f);
	printf("used.value %d\nmax.value %d\n", alloc-freeh, avail);
	return 0;
}
