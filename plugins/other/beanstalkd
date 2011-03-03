#!/usr/bin/python

import sys, re
from beanstalk import serverconn, protohandler

# Temporary workaround until my patch is merged.
if not protohandler._namematch.match('ABZDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+/;.$_()'):
    protohandler._namematch = re.compile(r'^[a-zA-Z0-9+\(\)/;.$_][a-zA-Z0-9+\(\)/;.$_-]{0,199}$')

STATES = ['ready', 'reserved', 'urgent', 'delayed', 'buried']

def connect(host='localhost', port=11300):
    return serverconn.ServerConn(host, port)

def config():
    c = connect()
    tubes = c.list_tubes()['data']
    print_config(tubes)

def print_config(tubes, graph_title='Beanstalkd jobs', graph_vlabel='count'):
    for tube in tubes:
        print 'multigraph job_count_' + tube
        print 'graph_title %s (%s)' % (graph_title, tube,)
        print 'graph_order ' + ' '.join(STATES)
        print 'graph_vlabel ' + graph_vlabel
        for state in STATES:
            print '%s.label %s' % (state, state,)
        print
    
def run():
    c = connect()
    tubes = c.list_tubes()['data']
    print_values(tubes, c)

def print_values(tubes, c):
    for tube in tubes:
        print 'multigraph job_count_' + tube
        stats = c.stats_tube(tube)['data']
        for state in STATES:
            key = 'current-jobs-' + state
            value = stats[key]
            print '%s.value %d' % (state, value,)
        print

def main():
    if len(sys.argv) > 1 and sys.argv[1] == "config":
        config()
    else:
        run()

if __name__ == "__main__":
    main()
