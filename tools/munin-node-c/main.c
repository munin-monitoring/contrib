#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>


char VERSION[] = "1.0.0";

int verbose;

char* host = "";
char* plugin_dir = "plugins";
char* spoolfetch_dir = "";

int main(int argc, char *argv[]) {

	int optch;
	extern int opterr;
	int optarg_len;

	char format[] = "vd:h:s:";

	char line[LINE_MAX];

	opterr = 1;

	while ((optch = getopt(argc, argv, format)) != -1)
	switch (optch) {
		case 'v':
			verbose ++;
			break;
		case 'd':
			optarg_len = strlen(optarg);
			plugin_dir = (char *) malloc(optarg_len + 1);
			strcpy(plugin_dir, optarg);
			break;
		case 'h':
			optarg_len = strlen(optarg);
			host = (char *) malloc(optarg_len + 1);
			strcpy(host, optarg);
			break;
		case 's':
			optarg_len = strlen(optarg);
			spoolfetch_dir = (char *) malloc(optarg_len + 1);
			strcpy(spoolfetch_dir, optarg);
			break;
	}

	/* get default hostname if not precised */
	if (! strlen(host)) {
		host = (char *) malloc(HOST_NAME_MAX + 1);
		gethostname(host, HOST_NAME_MAX);
	}

	printf("# munin node at %s\n", host);
	while (fgets(line, LINE_MAX, stdin) != NULL) {
		char* cmd;
		char* arg;

		line[LINE_MAX-1] = '\0';

		cmd = strtok(line, " \t\n");
		arg = strtok(line, " \t\n");

		if (!cmd || strlen(cmd) == 0) {
		} else if (strcmp(cmd, "version") == 0) {
			printf("munin c node version: %s\n", VERSION);
		} else if (strcmp(cmd, "nodes") == 0) {
			printf("%s\n", host);
			printf(".\n");
		} else if (strcmp(cmd, "quit") == 0) {
			return(0);
		} else if (strcmp(cmd, "list") == 0) {
		} else if (strcmp(cmd, "config") == 0) {
		} else if (strcmp(cmd, "fetch") == 0) {
		} else if (strcmp(cmd, "cap") == 0) {
			printf("cap ");
			if (strlen(spoolfetch_dir)) {
				printf("spool ");
			}
			printf("\n");
		} else if (strcmp(cmd, "spoolfetch") == 0) {
		} else {
			printf("# unknown cmd: %s\n", cmd);
		}
	}

	return 0;
}
