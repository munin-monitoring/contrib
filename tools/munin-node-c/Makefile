CC=gcc
CFLAGS=-W -Wall -pedantic -Wextra -g -O2
OBJS=main.o
LINKS=

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@
all: munin-node-c

munin-node-c: ${OBJS}
	${CC} ${CFLAGS} $^ -o $@
clean:
	rm -f munin-node-c ${OBJS} ${LINKS}
	rm -Rf plugins

plugins:
	mkdir -p plugins
	cd ../munin-plugins-busybox && make
	cd plugins && for i in $$(find ../../munin-plugins-busybox -type l); do ln -s $$i; done

.PHONY: all clean plugins
