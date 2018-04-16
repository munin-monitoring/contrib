munin-debian-packages
=====================

## Munin Debian Plugin

With this plugin munin can give you a nice graph and some details where your
packages come from, how old or new your installation is. Furtermore it tells
you how many updates you should have been installed, how many packages are
outdated and where they come from.

![A week of upgradable packages](/munin-monitoring/contrib/raw/master/plugins/apt/deb_packages/example/packages_label_archive_upgradable-week.png)

You can sort installed or upgradable Packages by 'archive', 'origin', 'site',
'label' and 'component' and even some of them at once.

The script uses caching cause it is quite expensive. It saves the output to a
cachefile and checks on each run, if dpkg-status or downloaded Packagefile have
changed. If one of them has changed, it runs, if not it gives you the cached
version.

### Installation

This plugin has checked on Debian - Wheezy and squeeze. If you want to use it
on older installations, tell me whether it works or which errors you had. It
should run past python-apt 0.7 and python 2.5.

check out this git repository from 
   
    aptitude install python-apt
    git clone git://github.com/munin-monitoring/contrib.git
    cd contrib/plugins/apt/deb_packages
    sudo cp deb_packages.py /etc/munin/plugins/deb_packages
    sudo cp deb_packages.munin.conf /etc/munin/plugin-conf.d/deb_packages

Verify the installation by

    sudo munin-run deb_packages

### Configuration
If you copied deb_packages.munin.conf to plugin-conf.d you have a starting point.
A typical configuration looks like this

    [deb_packages]
    # plugin is quite expensive and has to write statistics to cache output
    # so it has to write to plugins.cache
    user munin

    # Packagelists to this size are printed as extra information to munin.extinfo
    env.MAX_LIST_SIZE_EXT_INFO 50

    # Age in seconds an $CACHE_FILE can be. If it is older, the script updates
    # default if not set is 3540 (one hour)
    # at the moment this is not used, the plugin always runs (if munin calls it)
    #  
    env.CACHE_FILE_MAX_AGE 3540

    # All these numbers are only for sorting, so you can use env.graph01_sort_by_0
    # and env.graph01_sort_by_2 without using env.graph01_sort_by_1.
    # sort_by values ...
    # possible values are 'label', 'archive', 'origin', 'site', 'component'
    env.graph00_type installed
    env.graph00_sort_by_0 label
    env.graph00_sort_by_1 archive
    env.graph00_show_ext_0 origin
    env.graph00_show_ext_1 site

    env.graph01_type upgradable
    env.graph01_sort_by_0 label
    env.graph01_sort_by_1 archive
    env.graph01_show_ext_0 origin
    env.graph01_show_ext_1 site

You can sort_by one or some of these possible Values
