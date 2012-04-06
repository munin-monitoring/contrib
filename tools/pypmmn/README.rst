PyPMMN
======

PyPMMN is a pure python port of pmmn_. One small change: Instead of using the
current working dir as ``plugins`` folder, it will look for a *subdirectory*
called ``plugins`` in the current working folder. This value can be overridden
by a command-line parameter!

Requirements
============

PyPMMN does not have any requirements other than the python standard library.
For compatibility, it's targeted for Python 2.4 and up.

Known Issues
============

* The stdin mode does not work correctly. Consider using the original pmmn_
  instead.
* It's not multithreaded. Only one connection is handled at a time. But given
  the nature of munin, this should not be an issue.

Installation
============

The python way
--------------

Download the folder and run::

    python setup.py install

This will install ``pypmmn.py`` into your system's ``bin`` folder. Commonly,
this is ``/usr/local/bin``.

And of course, you can use virtual environments too!

Manually
--------

Download the folder and copy both files ``pypmmn/pypmmn.py`` and
``pypmmn/daemon.py`` to a location of your choice and ensure ``pypmmn.py`` is
executable.

Usage
=====

All command-line parameters are documented. Simply run::

    pypmmn.py --help

to get more information.

Daemon mode
-----------

In daemon mode, it's very helpful to specify a log folder. It gives you a
means to inspect what's happening. In the case you specified a log folder,
pypmmn will also create a file called ``pypmmn.pid`` containing the PID of the
daemon for convenience.


.. _pmmn: http://blog.pwkf.org/post/2008/11/04/A-Poor-Man-s-Munin-Node-to-Monitor-Hostile-UNIX-Servers

