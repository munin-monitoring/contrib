#!/usr/bin/python3
# -*- coding: utf-8 -*-
'''
=head1 NAME
znc_logs

=head1 DESCRIPTION
Shows lines/minute in today's znc-logs

=head2 CONFIGURATION
[znc_logs]
user znc # or any other user/group that can read the znclog-folder
group znc
env.logdir /var/lib/znc/moddata/log/ # path to the log-folder with a "/" at the end

=head1 COPYRIGHT
GPL VERSION 3

=head1 AUTHOR
Thor77 <thor77[at]thor77.org>
'''
from sys import argv
from time import strftime
from os import environ, listdir

logdir = environ.get('logdir')

if not logdir:
    raise Exception('You have to set the logdir with env.logdir <path to log> in the plugin-conf!')

date = strftime('%Y%m%d')
last_values_file = environ['MUNIN_PLUGSTATE'] + '/last_values'


def get_last():
    try:
        d = {}
        with open(last_values_file, 'r') as f:
            for line in f:
                line = line[:-1]
                key, value = line.split(':')
                d[key] = float(value)
        return d
    except FileNotFoundError:
        return {}


def data():
    last = get_last()
    current = {}
    for filename in listdir(logdir):
        filename_ = filename.replace('.log', '')
        network, channel, file_date = filename_.split('_')
        network_channel = '{}_{}'.format(network, channel)
        # check if log is from today and it is a channel
        if file_date == date and channel.startswith('#'):
            # current lines in the file
            current_value = sum(1 for i in open(logdir + filename, 'r', encoding='utf-8', errors='replace'))

            if network_channel not in last:
                value = 0
            else:
                last_value = last[network_channel]
                # what munin gets
                value = (current_value - last_value) / 5  # subtrate last from current and divide through 5 to get new lines / minute
                if value < 0:
                    value = 0
            # save it to the states-file
            current[network_channel] = current_value

            # print things to munin
            network_channel = network_channel.replace('.', '').replace('#', '')
            print('{network_channel}.label {channel}@{network}'.format(network_channel=network_channel, channel=channel, network=network))
            print('{network_channel}.value {value}'.format(network_channel=network_channel, value=value))
    with open(last_values_file, 'w') as f:
        for k in current:
            f.write('{}:{}\n'.format(k, current[k]))


if len(argv) > 1 and argv[1] == 'config':
    print('graph_title Lines in the ZNC-log')
    print('graph_category znc')
    print('graph_vlabel lines/minute')
    print('graph_scale no')
data()
