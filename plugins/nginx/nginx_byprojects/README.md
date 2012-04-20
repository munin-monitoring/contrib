# The 'byprojects' family
Those plugins are used to monitor different projects or vhost (i.e. either different log files or uing regular expression as filters) on the same web server.

## munin_byprojects_access
Count the number of hits per projects/vhost.  
![byproject_access](https://www.mantor.org/~northox/misc/munin-plugins/nginx_byprojects_access1-month.png "byproject_access")

## munin_byprojects_bandwidth
Count the total bandwidth used by each projects/vhost. [Logtail] (https://www.fourmilab.ch/webtools/logtail/) is required.
![byproject_bandwidth](https://www.mantor.org/~northox/misc/munin-plugins/apache_byprojects_bandwidth-month.png "byproject_bandwidth")

## munin_byprojects_inout_bandwidth
Counts the in/out bandwidth used by each projects/vhost. [Logtail] (https://www.fourmilab.ch/webtools/logtail/) is required.
![byproject_inout_bandwidth](https://www.mantor.org/~northox/misc/munin-plugins/apache_byprojects_inout_bandwidth-month.png "byproject_inout_bandwidth")

## Installation
Installation is pretty straight forward. First you need to configure the plugin:

Identify the file which will be used by logtail to identify it's position in the log and the path to logtail:

      $statepath = '/usr/local/var/munin/plugin-state'; # directory where logtail will save the state
      $logtail = '/usr/local/bin/logtail';

Multiple logs can be used for the same project/vhost and a regular expression (regex) can be used as a filter:

      %logs = (
      'prod' => ('/home/prod/log/access.log'),
      'test' => (
                  ('/var/log/httpd/access.log', '"[A-Z]+ /test/'),
                  '/home/test/log/access.log'
                )
      );

In the previous example the prod project graph will be using everything in /home/prod/log/access.log. The test project will be using eveything in /home/test/log/access.log and stuff that match '"[A-Z] /test/' in /var/log/httpd/access.log (e.g. "GET /test/).

Then link the file just as any other plugins.
