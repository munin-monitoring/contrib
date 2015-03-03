/*
 * multicpu1sec C plugin
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <time.h>

#include <sys/file.h>

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
		"graph_vlabel average cpu use %%\n"
		"graph_scale no\n"
		"graph_total All CPUs\n"
		"update_rate 1\n"
		"graph_data_size custom 1d, 10s for 1w, 1m for 1t, 5m for 1y\n"
	);

	int i;
	for (i = 0; i < ncpu; i++) {
		printf("cpu%d.label CPU %d\n", i, i);
		printf("cpu%d.draw %s\n", i, "AREASTACK");
		printf("cpu%d.type %s\n", i, "DERIVE");
		printf("cpu%d.min 0\n", i);
	}


	return 0;
}

char* pid_filename = "./multicpu1sec.pid";
char* cache_filename = "./multicpu1sec.value";

/* Wait until the next second, and return the EPOCH */
time_t wait_until_next_second() {
	struct timespec tp;
	clock_gettime(CLOCK_REALTIME, &tp);

	time_t current_epoch = tp.tv_sec;
	long nsec_to_sleep = 1000*1000*1000 - tp.tv_nsec;


	/* Only sleep if needed */
	if (nsec_to_sleep > 0) {
		tp.tv_sec = 0;
		tp.tv_nsec = nsec_to_sleep;
		nanosleep(&tp, NULL);
	}

	return current_epoch + 1;
}

int acquire() {

	/* fork ourselves if not asked otherwise */
	char* no_fork = getenv("no_fork");
	if (! no_fork || strcmp("1", no_fork)) {
		if (fork()) return;
		// we are the child, complete the daemonization

		/* Close standard IO */
		fclose(stdin);
		fclose(stdout);
		fclose(stderr);

		/* create new session and process group */
		setsid();
	}

	/* write the pid */
	FILE* pid_file = fopen(pid_filename, "w");
	fprintf(pid_file, "%d\n", getpid());
	fclose(pid_file);

	/* loop each second */
	while (1) {
		/* wait until next second */
		time_t epoch = wait_until_next_second();

		/* Reading /proc/stat */
		FILE* f = fopen(PROC_STAT, "r");
		// Read and ignore the 1rst line
		char buffer[1024];
		fgets(buffer, 1024, f);

		/* open the spoolfile */
		FILE* cache_file = fopen(cache_filename, "a");
		/* lock */
		flock(fileno(cache_file), LOCK_EX);

		while (! feof(f)) {
			if (fgets(buffer, 1024, f) == 0) {
				// EOF
				break;
			}

			// Not on CPU lines anymore
			if (strncmp(buffer, "cpu", 3)) break;

			char cpu_id[64];
			long usr, nice, sys, idle, iowait, irq, softirq;
			sscanf(buffer, "%s %ld %ld %ld %ld %ld", cpu_id, &usr, &nice, &sys, &idle, &iowait, &irq, &softirq);

			long used = usr + nice + sys + iowait + irq + softirq;

			fprintf(cache_file, "%s.value %ld:%ld\n", cpu_id, epoch, used);
		}

		fclose(cache_file);
		fclose(f);
	}
}

int fetch() {
	FILE* cache_file = fopen(cache_filename, "r+");

	/* lock */
	flock(fileno(cache_file), LOCK_EX);

	/* cat the cache_file to stdout */
	char buffer[1024];
	while (fgets(buffer, 1024, cache_file)) {
		printf("%s", buffer);
	}

	ftruncate(fileno(cache_file), 0);
	fclose(cache_file);
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
