/*
 * multicpu1sec C plugin
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <time.h>

#include <sys/file.h>

#define PROC_STAT "/proc/stat"

int fail(char* msg) {
	perror(msg);

	return 1;
}

int config() {
	/* Get the number of CPU */
	int f;
	if ( !(f=open(PROC_STAT, O_RDONLY)) ) {
		return fail("cannot open " PROC_STAT);
	}

	// Starting with -1, since the first line is the "global cpu line"
	int ncpu = -1;

	const int buffer_size = 64 * 1024;
	char buffer[buffer_size];

	// whole /proc/stat can be read in 1 syscall
	if (read(f, buffer, buffer_size) <= 0) {
		return fail("cannot read " PROC_STAT);
	}

	// tokenization per-line
	char* line;
	char* newl = "\n";
	for (line = strtok(buffer, newl); line; line = strtok(NULL, newl)) {
		if (! strncmp(line, "cpu", 3)) ncpu ++;
	}

	close(f);

	printf(
		"graph_title multicpu1sec\n"
		"graph_category 1sec\n"
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

char* pid_filename;
char* cache_filename;

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

	/* Reading /proc/stat */
	int f = open(PROC_STAT, O_RDONLY);

	/* open the spoolfile */
	int cache_file = open(cache_filename, O_CREAT | O_APPEND | O_WRONLY);

	/* loop each second */
	while (1) {
		/* wait until next second */
		time_t epoch = wait_until_next_second();


		const int buffer_size = 64 * 1024;
		char buffer[buffer_size];

		if (lseek(f, 0, SEEK_SET) < 0) {
			return fail("cannot seek " PROC_STAT);
		}

		// whole /proc/stat can be read in 1 syscall
		if (read(f, buffer, buffer_size) <= 0) {
			return fail("cannot read " PROC_STAT);
		}

		// ignore the 1rst line
		char* line;
		const char* newl = "\n";
		line = strtok(buffer, newl);

		/* lock */
		flock(cache_file, LOCK_EX);

		for (line = strtok(NULL, newl); line; line = strtok(NULL, newl)) {
			// Not on CPU lines anymore
			if (strncmp(line, "cpu", 3)) break;

			char cpu_id[64];
			long usr, nice, sys, idle, iowait, irq, softirq;
			sscanf(line, "%s %ld %ld %ld %ld %ld %ld %ld", cpu_id, &usr, &nice, &sys, &idle, &iowait, &irq, &softirq);

			long used = usr + nice + sys + iowait + irq + softirq;

			char out_buffer[1024];
			sprintf(out_buffer, "%s.value %ld:%ld\n", cpu_id, epoch, used);

			write(cache_file, out_buffer, strlen(out_buffer));
		}

		/* unlock */
		flock(cache_file, LOCK_UN);
	}

	close(cache_file);
	close(f);

	return 0;
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

	return 0;
}

int main(int argc, char **argv) {
	/* resolve paths */
	char *MUNIN_PLUGSTATE = getenv("MUNIN_PLUGSTATE");

	/* Default is current directory */
	if (! MUNIN_PLUGSTATE) MUNIN_PLUGSTATE = ".";

	size_t MUNIN_PLUGSTATE_length = strlen(MUNIN_PLUGSTATE);

	pid_filename = malloc(MUNIN_PLUGSTATE_length + strlen("/multicpu1sec.") + strlen("pid") + 1); pid_filename[0] = '\0';
	cache_filename = malloc(MUNIN_PLUGSTATE_length + strlen("/multicpu1sec.") + strlen("value") + 1); cache_filename[0] = '\0';

	strcat(pid_filename, MUNIN_PLUGSTATE);
	strcat(pid_filename, "/multicpu1sec.pid");

	strcat(cache_filename, MUNIN_PLUGSTATE);
	strcat(cache_filename, "/multicpu1sec.value");

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
