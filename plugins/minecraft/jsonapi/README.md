# munin-bukkit-plugins

This repository contains some useful [Munin](http://munin-monitoring.org/) plugins to monitor and observe a [Bukkit](http://bukkit.org) server.

Read more in my [blog post](http://).

## Requirements

* Bukkit server with [JSONAPI](https://github.com/alecgorge/jsonapi) plugin
* Web server with Munin (2) and `PHP` support

## Configuration

1. Adjust the JSONAPI variables in each plugin
1. Make sure the `PHP` binary in the Shebang line is executable

## Installation

1. Clone this repository: `git clone git@github.com:frdmn/munin-bukkit-plugins.git`
1. Perform your configuration (see above)
1. Move the plugins into the Munin plugin directory: `mv mcjson* /usr/share/munin/plugins/`
1. Change the ownership: `chown munin:munin /usr/share/munin/plugins/mcjson*`
1. Make sure they are exectuable: `chmod 755 /usr/share/munin/plugins/mcjson*`
1. Enable the plugins: `ln -s /usr/share/munin/plugins/mcjson* /etc/munin/plugins/`
1. Restart your munin-node: `service munin-node restart`
1. Run your cron:

    su - munin --shell=/bin/sh
    /usr/bin/munin-cron