/*
 * gpsd.c - a C rewrite of the 'gpsd' plugin
 *
 * The graphs are basically identical to the original Ruby plugin, but there is
 * no longer a dependency on the "gpspipe" utility; a direct TCP connection to
 * gpsd is established, and the JSON is parsed using the json-c library.
 *
 * The intent here is to provide the same functionality on systems that are
 * resource-constrained and/or sensitive to jitter (e.g., an embedded NTP
 * server).
 *
 * This can be compiled like such after editing GPSD_HOST / GPSD_PORT:
 *
 * 	gcc -o gpsd gpsd.c -ljson-c
 *
 * Original plugin: https://github.com/munin-monitoring/contrib/tree/master/plugins/gpsd/gpsd
 *
 * C rewrite Copyright (C) 2023 Matt Merhar <mattmerhar@protonmail.com>
 * Original Copyright (C) 2022 Kenyon Ralph <kenyon@kenyonralph.com>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see L<https://www.gnu.org/licenses/>.
 */

#define GPSD_HOST "127.0.0.1"
#define GPSD_PORT 2947
#define GPSD_MAX_SATELLITES "16"
#define GPSD_BUFSIZE 2048
#define GPSD_TIMEOUT 5000
#define GPSD_EOL "\r\n"
#define GPSD_COMMAND_WATCH "?WATCH={\"class\":\"WATCH\",\"enable\":true,\"json\":true}\n"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <poll.h>
#include <json-c/json.h>

typedef struct {
	char *key;
	char *label;
	char *info;
	char *type;
	char *min;
	char *max;
} metric_t;

typedef struct {
	char *name;
	char *title;
	char *category;
	char *vlabel;
	metric_t metrics[7];
	size_t count;
} plugin_t;

plugin_t plugins[] = {
	{
		.name = "gpsd_dop",
		.title = "GPS dop",
		.category = "gps",
		.vlabel = "dilution of precision",
		.metrics = {
			{ .key = "xdop", .label = "xdop", .info = "Longitudinal dilution of precision", .type = "GAUGE", .min = NULL, .max = NULL },
			{ .key = "ydop", .label = "ydop", .info = "Latitudinal dilution of precision", .type = "GAUGE", .min = NULL, .max = NULL },
			{ .key = "vdop", .label = "vdop", .info = "Vertical (altitude) dilution of precision", .type = "GAUGE", .min = NULL, .max = NULL },
			{ .key = "tdop", .label = "tdop", .info = "Time dilution of precision", .type = "GAUGE", .min = NULL, .max = NULL },
			{ .key = "hdop", .label = "hdop", .info = "Horizontal dilution of precision", .type = "GAUGE", .min = NULL, .max = NULL },
			{ .key = "gdop", .label = "gdop", .info = "Geometric (hyperspherical) dilution of precision, a combination of PDOP and TDOP", .type = "GAUGE", .min = NULL, .max = NULL },
			{ .key = "pdop", .label = "pdop", .info = "Position (spherical/3D) dilution of precision", .type = "GAUGE", .min = NULL, .max = NULL },
		},
		.count = 7,
	},
	{
		.name = "gpsd_satellites",
		.title = "GPS satellites",
		.category = "gps",
		.vlabel = "number of satellites",
		.metrics = {
			{ .key = "nSat", .label = "nSat", .info = "Number of satellites seen", .type = "GAUGE", .min = "0", .max = GPSD_MAX_SATELLITES },
			{ .key = "uSat", .label = "uSat", .info = "Number of satellites used", .type = "GAUGE", .min = "0", .max = GPSD_MAX_SATELLITES },
		},
		.count = 2,
	},
};

int gpsd_connect(char *host, uint16_t port) {
	int fd;
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(port),
		.sin_addr.s_addr = inet_addr(host)
	};


	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket failed");
		exit(errno);
	}

	if (connect(fd, (struct sockaddr *)&addr, (socklen_t)sizeof(addr))) {
		perror("connect failed");
		exit(errno);
	}

	if (fcntl(fd, F_SETFL, O_RDWR|O_NONBLOCK) == -1) {
		perror("fcntl failed");
		exit(errno);
	}

	return fd;
}

