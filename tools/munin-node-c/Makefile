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
.PHONY: all clean
