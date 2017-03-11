/*
 * if1sec C plugin
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <fcntl.h>

#include <time.h>

#include <sys/file.h>

#define PROC_STAT "/proc/net/dev"
#define PLUGIN_NAME "if1sec-c"

int fail(char* msg) {
	perror(msg);

	return 1;
}

/* Returns the ifname from a /proc/net/dev line
 * It will return an inside pointer to line, and modifiy the end with a \0
 */
char* get_ifname_from_procstatline(char* line) {
	char *ifname;
	for (ifname = line; (*ifname) == ' '; ifname ++);

	char *ifname_end;
	for (ifname_end = ifname; (*ifname_end) != ':'; ifname_end ++);
	(*ifname_end) = '\0';

	return ifname;
}

int config() {
	/* Get the number of if */
	int f;
	if ( !(f=open(PROC_STAT, O_RDONLY)) ) {
		return fail("cannot open " PROC_STAT);
	}

	// Starting with -2, since the 2 lines on top are header lines
	int nif = -2;

	const int buffer_size = 64 * 1024;
	char buffer[buffer_size];

	// whole /proc/stat can be read in 1 syscall
	if (read(f, buffer, buffer_size) <= 0) {
		return fail("cannot read " PROC_STAT);
	}

	// tokenization per-line
	char* line; char *saveptr;
	char* newl = "\n";
	for (line = strtok_r(buffer, newl, &saveptr); line; line = strtok_r(NULL, newl, &saveptr)) {
		// Skip the header lines
		if (nif ++ < 0) { continue; }

		char* if_name = get_ifname_from_procstatline(line);
		printf(
			"multigraph if_%s_1sec" "\n"
			"graph_order down up" "\n"
			"graph_title %s traffic" "\n"
			"graph_category 1sec" "\n"
			"graph_vlabel bits in (-) / out (+) per ${graph_period}" "\n"
			"graph_data_size custom 1d, 10s for 1w, 1m for 1t, 5m for 1y" "\n"
			, if_name, if_name
		);

		printf(
			"down.label -" "\n"
			"down.type DERIVE" "\n"
			"down.graph no" "\n"
			"down.cdef down,8,*" "\n"
			"down.min 0" "\n"
					        
			"up.label bps" "\n"
			"up.type DERIVE" "\n"
			"up.negative down" "\n"
			"up.cdef down,8,*" "\n"
			"up.min 0" "\n"
		);
	}

	close(f);


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
	int cache_file = open(cache_filename, O_CREAT | O_APPEND | O_WRONLY, S_IRUSR | S_IWUSR);

	/* loop each second */
	while (1) {
		/* wait until next second */
		time_t epoch = wait_until_next_second();

		const int buffer_size = 64 * 1024;
		char buffer[buffer_size];

		if (lseek(f, 0, SEEK_SET) < 0) {
			return fail("cannot seek " PROC_STAT);
		}

		// whole PROC file can be read in 1 syscall
		if (read(f, buffer, buffer_size) <= 0) {
			return fail("cannot read " PROC_STAT);
		}

		// ignore the 1rst line
		char* line; char *saveptr;
		const char* newl = "\n";

		/* lock */
		flock(cache_file, LOCK_EX);

		int nif = -2;
		for (line = strtok_r(buffer, newl, &saveptr); line; line = strtok_r(NULL, newl, &saveptr)) {
			// Skip the header lines
			if (nif ++ < 0) { continue; }

			char if_id[64];
			uint_fast64_t r_bytes, r_packets, r_errs, r_drop, r_fifo, r_frame, r_compressed, r_multicast;
			uint_fast64_t t_bytes, t_packets, t_errs, t_drop, t_fifo, t_frame, t_compressed, t_multicast;
			sscanf(line, "%s" 
				" "
				"%llu %llu %llu %llu %llu %llu %llu %llu" 
				" " 
				"%llu %llu %llu %llu %llu %llu %llu %llu"
				, if_id
				, &r_bytes, &r_packets, &r_errs, &r_drop, &r_fifo, &r_frame, &r_compressed, &r_multicast
				, &t_bytes, &t_packets, &t_errs, &t_drop, &t_fifo, &t_frame, &t_compressed, &t_multicast
			);

			// Remove trailing ':' of if_id
			if_id[strlen(if_id) - 1] = '\0';

			char out_buffer[1024];
			sprintf(out_buffer, 
				"multigraph if_%s_1sec" "\n"
				"up.value %ld:%llu" "\n"
				"down.value %ld:%llu" "\n"
				, if_id 
				, epoch, r_bytes
				, epoch, t_bytes
			);

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

	pid_filename = malloc(MUNIN_PLUGSTATE_length + strlen("/" PLUGIN_NAME ".") + strlen("pid") + 1); pid_filename[0] = '\0';
	cache_filename = malloc(MUNIN_PLUGSTATE_length + strlen("/" PLUGIN_NAME ".") + strlen("value") + 1); cache_filename[0] = '\0';

	strcat(pid_filename, MUNIN_PLUGSTATE);
	strcat(pid_filename, "/" PLUGIN_NAME "." "pid");

	strcat(cache_filename, MUNIN_PLUGSTATE);
	strcat(cache_filename, "/" PLUGIN_NAME "." "value");

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

/***** DEMO
 
/proc/net/dev sample

Inter-|   Receive                                                |  Transmit
 face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
    lo:    4364      54    0    0    0     0          0         0     4364      54    0    0    0     0       0          0
  eth0: 3459461624 22016512    0   70    0     0          0         0 3670486138 18117144    0    0    0     0       0          0

*****/
