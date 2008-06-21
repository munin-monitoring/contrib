#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include "common.h"

int fw_packets(int argc, char **argv) {
	FILE *f;
	char buff[1024], *s;
	if(argc > 1) {
		if(!strcmp(argv[1], "config")) {
			puts("graph_title Firewall Throughput\n"
				"graph_args --base 1000 -l 0\n"
				"graph_vlabel Packets/${graph_period}\n"
				"graph_category network\n"
				"received.label Received\n"
				"received.draw AREA\n"
				"received.type DERIVE\n"
				"received.min 0\n"
				"forwarded.label Forwarded\n"
				"forwarded.draw LINE2\n"
				"forwarded.type DERIVE\n"
				"forwarded.min 0");
			return 0;
		}
		if(!strcmp(argv[1], "autoconf")) {
			if(0 == access("/proc/net/snmp", R_OK))
				return writeyes();
			else
				return writeno("/proc/net/snmp not readable");
		}
	}
	if(!(f=fopen("/proc/net/snmp", "r"))) {
		fputs("cannot open /proc/net/snmp\n", stderr);
		return 1;
	}
	while(fgets(buff, 1024, f)) {
		if(!strncmp(buff, "Ip: ", 4) && isdigit(buff[4])) {
			fclose(f);
			if(!(s = strtok(buff+4, " \t")))
				break;
			if(!(s = strtok(NULL, " \t")))
				break;
			if(!(s = strtok(NULL, " \t")))
				break;
			printf("received.value %s\n", s);
			if(!(s = strtok(NULL, " \t")))
				break;
			if(!(s = strtok(NULL, " \t")))
				break;
			if(!(s = strtok(NULL, " \t")))
				break;
			printf("forwarded.value %s\n", s);
			return 0;
		}
	}
	fclose(f);
	fputs("no ip line found in /proc/net/snmp\n", stderr);
	return 1;
}
