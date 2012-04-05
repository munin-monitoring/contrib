#!/usr/bin/python
from os.path import join, isdir
from subprocess import call
from os import listdir, access, X_OK
import sys


__version__ = '1.0dev1'


spoolfetch_dir = ''
plugin_dir = 'plugins'
host = 'hostname'


class CmdHandler(object):

    def __init__(self, out_stream):
        self.out_stream = out_stream

    def do_version(self, arg):
        """
        Prints the version of this instance.
        """
        self.out_stream.write('pypmmn on %s version: %s' % (host,
            __version__))

    def do_nodes(self, arg):
        """
        Prints this hostname
        """
        self.out_stream.write('%s\n' % host)
        self.out_stream.write('.')

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
            for filename in listdir(plugin_dir):
                if not access(join(plugin_dir, filename), X_OK):
                    continue
                self.out_stream.write("%s " % filename)
        except OSError, exc:
            sys.stdout.writte("# ERROR: %s" % exc)

    def _caf(self, arg, cmd):
        """
        handler for ``config``, ``alert`` and ``fetch``
        Calls the plugin with ``cmd`` as only argument.
        """
        plugin_filename = join(plugin_dir, arg)
        if isdir(plugin_filename) or not access(plugin_filename, X_OK):
            self.out_stream.write("# Unknown plugin [%s] for %s" % (arg, cmd))
            return

        if cmd == 'fetch':
            arg_plugin = ''
        else:
            arg_plugin = cmd

        try:
            call([plugin_filename, arg_plugin])
        except OSError, exc:
            self.out_stream.write("# ERROR: %s" % exc)
            return
        self.out_stream.write('.')

    def do_alert(self, arg):
        self._caf(arg, 'alert')

    def do_fetch(self, arg):
        self._caf(arg, 'fetch')

    def do_config(self, arg):
        self._caf(arg, 'config')

    def do_cap(self, arg):
        self.out_stream.write("cap ")
        if spoolfetch_dir:
            self.out_stream.write("spool ")

    def do_spoolfetch(self, arg):
        call(['%s/spoolfetch_%s' % (spoolfetch_dir, host), arg])
        self.out_stream.write('.')

    # aliases
    do_exit = do_quit


def handle_input(in_stream, handler, out_stream):
    for line in in_stream:
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
            continue

        func = getattr(handler, 'do_%s' % cmd, None)
        if not func:
            commands = [_[3:] for _ in dir(handler) if _.startswith('do_')]
            print "# Unknown command. Supported commands: %s" % commands
            sys.exit(1)

        func(arg)
    out_stream.write('\n')


def main():
    handle_input(sys.stdin, CmdHandler(sys.stdout), sys.stdout)

if __name__ == '__main__':
    main()
