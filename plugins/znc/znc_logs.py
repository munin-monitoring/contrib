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
env.logdir /var/lib/znc/moddata/log/ # path to the GLOBAL log-folder with a "/" at the end
env.expire 0    # Keep channel names forever  - OR -
env.expire 1    # Forget channel names from last run

=head1 COPYRIGHT
GPL VERSION 3

=head1 AUTHOR
Thor77 <thor77[at]thor77.org>
'''
import json
import os, sys, time
import re
import stat
import traceback

logdir = os.environ.get('logdir')
expire = os.environ.get('expire', 0)

if not logdir:
    raise Exception('You have to set the logdir with env.logdir <path to log> in the plugin-conf!')

date = time.strftime('%Y%m%d')
longdate = time.strftime('%Y-%m-%d')
last_values_file = os.environ['MUNIN_PLUGSTATE'] + '/znc_logs_last'

def get_last():
    try:
        d = {}
        with open(last_values_file, 'r') as f:
            d = json.load(f)
        return d
    except FileNotFoundError:
        return {}

def tail_open(filename, position=0):
    # Based on tail_open from perls' Munin::Plugin
    filereset = 0
    size = os.stat(filename)[stat.ST_SIZE]
    if size is None:
        return (undef, undef)
    f = open(filename, 'r', encoding='utf-8', errors='replace')
    if position > size:
        filereset = 1
    else:
        f.seek(position, 0)
        newpos = f.tell()
        if newpos != position:
            raise Exception
    return (f, filereset)

def tail_close(fh):
    position = fh.tell()
    fh.close()
    return position


last = get_last()
if "users" in last:
    user_list = last["users"]
else:
    user_list = {}
if "channels" in last:
    channel_list = last["channels"]
else:
    channel_list = {}
if "log_pos" in last:
    log_pos = last["log_pos"]
else:
    log_pos = {}

channel_stats = {}
user_stats = {}

def read_data(savestate=True):
    # Version 1.6 will change to directory-based filing, so walk recursively
    for (dirpath, dirnames, filenames) in os.walk(logdir):
        for filename in filenames:
            filename_ = filename.replace('.log', '')

            user, network, channel, file_date = (None, None, None, None)

            try:
                if len(dirpath) > len(logdir):
                    # We're below the log path, so this is a 1.6-style log
                    reldir = dirpath.replace(logdir + "/", '', 1)
                    try:
                        network, channel = reldir.split(os.sep)
                    except ValueError as e:
                        user, network, channel = reldir.split(os.sep)
                    file_date = filename_
                else:
                    try:
                        network, channel, file_date = filename_.split('_')
                    except ValueError as e:
                        user, network, channel, file_date = filename_.split('_')
            except ValueError as e:
                continue
            network_channel = '{}@{}'.format(channel, network)
            if network.lower() not in channel_list:
                channel_list[network.lower()] = {}
            if channel.startswith('#'):
                channel_list[network.lower()][channel.lower()] = network_channel
            user_list[user.lower()] = user
            # check if log is from today
            if (file_date == date or file_date == longdate):
                # current lines in the file
                (fh, r) = tail_open(os.path.join(dirpath,filename), log_pos.get(os.path.join(dirpath, filename), 0))
                current_value = 0
                while True:
                    where = fh.tell()
                    line = fh.readline()
                    if line.endswith('\n'):
                        current_value += 1
                    else:
                        # Incomplete last line
                        fh.seek(where, 0)
                        log_pos[os.path.join(dirpath, filename)] = tail_close(fh)
                        break

                if network_channel.lower() in channel_stats and channel.startswith('#'):
                    channel_stats[network_channel.lower()] += current_value
                else:
                    channel_stats[network_channel.lower()] = current_value

                if user is not None and user.lower() in user_stats:
                    user_stats[user.lower()] += current_value
                else:
                    user_stats[user.lower()] = current_value
    if savestate:
        savedata = {}
        if int(expire) == 0:
            savedata["users"] = user_list
            savedata["channels"] = channel_list
        savedata["log_pos"] = log_pos
        with open(last_values_file, 'w') as f:
            json.dump(savedata,f)


def emit_config():
    print('graph_title Lines in the ZNC-log')
    print('graph_category chat')
    print('graph_vlabel lines / ${graph_period}')
    print('graph_scale no')
    print('graph_args --base 1000 --lower-limit 0')
    print('graph_period minute')
    graph_order = []

    if os.getenv('MUNIN_CAP_DIRTYCONFIG') == "1":
        read_data(1)
    else:
        read_data(0)

    for network in channel_list.keys():
        for channel in channel_list[network].keys():

            # print things to munin
            network_channel = "{}_{}".format(network,channel).replace('.', '').replace('#', '').replace('@','_')
            print('{network_channel}.label {label}'.format(network_channel=network_channel, label=channel_list[network][channel]))
            print('{network_channel}.type  ABSOLUTE'.format(network_channel=network_channel))
            print('{network_channel}.min   0'.format(network_channel=network_channel))
            print('{network_channel}.draw  AREASTACK'.format(network_channel=network_channel))

            graph_order.append(network_channel)
    for user in user_list.keys():
            fuser = re.sub(r'^[^A-Za-z_]', '_', user)
            fuser = re.sub(r'[^A-Za-z0-9_]', '_', fuser)
            print('{fuser}.label User {user}'.format(fuser=fuser, user=user))
            print('{fuser}.type  ABSOLUTE'.format(fuser=fuser))
            print('{fuser}.min   0'.format(fuser=fuser))
            print('{fuser}.draw  LINE1'.format(fuser=fuser))

    print('graph_order {}'.format(" ".join(sorted(graph_order, key=str.lower))))

def emit_values():
    read_data(1)
    for network in channel_list.keys():
        for channel in channel_list[network].keys():

            # print things to munin
            key = channel_list[network][channel]
            network_channel = "{}_{}".format(network,channel).replace('.', '').replace('#', '').replace('@','_')
            if key.lower() in channel_stats:
                print('{network_channel}.value {value}'.format(network_channel=network_channel, value=channel_stats[key.lower()]))
            else:
                print('{network_channel}.value U'.format(network_channel=network_channel))
    for user in user_list.keys():
        fuser = re.sub(r'^[^A-Za-z_]', '_', user)
        fuser = re.sub(r'[^A-Za-z0-9_]', '_', fuser)
        if user.lower() in user_stats:
            print('{fuser}.value {value}'.format(fuser=fuser, value=user_stats[user.lower()]))
        else:
            print('{fuser}.value U'.format(fuser=fuser))


if len(sys.argv) > 1 and sys.argv[1] == 'config':
    emit_config()
    sys.exit(0)

emit_values()
