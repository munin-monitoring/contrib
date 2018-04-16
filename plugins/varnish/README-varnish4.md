Munin Plugin for Varnish 4
==========================

Modified version of the plugin for Varnish 2/3 from the munin distribution
to adapt to the XML output of Varnish 4 varnishstat.

Installation
------------

- Copy `varnish4_` to the Munin plugins directory (`/usr/share/munin/plugins/`)
- Make `varnish4_` runnable (`chmod +x /usr/share/munin/plugins/varnish4_`)

Configuration
-------------

Run `munin-node-configure --shell` and link the aspects to monitor to
your actual plugins directory.

In your plugins.conf add
```
[varnish4_*]
     group varnish
     env.varnishstat varnishstat
     env.name
```
`group varnish` Since Varnish version 4.1, Varnish shared log utilities must be run in a context with *varnish* group membership.

`env.varnishstat` can be a full path to varnishstat if it's
not in the path already.

`env.name` is blank (undefined) by default and can be used to specify a -n
name argument to varnish if multiple instances are running on the same
server. 
