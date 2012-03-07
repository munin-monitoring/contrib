#!/bin/bash
# [cassandra_nodes_in_cluster]
# env.config cassandra/nodes_in_cluster
# env.query org.apache.cassandra.*:*

if [ -z "$MUNIN_LIBDIR" ]; then
    MUNIN_LIBDIR="`dirname $(dirname "$0")`"
fi

if [ -f "$MUNIN_LIBDIR/plugins/plugin.sh" ]; then
    . $MUNIN_LIBDIR/plugins/plugin.sh
fi

if [ "$1" = "autoconf" ]; then
    echo yes
    exit 0
fi

if [ -z "$url" ]; then
  # this is very common so make it a default
  url="service:jmx:rmi:///jndi/rmi://127.0.0.1:8080/jmxrmi"
fi

if [ -z "$config" -o -z "$query" -o -z "$url" ]; then
  echo "Configuration needs attributes config, query and optinally url"
  exit 1
fi

JMX2MUNIN_DIR="$MUNIN_LIBDIR/plugins"
CONFIG="$JMX2MUNIN_DIR/jmx2munin.cfg/$config"

if [ "$1" = "config" ]; then
    cat "$CONFIG"
    exit 0
fi

JAR="$JMX2MUNIN_DIR/jmx2munin.jar"
CACHED="/tmp/jmx2munin"

if test ! -f $CACHED || test `find "$CACHED" -mmin +2`; then

    java -jar "$JAR" \
      -url "$url" \
      -query "$query" \
      $ATTRIBUTES \
      > $CACHED

    echo "cached.value `date +%s`" >> $CACHED
fi

ATTRIBUTES=`awk '/\.label/ { gsub(/\.label/,""); print $1 }' $CONFIG`

for ATTRIBUTE in $ATTRIBUTES; do
  grep $ATTRIBUTE $CACHED
done