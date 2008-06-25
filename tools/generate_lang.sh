#!/bin/sh

NAME=`basename $0`

function usage()
{
	echo
	echo "$NAME gui_directory [lang-gen]"
	echo "where"
	echo "gui_directory - is the directory for gui sources (with lang subdirectory)"
	echo "lang-gen - binary to be used for ts -> qm conversion (lrelease by default)"
	exit 1
}

if [ $# -lt 1 ]
then
	echo "Not enough parameters" >&2
	usage
fi

LANGDIR="$1/lang"

if [ ! -d "$LANGDIR" ]
then
	echo "No lang directory in $LANGDIR" >&2
	exit 1
fi

cd "$LANGDIR"
LANG_GEN="lrelease"
if [ $# -ge 2 ]
then
	LANG_GEN="$2"
fi

if [ ! -x "$LANG_GEN" ]
then
	echo "Language generator ($LANG_GEN) binary not found" >&2
	exit 1
fi

$LANG_GEN *.ts
exit 0
