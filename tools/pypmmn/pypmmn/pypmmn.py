#!/usr/bin/env python
"""
A very simple munin-node written in pure python (no external libraries
required)
"""
from datetime import datetime
from logging.handlers import RotatingFileHandler
from optparse import OptionParser
from os import listdir, access, X_OK, getpid
from os.path import join, isdir, abspath, dirname, exists
from subprocess import Popen, PIPE
from time import sleep
import logging
import socket

import sys

LOG = logging.getLogger(__name__)
LOG_FORMAT = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
SESSION_TIMEOUT = 10 # Amount of seconds until an unused session is closed

from daemon import createDaemon


__version__ = '1.0b1'


class CmdHandler(object):
    """
    This handler defines the protocol between munin and this munin node.
    Each method starting with ``do_`` responds to the corresponding munin
    command.
    """

    def __init__(self, get_fun, put_fun, options):
        """
        Constructor

        :param get_fun: The function used to receive a message from munin
        :param put_fun: The function used to send a message back to munin
        :param options: The command-line options object
        """
        self.get_fun = get_fun
        self.put_fun = put_fun
        self.options = options

    def do_version(self, arg):
        """
        Prints the version of this instance.
        """
        LOG.debug('Command "version" executed with args: %r' % arg)
        self.put_fun('# munin node at %s\n' % (
            self.options.host,
            ))

    def do_nodes(self, arg):
        """
        Prints this hostname
        """
        LOG.debug('Command "nodes" executed with args: %r' % arg)
        self.put_fun('%s\n' % self.options.host)
        self.put_fun('.\n')

    def do_quit(self, arg):
        """
        Stops this process
        """
        LOG.debug('Command "quit" executed with args: %r' % arg)
        sys.exit(0)

    def do_list(self, arg):
        """
        Print a list of plugins
        """
        LOG.debug('Command "list" executed with args: %r' % arg)
        try:
            LOG.debug('Listing files inside %s' % self.options.plugin_dir)
            for filename in listdir(self.options.plugin_dir):
                if not access(join(self.options.plugin_dir, filename), X_OK):
                    LOG.warning('Non-executable plugin %s found!' % filename)
                    continue
                LOG.debug('Found plugin: %s' % filename)
                self.put_fun("%s " % filename)
        except OSError, exc:
            self.put_fun("# ERROR: %s" % exc)
        self.put_fun("\n")

    def _caf(self, plugin, cmd):
        """
        handler for ``config``, ``alert`` and ``fetch``
        Calls the plugin with ``cmd`` as only argument.

        :param plugin: The plugin name
        :param cmd: The command which is to passed to the plugin
        """
        plugin_filename = join(self.options.plugin_dir, plugin)

        # Sanity checks
        if isdir(plugin_filename) or not access(plugin_filename, X_OK):
            msg = "# Unknown plugin [%s] for %s" % (plugin, cmd)
            LOG.warning(msg)
            self.put_fun(msg)
            return

        # for 'fetch' we don't need to pass a command to the plugin
        if cmd == 'fetch':
            plugin_arg = ''
        else:
            plugin_arg = cmd

        try:
            cmd = [plugin_filename, plugin_arg]
            LOG.debug('Executing %r' % cmd)
            output = Popen(cmd, stdout=PIPE).communicate()[0]
        except OSError, exc:
            LOG.exception("Unable to execute the command %r" % cmd)
            self.put_fun("# ERROR: %s\n" % exc)
            return
        self.put_fun(output)
        self.put_fun('.\n')

    def do_alert(self, arg):
        """
        Handle command "alert"
        """
        LOG.debug('Command "alert" executed with args: %r' % arg)
        self._caf(arg, 'alert')

    def do_fetch(self, arg):
        """
        Handles command "fetch"
        """
        LOG.debug('Command "fetch" executed with args: %r' % arg)
        self._caf(arg, 'fetch')

    def do_config(self, arg):
        """
        Handles command "config"
        """
        LOG.debug('Command "config" executed with args: %r' % arg)
        self._caf(arg, 'config')

    def do_cap(self, arg):
        """
        Handles command "cap"
        """
        LOG.debug('Command "cap" executed with args: %r' % arg)
        self.put_fun("cap ")
        if self.options.spoolfetch_dir:
            self.put_fun("spool")
        else:
            LOG.debug('No spoolfetch_dir specified. Result spooling disabled')

        self.put_fun("\n")

    def do_spoolfetch(self, arg):
        """
        Handles command "spoolfetch"
        """
        LOG.debug('Command "spellfetch" executed with args: %r' % arg)
        output = Popen(['%s/spoolfetch_%s' % (self.options.spoolfetch_dir,
            self.options.host),
            arg]).communicate()[0]
        self.put_fun(output)
        self.put_fun('.\n')

    # aliases
    do_exit = do_quit

    def handle_input(self, line):
        """
        Handles one input line and sends any result back using ``put_fun``
        """
        line = line.strip()
        line = line.split(' ')
        cmd = line[0]
        if len(line) == 1:
            arg = ''
        elif len(line) == 2:
            arg = line[1]
        else:
            self.put_fun('# Invalid input: %s\n' % line)
            return

        if not cmd:
            return

        func = getattr(self, 'do_%s' % cmd, None)
        if not func:
            # Give the client a list of supported commands.
            commands = [_[3:] for _ in dir(self) if _.startswith('do_')]
            self.put_fun("# Unknown command. Supported commands: %s\n" % (
                commands))
            return

        func(arg)

    def is_timed_out(self):
        return (datetime.now() - self._last_command).seconds > SESSION_TIMEOUT

    def reset_time(self):
        self._last_command = datetime.now()


