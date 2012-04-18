# jmx2munin

The [jmx2munin](http://github.com/tcurdt/jmx2munin) project exposes JMX MBean attributes to [Munin](http://munin-monitoring.org/).
Some of it's features:

 * strictly complies to the plugin format
 * exposes composite types like Lists, Maps, Set as useful as possible
 * String values can be mapped to numbers

# How to use

This is what the Munin script will call. So you should test this first. Of course with your parameters. This example expose all Cassandra information to Munin.

    java -jar jmx2munin.jar \
         -url service:jmx:rmi:///jndi/rmi://localhost:8080/jmxrmi \
         -query "org.apache.cassandra.*:*"

The "url" parameters specifies the JMX URL, the query selects the MBeans (and optionally also the attributes) to expose.

    java -jar jmx2munin.jar \
         -url service:jmx:rmi:///jndi/rmi://localhost:8080/jmxrmi \
         -query "org.apache.cassandra.*:*" \
         -attribute org_apache_cassandra_db_storageservice_livenodes_size

The script that does the actual interaction with munin you can find in the contrib section. It's the one you should link in the your Munin plugin directory.

    :/etc/munin/plugins$ ls -la cassandra_*
    lrwxrwxrwx 1 root root 37 2011-04-07 19:58 cassandra_nodes_in_cluster -> /usr/share/munin/plugins/jmx2munin.sh

In the plugin conf you point to the correct configuration

    [cassandra_*]
    env.query org.apache.cassandra.*:*

    [cassandra_nodes_in_cluster]
    env.config cassandra/nodes_in_cluster

A possible configuration could look like this

    graph_title Number of Nodes in Cluster
    graph_vlabel org_apache_cassandra_db_storageservice_livenodes_size
    org_apache_cassandra_db_storageservice_livenodes_size.label number of nodes

The script will extract the attributes from the config and caches the JMX results to reduce the load when showing many values.

# More advanced

Sometimes it can be useful to track String values by mapping them into an enum as they really describe states. To find this possible candidates you can call:

    java -jar jmx2munin.jar \
         -url service:jmx:rmi:///jndi/rmi://localhost:8080/jmxrmi \
         -query "org.apache.cassandra.*:*" \
         list

It should output a list of possible candidates. This can now be turned into a enum configuration file:

    [org.apache.cassandra.db.StorageService:OperationMode]
    0 = ^Normal
    1 = ^Client
    2 = ^Joining
    3 = ^Bootstrapping
    4 = ^Leaving
    5 = ^Decommissioned
    6 = ^Starting drain
    7 = ^Node is drained

Which you then can provide:

    java -jar jmx2munin.jar \
         -url service:jmx:rmi:///jndi/rmi://localhost:8080/jmxrmi \
         -query "org.apache.cassandra.*:*" \
         -enums /path/to/enums.cfg

Now matching values get replaced by their numerical representation. On the left needs to be a unique number on the right side is a regular expression. If a string cannot be matched according to the spec "U" for "undefined" will be returned.

# License

Licensed under the Apache License, Version 2.0 (the "License")
You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
