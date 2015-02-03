ZNC-Logs
========

# Configuration
1. Make sure Python3 is installed and added to your path, this plugin will try to use `/usr/bin/python3`

2. Enable the log-plugin in znc (if you enable it for the complete instance, there will be some issues because this plugin only uses the network-name + channel-name, if there are some networks with the same name, it will count all lines together)

3. Add this to your `/etc/munin/plugin-conf.d/munin-node`  
```
[znc_logs]
user $your_znc_user
group $your_znc_group
env.logdir $path_to_znc_logs
```
`$your_znc_user` has to be an user, which can list `$path_to_znc_logs` and read all files in this directory.

4. Run `sudo wget -O /usr/share/munin/plugins/znc_logs https://raw.githubusercontent.com/munin-monitoring/contrib/master/plugins/znc/znc_logs.py` to download the `znc_logs.py` file from github to `/usr/share/munin/plugins/znc_logs` (of course you can use any directory you want)

5. Make the plugin-file executable `sudo chmod +x /usr/share/munin/plugins/znc_logs`

6. Enable the plugin with `sudo ln -s /usr/share/munin/plugins/znc_logs /etc/munin/plugins/`

7. Try the plugin with `sudo munin-run znc_logs`

7. Restart munin-node to load the plugin: `sudo service munin-node restart`

# FAQ
## What is ZNC?
ZNC is an IRC-Bouncer (a proxy to irc), further information: [znc.in](http://wiki.znc.in/ZNC)

## What does this plugin do?
This plugin counts the lines/minute in znc's log-files

# Contact
If you experience any issues, feel free to contact me at thor77[at]thor77.org