def usage(option, opt, value, parser):
    """
    Prints the command usage and exits
    """
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
                ' Default: %s' % socket.gethostname()),
            default=socket.gethostname())
    parser.add_option('-n', '--no-daemon', dest='no_daemon',
            default=False,
            action='store_true',
            help='Run in foreground. Do not daemonize. '
               'Will also enable debug logging to stdout.')
    parser.add_option('-l', '--log-dir', dest='log_dir',
            default=None,
            help='The log folder. Default: disabled')
    parser.add_option('-s', '--spoolfech-dir', dest='spoolfetch_dir',
            default=None,
            help='The spoolfetch folder. Default: disabled')
    parser.add_option('--help', action='callback', callback=usage,
            help='Shows this help')

    options, args = parser.parse_args()

    # ensure we are using absolute paths (for daemonizing)
    if options.log_dir:
        options.log_dir = abspath(options.log_dir)

    if options.spoolfetch_dir:
        options.spoolfetch_dir = abspath(options.spoolfetch_dir)

    if options.plugin_dir:
        options.plugin_dir = abspath(options.plugin_dir)

    return (options, args)


def process_stdin(options):
    """
    Process commands by reading from stdin
    """
    rfhandler = RotatingFileHandler(
        join(abspath(dirname(__file__)), 'log', 'pypmmn.log'),
        maxBytes=100 * 1024,
        backupCount=5
        )
    rfhandler.setFormatter(logging.Formatter(LOG_FORMAT))
    logging.getLogger().addHandler(rfhandler)
    handler = CmdHandler(sys.stdin.read, sys.stdout.write, options)
    handler.do_version(None)
    LOG.info('STDIN handler opened')
    while True:
        data = sys.stdin.readline().strip()
        if not data:
            return
        handler.handle_input(data)


def process_socket(options):
    """
    Process socket connections.

    .. note::

        This is not a multithreaded process. So only one connection can be
        handled at any given time. But given the nature of munin, this is Good
        Enough.
    """

    retcode = 0
    if options.no_daemon:
        # set up on-screen-logging
        console_handler = logging.StreamHandler(sys.stdout)
        console_handler.setFormatter(logging.Formatter(LOG_FORMAT))
        logging.getLogger().addHandler(console_handler)
    else:
        # fork fork
        retcode = createDaemon()

        # set up a rotating file log
        rfhandler = RotatingFileHandler(
            join(options.log_dir, 'daemon.log'),
            maxBytes=100 * 1024,
            backupCount=5
            )
        rfhandler.setFormatter(logging.Formatter(LOG_FORMAT))
        logging.getLogger().addHandler(rfhandler)

        # write down some house-keeping information
        LOG.info('New process PID: %d' % getpid())
        pidfile = open(join(options.log_dir, 'pypmmn.pid'), 'w')
        pidfile.write(str(getpid()))
        pidfile.close()
        LOG.info('PID file created in %s' % join(options.log_dir,
            'pypmmn.pid'))

    LOG.info('Socket handler started.')

    host = '' # listens on all addresses TODO: make this configurable
    port = int(options.port)

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((host, port))
    s.listen(1)

    LOG.info('Listening on host %r, port %r' % (host, port))

    conn, addr = s.accept()
    handler = CmdHandler(conn.recv, conn.send, options)
    handler.do_version(None)
    handler.reset_time()

    LOG.info("Accepting incoming connection from %s" % (addr, ))
    while True:
        data = conn.recv(1024)
        if not data.strip():
            sleep(1)
            if handler.is_timed_out():
                LOG.info('Session timeout.')
                conn.shutdown(socket.SHUT_RDWR)
                conn.close()

                LOG.info('Listening on host %r, port %r' % (host, port))

                conn, addr = s.accept()
                handler.reset_time()
                handler.get_fun = conn.recv
                handler.put_fun = conn.send
                handler.do_version(None)

                LOG.info("Accepting incoming connection from %s" % (addr, ))
            try:
                data = conn.recv(1024)
            except socket.error, exc:
                LOG.warning("Socket error. Reinitialising.: %s" % exc)
                conn, addr = s.accept()
                handler.reset_time()
                handler.get_fun = conn.recv
                handler.put_fun = conn.send
                handler.do_version(None)

                LOG.info("Accepting incoming connection from %s" % (addr, ))

        if data.strip() == 'quit':
            LOG.info('Client requested session end. Closing connection.')
            conn.shutdown(socket.SHUT_RDWR)
            conn.close()

            LOG.info('Listening on host %r, port %r' % (host, port))

            conn, addr = s.accept()
            handler.reset_time()
            handler.get_fun = conn.recv
            handler.put_fun = conn.send
            handler.do_version(None)

            LOG.info("Accepting incoming connection from %s" % (addr, ))

            continue

        handler.handle_input(data)

    sys.exit(retcode)


def main():
    """
    The main entry point of the application
    """
    options, args = get_options()

    # Handle logging as early as possible.
    if options.log_dir:
        if not exists(options.log_dir):
            raise IOError('[Errno 2] No such file or directory: %r' % (
                options.log_dir))
        # set up logging if requested
        root_logger = logging.getLogger()
        root_logger.setLevel(logging.NOTSET)  # TODO: Make configurable

    # Start either the "stdin" interface, or the socked daemon. Depending on
    # whether a port was given on startup or not.
    if not options.port:
        process_stdin(options)
    else:
        process_socket(options)


if __name__ == '__main__':
    main()
