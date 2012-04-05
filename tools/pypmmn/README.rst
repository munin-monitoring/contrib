PyPMMN
======

PyPMMN is a pure python port of pmmn_. One small change. Instead of using the
current working dir as ``plugins`` folder, it will look for a *subdirectory*
called ``plugins`` in the current working folder.

Requirements
============

PyPMMN does not have any requirements other than the python standard library.
For compatibility, it's targeted for Python 2.4 and up.

Installation
============

The python way
--------------

Download the folder and run::

    python setup.py install

This will install ``pypmmn.py`` into your system's ``bin`` folder. Commonly,
this is ``/usr/local/bin``.

Manually
--------

Download the folder and copy the file ``pypmmn/pypmmn.py`` to a location of
your choice and ensure it's executable.


.. _pmmn: http://blog.pwkf.org/post/2008/11/04/A-Poor-Man-s-Munin-Node-to-Monitor-Hostile-UNIX-Servers

