#include <libgen.h>
#include <string.h>
#include <stdio.h>

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
int uptime(int argc, char **argv);

int busybox(int argc, char **argv) {
	if(argc < 2) {
		fprintf(stderr, "missing parameter\n");
		return 1;
	}
	if(0 != strcmp(argv[1], "listplugins")) {
		fprintf(stderr, "unknown parameter\n");
		return 1;
	}
	puts("cpu\nentropy\nforks\nfw_packets\ninterrupts\nload\n"
		"open_files\nopen_inodes\nprocesses\nswap\nuptime");
	return 0;
}

int main(int argc, char **argv) {
	char *progname;
	progname = basename(argv[0]);
	switch(*progname) {
		case 'c':
			if(!strcmp(progname+1, "pu"))
				return cpu(argc, argv);
			break;
		case 'e':
			if(!strcmp(progname+1, "ntropy"))
				return entropy(argc, argv);
			break;
		case 'f':
			if(!strcmp(progname+1, "orks"))
				return forks(argc, argv);
			if(!strcmp(progname+1, "w_packets"))
				return fw_packets(argc, argv);
			break;
		case 'i':
			if(!strcmp(progname+1, "nterrupts"))
				return interrupts(argc, argv);
			if(!strncmp(progname+1, "f_err_", 6))
				return if_err_(argc, argv);
			break;
		case 'l':
			if(!strcmp(progname+1, "oad"))
				return load(argc, argv);
			break;
		case 'm':
			if(!strcmp(progname+1, "unin-plugins-busybox"))
				return busybox(argc, argv);
			break;
		case 'o':
			if(!strcmp(progname+1, "pen_files"))
				return open_files(argc, argv);
			if(!strcmp(progname+1, "pen_inodes"))
				return open_inodes(argc, argv);
			break;
		case 'p':
			if(!strcmp(progname+1, "rocesses"))
				return processes(argc, argv);
			break;
		case 's':
			if(!strcmp(progname+1, "wap"))
				return swap(argc, argv);
			break;
		case 'u':
			if(!strcmp(progname+1, "ptime"))
				return uptime(argc, argv);
			break;
	}
	fprintf(stderr, "unknown basename\n");
	return 1;
}
