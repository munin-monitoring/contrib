# munin-bukkit-plugins

This repository contains some useful [Munin](http://munin-monitoring.org/) plugins to monitor and observe a [Bukkit](http://bukkit.org) server:

* **mcjsonplayers** - players currently online
* **mcjsonramusage** - RAM usage
* **mcjsontps** - TPS (ticks per second)
* **mcsqls2killshostile** - hostile mob kills
* **mcsqls2killsneutral** - neutral mob kills
* **mcsqls2killspassive** - passive mob kills
* **mcsqls2players** - new players per day
* **mcsqlubshame** - kicks/bans/mutes/etc. per day

mcjson* requires [JSONAPI](https://github.com/alecgorge/jsonapi/)
mcsqls2* requires [Statistician](http://dev.bukkit.org/server-mods/statisticianv2/)
mcsqlub* requires [Ultrabans](http://dev.bukkit.org/server-mods/ultrabans/)

Read more in my [blog post](http://blog.frd.mn/munin-bukkit-plugins/).

## Requirements

* Web server with `PHP` support and Munin (2)
* Bukkit server with JSONAPI for the JSONAPI plugins (`mcjson*`)
* Bukkit server with Ultrabans for the Ultrabans plugins (`mcsqlub*`)
* Bukkit server with Statistician for the MySQL plugins  (`mcsqls2*`)
* MySQL server for the SQL plugins

## Configuration

1. Clone this repository: `git clone git@github.com:frdmn/munin-bukkit-plugins.git`
1. Adjust the JSONAPI variables in the mcjson* files
1. Adjust the MySQL variables in the mcsql* files
1. Make sure the `PHP` binary in the Shebang line is executable

## Installation

1. Perform your configuration (see above)
1. Move the plugins into the Munin plugin directory: `mv mc* /usr/share/munin/plugins/`
1. Change the ownership: `chown munin:munin /usr/share/munin/plugins/mc*`
1. Make sure they are exectuable: `chmod 755 /usr/share/munin/plugins/mc*`
1. Enable the plugins: `ln -s /usr/share/munin/plugins/mc* /etc/munin/plugins/`
1. Restart your munin-node: `service munin-node restart`
1. Run your cron: `su - munin --shell=/bin/sh -c /usr/bin/munin-cron`

## Alerts and limits?

To setup alerts and limits add the following lines in your specific node in the `munin.conf` file:

   [kotor.yeahwh.at]
      address 5.9.115.5
      [...]
      mctps_main.warning 19.9:      # Warning alert on < 19.9
      mctps_main.critical 19:		# Critical alert on < 19.0
      mcplayer_main.warning 20		# Warning alert when there are 20 players online
      mcplayer_main.critical 30		# Critical alert when there are more than 30 players online