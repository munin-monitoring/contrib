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

plugins: plugins/.munin-plugins-busybox.installed

plugins/.munin-plugins-busybox.installed:
	mkdir -p plugins
	cd ../munin-plugins-busybox && $(MAKE)
	cd plugins && for i in $$(find ../../munin-plugins-busybox -type l); do ln -s $$i; done
	touch plugins/.munin-plugins-busybox.installed

.PHONY: all clean plugins
