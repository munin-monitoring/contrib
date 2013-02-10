#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>


char VERSION[] = "1.0.0";

int verbose = 0;

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
		if(cmd == NULL)
			arg = NULL;
		else
			arg = strtok(NULL, " \t\n");

		if (!cmd || strlen(cmd) == 0) {
			printf("# empty cmd\n");
		} else if (strcmp(cmd, "version") == 0) {
			printf("munin c node version: %s\n", VERSION);
		} else if (strcmp(cmd, "nodes") == 0) {
			printf("%s\n", host);
			printf(".\n");
		} else if (strcmp(cmd, "quit") == 0) {
			return(0);
		} else if (strcmp(cmd, "list") == 0) {
			DIR* dirp = opendir(plugin_dir);
			struct dirent* dp;
			while ((dp = readdir(dirp)) != NULL) {
				char cmdline[LINE_MAX];
				char* plugin_filename = dp->d_name;;

				if (plugin_filename[0] == '.') {
					/* No dotted plugin */
					continue;
				}

				snprintf(cmdline, LINE_MAX, "%s/%s", plugin_dir, plugin_filename);
				if (access(cmdline, X_OK) == 0) {
					printf("%s ", plugin_filename);
				}
			}
			printf("\n");
			closedir(dirp);
		} else if (
				strcmp(cmd, "config") == 0 ||
				strcmp(cmd, "fetch") == 0
			) {
			char cmdline[LINE_MAX];
			sprintf(cmdline, "%s/%s", plugin_dir, arg);
			if (access(cmdline, X_OK) == -1) {
				printf("# unknown plugin: %s\n", arg);
				continue;
			}
			snprintf(cmdline, LINE_MAX, "exec %s/%s %s", plugin_dir, arg, cmd);
			system(cmdline);
			printf(".\n");
		} else if (strcmp(cmd, "cap") == 0) {
			printf("cap ");
			if (strlen(spoolfetch_dir)) {
				printf("spool ");
			}
			printf("\n");
		} else if (strcmp(cmd, "spoolfetch") == 0) {
			printf("# not implem yet cmd: %s\n", cmd);
		} else {
			printf("# unknown cmd: %s\n", cmd);
		}
	}

	return 0;
}
