#include <ctype.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "common.h"

#define PROC_NET_DEV "/proc/net/dev"

int if_err_(int argc, char **argv) {
	char *interface;
	size_t interface_len;
	FILE *f;
	char buff[256], *s;
	int i;

	interface = basename(argv[0]);
	if(strncmp(interface, "if_err_", 7) != 0)
		return fail("if_err_ invoked with invalid basename");
	interface += 7;
	interface_len = strlen(interface);

	if(argc > 1) {
		if(!strcmp(argv[1], "autoconf"))
			return autoconf_check_readable(PROC_NET_DEV);
		if(!strcmp(argv[1], "suggest")) {
			if(NULL == (f = fopen(PROC_NET_DEV, "r")))
				return 1;
			while(fgets(buff, 256, f)) {
				for(s=buff;*s == ' ';++s)
					;
				i = 0;
				if(!strncmp(s, "lo:", 3))
					continue;
				if(!strncmp(s, "sit", 3)) {
					for(i=3; isdigit(s[i]); ++i)
						;
					if(s[i] == ':')
						continue;
				}
				while(s[i] != ':' && s[i] != '\0')
					++i;
				if(s[i] != ':')
					continue; /* a header line */
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
			print_warncrit("rcvd");
			print_warncrit("trans");
			return 0;
		}
	}
	if(NULL == (f = fopen(PROC_NET_DEV, "r")))
		return 1;
	while(fgets(buff, 256, f)) {
		for(s=buff;*s == ' ';++s)
			;
		if(0 != strncmp(s, interface, interface_len))
			continue;
		s += interface_len;
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
