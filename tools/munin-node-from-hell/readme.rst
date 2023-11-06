munin-node from hell
====================

This is a simple implementation of a munin node (http://munin-monitoring.org/)
that is made to give the polling server a hard time.

In practice this is the munin-node that we use to develop and test the awesome
stuff we do at http://hostedmunin.com/ . Use it as you feel fit :)

Current features controlled via config file:

* Respond slowly or never to queries.
* Have plugins that always are in warning or critical.
* Extensive number of plugins running at once.
* Run on multiple ports at the same time, to test huge amounts of clients.


Usage
-----

munin-node-from-hell takes two arguments; the mode and which config file to
use. Mode is either --run or --muninconf.

This software is meant to run as an ordinary Unix user, please don't run
it as root.

You probably want:

	./munin-node-from-hell --run simple.conf

To make a config snippet to put in munin.conf:

	./munin-node-from-hell --muninconf simple.conf > snippet.conf

License
-------

See the file MIT-LICENSE for details.

Contact
-------

Lasse Karstensen <lasse.karstensen@gmail.com>
