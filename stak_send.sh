#!/bin/sh

cmd=$(nc -u -c 127.0.0.1 2000)

if [ $# = 1 ]; then
	if [ "$1" = 'QUIT' ]; then
		echo '' | $cmd
	else
		printf '%s\n' "$1" | $cmd
	fi
else
	$cmd
fi
