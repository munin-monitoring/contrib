#!/usr/bin/python
# Copyright (C) 2013 Helmut Grohne <helmut@subdivi.de>
# License: GPLv2 like the rest of munin
"""
Usage:
    tcpdump -npi lo "tcp port 4949" -w munin.pcap
    # wait for one munin run, then press Ctrl-C
    ./munin-profile-node.py munin.pcap
"""

import sys
from scapy.utils import rdpcap
import scapy.layers.l2
from scapy.layers.inet import TCP

class MuninProfiler:
    def __init__(self):
        self.to_node = ""
        self.from_node = ""
        self.times = dict()
        self.idles = []
        self.curcommand = None
        self.commandstart = None

    def handle_packet(self, packet):
        payload = str(packet[TCP].payload)
        if not payload:
            return
        if packet[TCP].dport == 4949:
            self.to_node += payload
        else:
            self.from_node += payload
        lines = self.to_node.split("\n")
        self.to_node = lines.pop()
        for line in lines:
            self.handle_to_node(packet.time, line)
        lines = self.from_node.split("\n")
        self.from_node = lines.pop()
        for line in lines:
            self.handle_from_node(packet.time, line)

    def handle_to_node(self, timestamp, line):
        if self.curcommand is None and self.commandstart is not None:
            self.idles.append(timestamp - self.commandstart)
        self.curcommand = line
        self.commandstart = timestamp

    def handle_from_node(self, timestamp, line):
        if line != ".":
            return
        if self.curcommand is None:
            return
        duration = timestamp - self.commandstart
        self.times.setdefault(self.curcommand, []).append(duration)
        self.curcommand = None
        self.commandstart = timestamp

def main():
    mp = MuninProfiler()
    for pkt in rdpcap(sys.argv[1]):
        mp.handle_packet(pkt)
    print("Client idle time during connection: %.2fs" % sum(mp.idles))
    times = [(key, sum(value)) for key, value in mp.times.items()]
    times.sort(key=lambda tpl: -tpl[1])
    total = sum(value for key, value in times)
    print("Total time waiting for the node:    %.2fs" % total)
    print("Top 10 plugins using most of the time")
    for key, value in times[:10]:
        print("%-40s: %.2fs (%d%%)" % (key, value, 100 * value / total))

if __name__ == '__main__':
    main()
