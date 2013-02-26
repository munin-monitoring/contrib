#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>


char VERSION[] = "1.0.0";

int verbose = 0;
int extension_stripping = 0;

char* host = "";
char* plugin_dir = "plugins";
char* spoolfetch_dir = "";

static int find_plugin_with_basename(char *cmdline, char *plugin_dir, char *plugin_basename) {
	DIR* dirp = opendir(plugin_dir);
	struct dirent* dp;
	int found = 0;

	/* Empty cmdline */
	cmdline[0] = '\0';

	while ((dp = readdir(dirp)) != NULL) {
		char* plugin_filename = dp->d_name;
		int plugin_basename_len = strlen(plugin_basename);

		if (plugin_filename[0] == '.') {
			/* No dotted plugin */
			continue;
		}

		if (strncmp(plugin_filename, plugin_basename, plugin_basename_len) != 0) {
			/* Does not start with base */
			continue;
		}

		if (plugin_filename[plugin_basename_len] != '\0' && plugin_filename[plugin_basename_len] != '.') {
			/* Does not end the string or start an extension */
			continue;
		}

		snprintf(cmdline, LINE_MAX, "%s/%s", plugin_dir, plugin_filename);
		if (access(cmdline, X_OK) == 0) {
			/* Found it */
			found ++;
			break;
		}
	}
	closedir(dirp);

	return found;
}

int main(int argc, char *argv[]) {

	int optch;
	extern int opterr;
	int optarg_len;

	char format[] = "evd:h:s:";

	char line[LINE_MAX];

	opterr = 1;

	while ((optch = getopt(argc, argv, format)) != -1)
	switch (optch) {
		case 'e':
			extension_stripping ++;
			break;
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
					if(extension_stripping) {
						/* Strip after the last . */
						char *last_dot_idx = strrchr(plugin_filename, '.');
						if (last_dot_idx != NULL) {
							*last_dot_idx = '\0';
						}
					}
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
			pid_t pid;
			if(arg == NULL) {
				printf("# no plugin given\n");
				continue;
			}
			if(arg[0] == '.' || strchr(arg, '/')) {
				printf("# invalid plugin character");
				continue;
			}
			if (! extension_stripping || find_plugin_with_basename(cmdline, plugin_dir, arg) == 0) {
				/* extension_stripping failed, using the plain method */
				snprintf(cmdline, LINE_MAX, "%s/%s", plugin_dir, arg);
			}
			if (access(cmdline, X_OK) == -1) {
				printf("# unknown plugin: %s\n", arg);
				continue;
			}
			if(0 == (pid = vfork())) {
				execl(cmdline, arg, cmd, NULL);
				/* according to vfork(2) we must use _exit */
				_exit(1);
			} else if(pid < 0) {
				printf("# fork failed\n");
				continue;
			} else {
				waitpid(pid, NULL, 0);
			}
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
