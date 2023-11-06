/*
 * Copyright (C) 2017 Bastiaan van Kesteren <bas@edeation.nl> - All rights reserved.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License v.2 or v.3.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <sys/wait.h>
#include "common.h"

static char getitem(char *input, unsigned char item, char *output)
{
	unsigned char i = 0;
	unsigned char separators = 0;
	char know_this_seperator = 0;
	unsigned char start = 0;
	unsigned char stop = 0;

	/* Trim starting spaces */
	while (input[i] == ' ') {
		i++;
	}

	/* If we're requested to return the very first item... */
	if (separators == item) {
		start = i;
	}

	while (input[i] && separators < item + 1) {
		if (input[i] == ' ') {
			if (know_this_seperator == 0) {
				know_this_seperator = 1;
				separators++;
				if (separators == item + 1) {
					stop = i;
					break;
				}
			}
		} else if (know_this_seperator) {
			know_this_seperator = 0;
			if (separators == item) {
				start = i;
			}
		} else if (input[i] == '\n') {
			input[i] = 0;
			break;
		}

		i++;
	}

	if (stop) {
		/* Found stop, means we have a start as well */
		strncpy(output, &input[start], stop - start);
		return 1;
	} else if (start) {
		/* Have a start, no stop. We're returning the last item of the string */
		strcpy(output, &input[start]);
		return 1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	char command[50];
	char output[255];
	char label[25][100];
	char value[25][25];
	unsigned char attribute = 0;
	FILE *f;
	unsigned int i;

	/* Prepare and execute command */
	snprintf(command, sizeof(command),
		 "smartctl -A -d ata /dev/%s -n standby",
		 &basename(argv[0])[6]);
	if ((f = popen(command, "r")) == 0) {
		return fail("cannot initiate command execution");
	}

	/* Process command output */
	while (fgets(output, sizeof(output), f) != NULL) {
		printf("#%s", output);
		/* Filter out attribute lines; look for lines starting with an attribute ID */
		if ((output[0] >= '0' && output[0] <= '9') ||
		    (output[0] == ' '
		     && (output[1] >= '0' && output[1] <= '9'))
		    || (output[0] == ' ' && output[1] == ' '
			&& (output[2] >= '0' && output[2] <= '9'))) {
			/* Now, print the 2nd column (attribute name) and the 10th (raw value) */

			getitem(output, 1, label[attribute]);
			getitem(output, 9, value[attribute]);
			attribute++;
			if (attribute == 25) {
				break;
			}
		}
	}

	/* Close command (this is where we get the exit code! */
	{
		int status = pclose(f); /* using an explicit temp var, to be compatible with macos & openbsd */
		i = WEXITSTATUS(status);
	}
	if (i == 1 ||		/* smartctl command did not parse */
	    /*i == 2 || *//* smartctl device open failed */
	    i == 127) {		/* command not found */
		return fail("command execution failed");
	}

	/* Setup for caching */
	snprintf(command, sizeof(command), "/mnt/ram/smart_%s",
		 &basename(argv[0])[6]);

	if (attribute == 0) {
		printf("#Cached attributes\n");
		/* No output from command, try to fetch attribute-list from disk with NaN values */
		if ((f = fopen(command, "r")) == 0) {
			return
			    fail
			    ("command did not return data, no cached attribute-list");
			return 0;
		}
	}

	if (argc > 1) {
		if (strcmp(argv[1], "config") == 0) {
			printf
			    ("graph_title S.M.A.R.T values for drive %s\n"
			     "graph_args --base 1000 --lower-limit 0\n"
			     "graph_vlabel Attribute S.M.A.R.T value\n"
			     "graph_category disk\n",
			     &basename(argv[0])[6]);

			if (attribute == 0) {
				while (fgets(output, sizeof(output), f) !=
				       NULL) {
					for (i = 0; i < strlen(output);
					     i++) {
						if (output[i] == '\n') {
							output[i] = '\0';
							break;
						}
					}
					printf("%s.label %s\n", output,
					       output);
				}
				fclose(f);
			} else {
				f = fopen(command, "w");
				do {
					attribute--;
					printf("%s.label %s\n",
					       label[attribute],
					       label[attribute]);
					if (f) {
						fprintf(f, "%s\n",
							label[attribute]);
					}
				} while (attribute);

				if (f) {
					fclose(f);
				}
			}
			printf("standby.label standby\n");
			return 0;
		}
	}

	/* Asking for a fetch */
	if (attribute == 0) {
		/* No data, use cached info */
		while (fgets(output, sizeof(output), f) != NULL) {
			for (i = 0; i < strlen(output); i++) {
				if (output[i] == '\n') {
					output[i] = '\0';
					break;
				}
			}
			printf("%s.value U\n", output);
		}
		printf("standby.value 1\n");
		fclose(f);
	} else {
		do {
			attribute--;
			printf("%s.value %s\n", label[attribute],
			       value[attribute]);
		} while (attribute);
		printf("standby.value 0\n");
	}

	return 0;
}
