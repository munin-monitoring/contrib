#!/usr/bin/python
from optparse import OptionParser
from os import listdir, access, X_OK
from os.path import join, isdir
from subprocess import Popen, PIPE
from socket import gethostname, SHUT_RDWR
from time import sleep

import sys


__version__ = '1.0dev3'


class CmdHandler(object):

    def __init__(self, get_fun, put_fun, options):
        self.get_fun = get_fun
        self.put_fun = put_fun
        self.options = options

    def do_version(self, arg):
        """
        Prints the version of this instance.
        """
        self.put_fun('# munin node at %s\n' % (
            self.options.host,
            ))

    def do_nodes(self, arg):
        """
        Prints this hostname
        """
        self.put_fun('%s\n' % self.options.host)
        self.put_fun('.')

    def do_quit(self, arg):
        """
        Stops this process
        """
        sys.exit(0)

    def do_list(self, arg):
        """
        Print a list of plugins
        """
        try:
            for filename in listdir(self.options.plugin_dir):
                if not access(join(self.options.plugin_dir, filename), X_OK):
                    continue
                self.put_fun("%s " % filename)
        except OSError, exc:
            sys.stdout.write("# ERROR: %s" % exc)
        self.put_fun("\n")

    def _caf(self, arg, cmd):
        """
        handler for ``config``, ``alert`` and ``fetch``
        Calls the plugin with ``cmd`` as only argument.
        """
        plugin_filename = join(self.options.plugin_dir, arg)
        if isdir(plugin_filename) or not access(plugin_filename, X_OK):
            self.put_fun("# Unknown plugin [%s] for %s" % (arg, cmd))
            return

        if cmd == 'fetch':
            arg_plugin = ''
        else:
            arg_plugin = cmd

        try:
            output = Popen([plugin_filename, arg_plugin], stdout=PIPE).communicate()[0]
        except OSError, exc:
            self.put_fun("# ERROR: %s" % exc)
            return
        self.put_fun(output)
        self.put_fun('.\n')

    def do_alert(self, arg):
        self._caf(arg, 'alert')

    def do_fetch(self, arg):
        self._caf(arg, 'fetch')

    def do_config(self, arg):
        self._caf(arg, 'config')

    def do_cap(self, arg):
        self.put_fun("cap ")
        if self.options.spoolfetch_dir:
            self.put_fun("spool")
        self.put_fun("cap \n")

    def do_spoolfetch(self, arg):
        output = Popen(['%s/spoolfetch_%s' % (self.options.spoolfetch_dir,
            self.options.host),
            arg]).communicate()[0]
        self.put_fun(output)
        self.put_fun('.\n')

    # aliases
    do_exit = do_quit


    def handle_input(self, line):
        line = line.strip()
        line = line.split(' ')
        cmd = line[0]
        if len(line) == 1:
            arg = ''
        elif len(line) == 2:
            arg = line[1]
        else:
            raise ValueError('Invalid input: %s' % line)

        if not cmd:
            return

        func = getattr(self, 'do_%s' % cmd, None)
        if not func:
            commands = [_[3:] for _ in dir(self) if _.startswith('do_')]
            self.put_fun("# Unknown command. Supported commands: %s" % commands)
            return

        func(arg)


def usage(option, opt, value, parser):
    parser.print_help()
    sys.exit(0)


def get_options():
    """
    Parses command-line arguments.
    """
    parser = OptionParser(add_help_option=False)
    parser.add_option('-p', '--port', dest='port',
            default=None,
            help='TCP Port to listen on. (If not specified, use stdin/stdout)')
    parser.add_option('-d', '--plugin-dir', dest='plugin_dir',
            default='plugins',
            help=('The directory containing the munin-plugins.'
                ' Default: <current working dir>/plugins'))
    parser.add_option('-h', '--host', dest='host',
            help=('The hostname which will be reported in the plugins.'
                ' Default: %s' % gethostname()),
            default=gethostname())
    parser.add_option('-s', '--spoolfech-dir', dest='spoolfetch_dir',
            default=None,
            help='The spoolfetch folder. Default: disabled')
    parser.add_option('--help', action='callback', callback=usage,
            help='Shows this help')
    return parser.parse_args()


def main():
    options, args = get_options()
    handler = CmdHandler(None, None, options)
    if not options.port:
        handler.get_fun = sys.stdin.read
        handler.put_fun = sys.stdout.write
    else:
        import socket
        host = ''
        port = int(options.port)
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((host, port))
        s.listen(1)

        conn, addr = s.accept()
        handler.get_fun = conn.recv
        handler.put_fun = conn.send
        handler.do_version(None)
        counter = 0

        print 'Connected by', addr
        while True:
            data = conn.recv(1024)
            if not data.strip():
                sleep(1)
                counter += 1
                if counter > 3:
                    conn.shutdown(SHUT_RDWR)
                    conn.close()
                    conn, addr = s.accept()
                    counter = 0
                    handler.get_fun = conn.recv
                    handler.put_fun = conn.send
                    handler.do_version(None)
                print "sleep"
                try:
                    data = conn.recv(1024)
                    print 'data2', `data`
                except socket.error, exc:
                    conn, addr = s.accept()
                    counter = 0
                    handler.get_fun = conn.recv
                    handler.put_fun = conn.send
                    handler.do_version(None)
                    print "Socket error: %s" % exc

            if data.strip() == 'quit':
                print 'shutting down remote connection'
                conn.shutdown(SHUT_RDWR)
                conn.close()
                conn, addr = s.accept()
                counter = 0
                handler.get_fun = conn.recv
                handler.put_fun = conn.send
                handler.do_version(None)
                continue

            handler.handle_input(data)


if __name__ == '__main__':
    main()
