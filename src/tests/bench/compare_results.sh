#!/bin/sh

usage()
{
	echo "`basename $0` result_name field_name file1 [files]"
	echo
	echo "where"
	echo -e "\tresult_name - name of results to filter out (grep like)"
	echo -e "\tfield_name - either number of the field or max, min, avg, count"
	echo 
	echo "At least one file is expected. If the file is - then reads from standard input"
}

from_files()
{
	for i in $@
	do
		echo $i
		grep "$WHICH" $i | cut -d: -f$WHAT | cut -d= -f2
		echo ---
	done
}

from_input()
{
	grep "$WHICH" | cut -d: -f$WHAT | cut -d= -f2
}

if [ $# -lt 3 ]
then
	usage
fi

WHICH=$1
WHAT=$2
shift 2

case $WHAT in
	2|max ) WHAT=2 ;;
	3|min ) WHAT=3 ;;
	4|avg ) WHAT=4 ;;
	5|count) WHAT=5 ;;
	* ) echo "Bad value for field" >&2; exit 1 ;;
esac

if [ ! -f "$1" ]
then
	from_input
else
	from_files $@
fi
