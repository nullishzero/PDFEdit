#!/bin/sh
# This script simulates rmdir --ignore-fail-on-non-empty which is not 
# availeable on all systems
# It will fail only if at least one of given directories doesn't exist

fail()
{
	echo $1 >&2
	exit 1
}

for i in "$@"
do
	[ ! -d "$i" ] && fail "Directory \"$i\" not found"
	rmdir -p "$i" 2>/dev/null
done

exit 0
