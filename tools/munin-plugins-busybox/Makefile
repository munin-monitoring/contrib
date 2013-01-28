CC=gcc
CFLAGS=-W -Wall -pedantic -Wextra -g -O2
OBJS=main.o common.o cpu.o entropy.o forks.o fw_packets.o interrupts.o \
     if_err_.o load.o open_files.o open_inodes.o processes.o swap.o uptime.o
LINKS=cpu entropy forks fw_packets interrupts if_err_eth0 load open_files \
      open_inodes processes swap uptime

%.o:%.c
	${CC} ${CFLAGS} -c $< -o $@
all:munin-plugins-busybox
	strip -s munin-plugins-busybox
	for l in ${LINKS}; do test -f $$l || ln -s munin-plugins-busybox $$l; done
munin-plugins-busybox:${OBJS}
	${CC} ${CFLAGS} $^ -o $@
clean:
	rm -f munin-plugins-busybox ${OBJS} ${LINKS}
.PHONY:all clean
