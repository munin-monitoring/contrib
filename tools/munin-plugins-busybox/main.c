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
			if(!strcmp(progname+1, "cpu"+1))
				return cpu(argc, argv);
			break;
		case 'e':
			if(!strcmp(progname+1, "entropy"+1))
				return entropy(argc, argv);
			break;
		case 'f':
			if(!strcmp(progname+1, "forks"+1))
				return forks(argc, argv);
			if(!strcmp(progname+1, "fw_packets"+1))
				return fw_packets(argc, argv);
			break;
		case 'i':
			if(!strcmp(progname+1, "interrupts"+1))
				return interrupts(argc, argv);
			if(!strncmp(progname+1, "if_err_"+1, 6))
				return if_err_(argc, argv);
			break;
		case 'l':
			if(!strcmp(progname+1, "load"+1))
				return load(argc, argv);
			break;
		case 'm':
			if(!strcmp(progname+1, "munin-plugins-busybox"+1))
				return busybox(argc, argv);
			break;
		case 'o':
			if(!strcmp(progname+1, "open_files"+1))
				return open_files(argc, argv);
			if(!strcmp(progname+1, "open_inodes"+1))
				return open_inodes(argc, argv);
			break;
		case 'p':
			if(!strcmp(progname+1, "processes"+1))
				return processes(argc, argv);
			break;
		case 's':
			if(!strcmp(progname+1, "swap"+1))
				return swap(argc, argv);
			break;
		case 'u':
			if(!strcmp(progname+1, "uptime"+1))
				return uptime(argc, argv);
			break;
	}
	fprintf(stderr, "unknown basename\n");
	return 1;
}
