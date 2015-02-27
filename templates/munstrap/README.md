Munstrap
========

Alternative Munin 2.x templates based on Twitter Bootstrap 3

![Sample](sample.jpg)

Installation
------------

Get Munstrap:

```
cd /etc/munin
git clone https://github.com/jonnymccullagh/munstrap.git
```

Replace the Munin stock template by Munstrap (don't worry, there is a backup of the old files):

```
cp -rb munstrap/templates .
cp -rb munstrap/static .
```

Clean the old generated files:

```
rm -rf /var/www/munin/*
```

Take a coffee and wait some minutes for the html generation by Munin.

Upgrades
--------

Get the last changes from Munstrap:

```
cd /etc/munin/munstrap
git pull
```

Replace the old version by the new one:

```
cd /etc/munin
rm -rf templates static
cp -r munstrap/templates .
cp -r munstrap/static .
```

Revert to stock
---------------

```
cd /etc/munin
rm -rf templates static
mv templates~ templates
mv static~ static
```

Samples
-------

View of a specific node:
![Node view](sample-node.jpg)

Zoom feature:
![Zoom view](sample-zoom.jpg)