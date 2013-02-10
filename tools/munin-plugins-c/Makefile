CC=gcc
CFLAGS=-W -Wall -pedantic -Wextra -g -O2
OBJS=main.o common.o cpu.o entropy.o forks.o fw_packets.o interrupts.o \
     if_err_.o load.o open_files.o open_inodes.o processes.o swap.o threads.o \
     uptime.o
LINKS=cpu entropy forks fw_packets interrupts if_err_eth0 load open_files \
      open_inodes processes swap threads uptime

%.o:%.c
	${CC} ${CFLAGS} -c $< -o $@
all:munin-plugins-c
	strip -s munin-plugins-c
	for l in ${LINKS}; do test -f $$l || ln -s munin-plugins-c $$l; done
munin-plugins-c:${OBJS}
	${CC} ${CFLAGS} $^ -o $@
clean:
	rm -f munin-plugins-c ${OBJS} ${LINKS}
.PHONY:all clean
