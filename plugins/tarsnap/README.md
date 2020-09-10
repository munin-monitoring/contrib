Munin plugin for Tarsnap
========================

https://github.com/warrenguy/munin-tarsnap

This plugin creates two graphs:

* *tarsnap_total* - summarising the total amount of data the local tarsnap
instance has stored on the service (total and compressed).
* *tarsnap_unique* - summarising the total amount of unique (deduplicated)
data the local tarsnap instance has stored on the service (total and
compressed). The compressed value here is the actual amount of data stored
on the tarnap servers and what tarsnap uses for billing.

Usage
-----

Add the following to your backup script (after tarsnap has run), or to a
cron job:

    /usr/local/bin/tarsnap --print-stats > /path/to/tarsnap-stats.txt

N.B.: ensure `/path/to/munin-stats.txt` is readable by munin-node.

Configuration
-------------

Define the path to the stats file created above in your munin-node
configuration:

    [tarsnap]
    env.STATSFILE /path/to/tarsnap-stats.txt

The default value is `/var/lib/munin/tarsnap-stats.txt`.

Author
------

Warren Guy <warren@guy.net.au>

https://warrenguy.me

Copyright
---------

Copyright (C) 2014 Warren Guy <warren@guy.net.au>