bool gpsd_process_response(char *line, int fd) {
	struct json_object *object, *key;
	enum json_tokener_error error;
	const char *value;
	bool complete = false;

	if (!strlen(line))
		return false;

	if ((object = json_tokener_parse_verbose(line, &error)) == NULL) {
		fprintf(stderr, "parsing json failed: %s\n", json_tokener_error_desc(error));
		exit(EXIT_FAILURE);
	}

	if (!json_object_object_get_ex(object, "class", &key)) {
		fputs("json missing class key\n", stderr);
		exit(EXIT_FAILURE);
	}

	value = json_object_get_string(key);

	if (!strcmp(value, "VERSION")) {
		write(fd, GPSD_COMMAND_WATCH, sizeof(GPSD_COMMAND_WATCH));
	} else if (!strcmp(value, "SKY")) {
		if (!json_object_object_get_ex(object, "satellites", &key)) {
			goto end;
		}

		for (size_t p = 0; p < sizeof(plugins) / sizeof(plugin_t); p++) {
			plugin_t *plugin = &plugins[p];

			printf("multigraph %s\ngraph_title %s\ngraph_category %s\ngraph_vlabel %s\n", plugin->name, plugin->title, plugin->category, plugin->vlabel);

			for (size_t m = 0; m < plugin->count; m++) {
				metric_t *metric = &plugin->metrics[m];

				printf("%s.label %s\n", metric->key, metric->label);
				printf("%s.info %s\n", metric->key, metric->info);
				printf("%s.type %s\n", metric->key, metric->type);
				if (metric->min)
					printf("%s.min %s\n", metric->key, metric->min);
				if (metric->max)
					printf("%s.max %s\n", metric->key, metric->max);

				json_object_object_get_ex(object, metric->key, &key);
				printf("%s.value %.2f\n", metric->key, json_object_get_double(key));
			}
		}

		complete = true;
	} else {
		// ignore
	}

end:
	json_object_put(object);

	return complete;
}

bool gpsd_read_cb(char *data, ssize_t bytes, int fd) {
	static char sbuf[GPSD_BUFSIZE + 1] = {};
	static ssize_t length = 0;
	char *response = (char *)&sbuf;
	ssize_t offset = length;

	if (bytes + length > GPSD_BUFSIZE) {
		fputs("read buffer full\n", stderr);
		exit(EXIT_FAILURE);
	}

	data[bytes] = '\0';
	strncat((char *)&sbuf, data, GPSD_BUFSIZE);
	length += bytes;

	if (!strstr(data, GPSD_EOL))
		return false;

	while (offset < length) {
		if (iscntrl(sbuf[offset])) {
			do {
				sbuf[offset] = '\0';
			} while (++offset < length && iscntrl(sbuf[offset]));

			if (gpsd_process_response(response, fd))
				return true;

			response = (char *)&sbuf[offset];

			if (!strstr(data, GPSD_EOL))
				break;
		}

		offset++;
	}

	length = strlen(response);
	memmove(&sbuf, response, length + 1);

	return false;
}

int main(int argc, const char *argv[]) {
	struct pollfd fds[1] = {};
	struct pollfd *gpsd = &fds[0];
	int p;

	if (argc != 2 || strcmp(argv[1], "config")) {
		fputs("plugin designed for the dirtyconfig protocol, must be run with the config argument\n", stderr);
		exit(EXIT_FAILURE);
	}

	gpsd->fd = gpsd_connect(GPSD_HOST, GPSD_PORT);
	gpsd->events = POLLIN;

	while ((p = poll(fds, 1, GPSD_TIMEOUT)) > 0) {
		if (gpsd->revents & (POLLERR | POLLHUP | POLLNVAL)) {
			int error;
			socklen_t errorlen = sizeof(error);
			if (getsockopt(gpsd->fd, SOL_SOCKET, SO_ERROR, &error, &errorlen) < 0) {
				perror("getsockopt failed");
				exit(errno);
			}

			fprintf(stderr, "socket error: %s\n", strerror(error));
			exit(error);
		} else if (gpsd->revents & POLLIN) {
			char buffer[GPSD_BUFSIZE + 1] = {};
			ssize_t bytes;

			while ((bytes = read(gpsd->fd, &buffer, GPSD_BUFSIZE)) >= 0) {
				if (!bytes) {
					fputs("unexpected eof\n", stderr);
					exit(EXIT_FAILURE);
				}

				if (gpsd_read_cb((char *)&buffer, bytes, gpsd->fd))
					goto done;
			}
		} else {
			// ignore
		}
	}

done:
	close(gpsd->fd);

	if (!p) {
		fputs("read timeout\n", stderr);
		exit(EXIT_FAILURE);
	} else if (p == -1) {
		perror("poll failed");
		exit(errno);
	}

	exit(EXIT_SUCCESS);
}
