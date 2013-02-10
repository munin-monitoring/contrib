#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include "common.h"

#define SYSWARNING 30
#define SYSCRITICAL 50
#define USRWARNING 80

/* TODO: port support for env.foo_warning and env.foo_critical from mainline plugin */

int cpu(int argc, char **argv) {
	FILE *f;
	char buff[256], *s;
	int ncpu=0, extinfo=0, scaleto100=0, hz;
	if(argc > 1) {
		if(!strcmp(argv[1], "config")) {
			s = getenv("scaleto100");
			if(s && !strcmp(s, "yes"))
				scaleto100=1;

			if(!(f=fopen(PROC_STAT, "r")))
				return fail("cannot open " PROC_STAT);
			while(fgets(buff, 256, f)) {
				if(!strncmp(buff, "cpu", 3)) {
					if(isdigit(buff[3]))
						ncpu++;
					if(buff[3] == ' ') {
						s = strtok(buff+4, " \t");
						for(extinfo=0;strtok(NULL, " \t");extinfo++)
							;
					}
				}
			}
			fclose(f);

			if(ncpu < 1 || extinfo < 4)
				return fail("cannot parse " PROC_STAT);

			puts("graph_title CPU usage");
			if(extinfo >= 7)
				puts("graph_order system user nice idle iowait irq softirq");
			else
				puts("graph_order system user nice idle");
			if(scaleto100)
				puts("graph_args --base 1000 -r --lower-limit 0 --upper-limit 100");
			else
				printf("graph_args --base 1000 -r --lower-limit 0 --upper-limit %d\n", 100 * ncpu);
			puts("graph_vlabel %\n"
				"graph_scale no\n"
				"graph_info This graph shows how CPU time is spent.\n"
				"graph_category system\n"
				"graph_period second\n"
				"system.label system\n"
				"system.draw AREA");
			printf("system.max %d\n", 100 * ncpu);
			puts("system.min 0\n"
				"system.type DERIVE");
			printf("system.warning %d\n", SYSWARNING * ncpu);
			printf("system.critical %d\n", SYSCRITICAL * ncpu);
			puts("system.info CPU time spent by the kernel in system activities\n"
				"user.label user\n"
				"user.draw STACK\n"
				"user.min 0");
			printf("user.max %d\n", 100 * ncpu);
			printf("user.warning %d\n", USRWARNING * ncpu);
			puts("user.type DERIVE\n"
				"user.info CPU time spent by normal programs and daemons\n"
				"nice.label nice\n"
				"nice.draw STACK\n"
				"nice.min 0");
			printf("nice.max %d\n", 100 * ncpu);
			puts("nice.type DERIVE\n"
				"nice.info CPU time spent by nice(1)d programs\n"
				"idle.label idle\n"
				"idle.draw STACK\n"
				"idle.min 0");
			printf("idle.max %d\n", 100 * ncpu);
			puts("idle.type DERIVE\n"
				"idle.info Idle CPU time");
			if(scaleto100)
				printf("system.cdef system,%d,/\n"
					"user.cdef user,%d,/\n"
					"nice.cdef nice,%d,/\n"
					"idle.cdef idle,%d,/\n", ncpu, ncpu, ncpu, ncpu);
			if(extinfo >= 7) {
				puts("iowait.label iowait\n"
					"iowait.draw STACK\n"
					"iowait.min 0");
				printf("iowait.max %d\n", 100 * ncpu);
				puts("iowait.type DERIVE\n"
					"iowait.info CPU time spent waiting for I/O operations to finish\n"
					"irq.label irq\n"
					"irq.draw STACK\n"
					"irq.min 0");
				printf("irq.max %d\n", 100 * ncpu);
				puts("irq.type DERIVE\n"
					"irq.info CPU time spent handling interrupts\n"
					"softirq.label softirq\n"
					"softirq.draw STACK\n"
					"softirq.min 0");
				printf("softirq.max %d\n", 100 * ncpu);
				puts("softirq.type DERIVE\n"
					"softirq.info CPU time spent handling \"batched\" interrupts");
				if(scaleto100)
					printf("iowait.cdef iowait,%d,/\n"
						"irq.cdef irq,%d,/\n"
						"softirq.cdef softirq,%d,/\n", ncpu, ncpu, ncpu);
			}
			if(extinfo >= 8) {
				puts("steal.label steal\n"
					"steal.draw STACK\n"
					"steal.min 0");
				printf("steal.max %d\n", 100 * ncpu);
				puts("steal.type DERIVE\n"
					"steal.info The time that a virtual CPU had runnable tasks, but the virtual CPU itself was not running");
				if(scaleto100)
					printf("steal.cdef steal,%d,/\n", ncpu);
			}
			if(extinfo >= 9) {
				puts("guest.label guest\n"
					"guest.draw STACK\n"
					"guest.min 0");
				printf("guest.max %d\n", 100 * ncpu);
				puts("guest.type DERIVE\n"
					"guest.info The time spent running a virtual CPU for guest operating systems under the control of the Linux kernel.");
				if(scaleto100)
					printf("guest.cdef guest,%d,/\n", ncpu);
			}
			return 0;
		}
		if(!strcmp(argv[1], "autoconf"))
			return autoconf_check_readable(PROC_STAT);
	}
	if(!(f=fopen(PROC_STAT, "r")))
		return fail("cannot open " PROC_STAT);
	while(fgets(buff, 256, f)) {
		hz = getenvint("HZ", 100);
		if(!strncmp(buff, "cpu ", 4)) {
			fclose(f);
			if(!(s = strtok(buff+4, " \t")))
				break;
			printf("user.value %ld\n", atol(s) * 100 / hz);
			if(!(s = strtok(NULL, " \t")))
				break;
			printf("nice.value %ld\n", atol(s) * 100 / hz);
			if(!(s = strtok(NULL, " \t")))
				break;
			printf("system.value %ld\n", atol(s) * 100 / hz);
			if(!(s = strtok(NULL, " \t")))
				break;
			printf("idle.value %ld\n", atol(s) * 100 / hz);
			if(!(s = strtok(NULL, " \t")))
				return 0;
			printf("iowait.value %ld\n", atol(s) * 100 / hz);
			if(!(s = strtok(NULL, " \t")))
				return 0;
			printf("irq.value %ld\n", atol(s) * 100 / hz);
			if(!(s = strtok(NULL, " \t")))
				return 0;
			printf("softirq.value %ld\n", atol(s) * 100 / hz);
			if(!(s = strtok(NULL, " \t")))
				return 0;
			printf("steal.value %ld\n", atol(s) * 100 / hz);
			if(!(s = strtok(NULL, " \t")))
				return 0;
			printf("guest.value %ld\n", atol(s) * 100 / hz);
			return 0;
		}
	}
	fclose(f);
	return fail("no cpu line found in " PROC_STAT);
}
