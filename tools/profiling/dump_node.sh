#! /bin/sh

NODE=${1:-localhost}
PORT=${2:-4949}

{
	for plugin in $(echo "list" | nc $NODE $PORT | tail -n 1)
	do
		echo "config $plugin"
		echo "fetch $plugin"
	done 
	echo "quit"
} | nc $NODE $PORT
