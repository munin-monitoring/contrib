#include <ctype.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PROC_NET_DEV "/proc/net/dev"

int if_err_(int argc, char **argv) {
	char *interface;
	FILE *f;
	char buff[256], *s;
	int i;

	interface = basename(argv[0]);
	if(strncmp(interface, "if_err_", 7) != 0) {
		fputs("if_err_ invoked with invalid basename\n", stderr);
		return 1;
	}
	interface += 7;

	if(argc > 1) {
		if(!strcmp(argv[1], "autoconf")) {
			if(access(PROC_NET_DEV, R_OK) == 0) {
				puts("yes");
				return 0;
			} else {
				puts("no (/proc/net/dev not found)");
				return 1;
			}
		}
		if(!strcmp(argv[1], "suggest")) {
			if(NULL == (f = fopen(PROC_NET_DEV, "r")))
				return 1;
			while(fgets(buff, 256, f)) {
				for(s=buff;*s == ' ';++s)
					;
				i = 0;
				if(!strncmp(s, "eth", 3))
					i = 3;
				else if(!strncmp(s, "wlan", 4))
					i = 4;
				else if(!strncmp(s, "ath", 3))
					i = 3;
				else if(!strncmp(s, "ra", 2))
					i = 2;
				if(i == 0)
					continue;
				while(isdigit(s[i]))
					++i;
				if(s[i] != ':')
					continue;
				s[i] = '\0';
				puts(s);
			}
			fclose(f);
			return 0;
		}
		if(!strcmp(argv[1], "config")) {
			puts("graph_order rcvd trans");
			printf("graph_title %s errors\n", interface);
			puts("graph_args --base 1000\n"
				"graph_vlabel packets in (-) / out (+) per "
					"${graph_period}\n"
				"graph_category network");
			printf("graph_info This graph shows the amount of "
				"errors on the %s network interface.\n",
				interface);
			puts("rcvd.label packets\n"
				"rcvd.type COUNTER\n"
				"rcvd.graph no\n"
				"rcvd.warning 1\n"
				"trans.label packets\n"
				"trans.type COUNTER\n"
				"trans.negative rcvd\n"
				"trans.warning 1");
			return 0;
		}
	}
	if(NULL == (f = fopen(PROC_NET_DEV, "r")))
		return 1;
	while(fgets(buff, 256, f)) {
		for(s=buff;*s == ' ';++s)
			;
		if(0 != strncmp(s, interface, strlen(interface)))
			continue;
		s += strlen(interface);
		if(*s != ':')
			continue;
		++s;

		while(*s == ' ')
			++s;

		for(i=1;i<3;++i) {
			while(isdigit(*s))
				++s;
			while(isspace(*s))
				++s;
		}
		for(i=0;isdigit(s[i]);++i)
			;
		printf("rcvd.value ");
		fwrite(s, 1, i, stdout);
		putchar('\n');
		s += i;
		while(isspace(*s))
			++s;

		for(i=4;i<11;++i) {
			while(isdigit(*s))
				++s;
			while(isspace(*s))
				++s;
		}
		for(i=0;isdigit(s[i]);++i)
			;
		printf("trans.value ");
		fwrite(s, 1, i, stdout);
		putchar('\n');
	}
	return 0;
}
