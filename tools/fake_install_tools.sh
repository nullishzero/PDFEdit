#!/bin/sh

LOGFILE=/tmp/fake_install_tools.log

function usage()
{
	echo -e "\
	`basename $0` command [params] \n\
	where\n\
	\tcommand - name of the command to emulate\n\
	\t[params] - parameters for command"
	exit 1
}

if [ $# -lt 1 ]
then
	echo Command expected
	usage
fi

CMD=$1
shift

echo -n $CMD: >> "$LOGFILE"

# Some command should contain additional info
case $CMD in
	# Get current directory to know where we are erasing
	DEL_* ) echo -n "`pwd`:" >> "$LOGFILE" ;;
esac

echo $* >> "$LOGFILE"
exit 0
