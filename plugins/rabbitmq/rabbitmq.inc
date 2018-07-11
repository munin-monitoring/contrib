CMD=$(which rabbitmqctl)

if [ "$1" = "autoconf" ]; then
    test -x "$CMD" && echo yes || echo "no ('$CMD' not executable)"
	exit 0
fi

if [ "$1" = "suggest" ]; then
    for h in $(HOME=$HOME sudo $CMD list_vhosts -q); do 
        vhost=$(echo $h | sed -e 's#/##')
        if [ -z "$vhost" ] ; then
            vhost="_root"
        fi
        echo $vhost
    done
    exit 0
fi

# If run with the "config"-parameter, give out information on how the
# graphs should look. 

HOME=/tmp/
SELF=$(basename $0)
PREFIX=$(basename $(readlink -f "$0"))
VHOST=${SELF##$PREFIX}

if [ "$VHOST" = "_root" ] ; then
    VHOST="/"
fi

VHOST=${vhost:-$VHOST}
IGNORE=${ignore:-'\A(?!x)x'}

