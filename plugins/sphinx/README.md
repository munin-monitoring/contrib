This plugin shows graphs of numbers of documents in Sphinxsearch indexes.

## Requirements
This plugin requires pythons sphinxsearch module which can be installed via easy_install.

## Installation
Copy file to directory /usr/share/munin/pligins/ and create symbolic links for each index you wish to monitor.
For example, if you've got indexex called index1 and index2 create these symlinks:

    ln -s /usr/share/munin/plugins/sphindex_ /etc/munin/plugins/sphindex_index1
    ln -s /usr/share/munin/plugins/sphindex_ /etc/munin/plugins/sphindex_index2

If you run munin-node at different box than Sphinxsearch you can specify hostname and port options in munin-node.conf:

    [sphindex_*]
    env.server 10.216.0.141
    env.port 9312


