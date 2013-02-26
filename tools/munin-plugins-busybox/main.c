#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include "common.h"

int cpu(int argc, char **argv);
int entropy(int argc, char **argv);
int forks(int argc, char **argv);
int fw_packets(int argc, char **argv);
int if_err_(int argc, char **argv);
int interrupts(int argc, char **argv);
int load(int argc, char **argv);
int open_files(int argc, char **argv);
int open_inodes(int argc, char **argv);
int processes(int argc, char **argv);
int swap(int argc, char **argv);
int threads(int argc, char **argv);
int uptime(int argc, char **argv);

int busybox(int argc, char **argv) {
	if(argc < 2)
		return fail("missing parameter");
	if(0 != strcmp(argv[1], "listplugins"))
		return fail("unknown parameter");
	puts("cpu\nentropy\nforks\nfw_packets\ninterrupts\nload\n"
		"open_files\nopen_inodes\nswap\nthreads\nuptime");
	return 0;
}

int main(int argc, char **argv) {
	char *progname;
	char *ext;
	progname = basename(argv[0]);
	ext = strrchr(progname, '.');
	if (ext != NULL) ext[0] = '\0';
	switch(*progname) {
		case 'c':
			if(!strcmp(progname, "cpu"))
				return cpu(argc, argv);
			break;
		case 'e':
			if(!strcmp(progname, "entropy"))
				return entropy(argc, argv);
			break;
		case 'f':
			if(!strcmp(progname, "forks"))
				return forks(argc, argv);
			if(!strcmp(progname, "fw_packets"))
				return fw_packets(argc, argv);
			break;
		case 'i':
			if(!strcmp(progname, "interrupts"))
				return interrupts(argc, argv);
			if(!strncmp(progname, "if_err_", 6))
				return if_err_(argc, argv);
			break;
		case 'l':
			if(!strcmp(progname, "load"))
				return load(argc, argv);
			break;
		case 'm':
			if(!strcmp(progname, "munin-plugins-busybox"))
				return busybox(argc, argv);
			break;
		case 'o':
			if(!strcmp(progname, "open_files"))
				return open_files(argc, argv);
			if(!strcmp(progname, "open_inodes"))
				return open_inodes(argc, argv);
			break;
		case 'p':
			if(!strcmp(progname, "processes"))
				return processes(argc, argv);
			break;
		case 's':
			if(!strcmp(progname, "swap"))
				return swap(argc, argv);
			break;
		case 't':
			if(!strcmp(progname, "threads"))
				return threads(argc, argv);
			break;
		case 'u':
			if(!strcmp(progname, "uptime"))
				return uptime(argc, argv);
			break;
	}
	return fail("unknown basename");
}
