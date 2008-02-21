#!/bin/bash

PROG_NAME=`basename $0`

usage()
{
	echo "$PROG_NAME usage:"
	echo -e "\t$PROG_NAME patch_file [dir]* [-dry]"
	echo -e "\n\tpatch_file\tpatch to apply. Note that patch file has to be without file header (contains only hunks)"
	echo -e "\tdir\tdirectory to process (multiple are possible). Current one is used if none specified"
	echo -e "\t-dry\tdry run - don't do any changes just try what would be done."
	exit 1
}

if [ $# -lt 1 ]
then
       usage
fi	

PATCH_FILE=$1
DIR=
DRY=

shift
for param in $*
do
	if [ -d $1 ]
	then
		DIR="$DIR \"$1\""
		shift
		continue
	fi

	if [ "i$1" = "i-dry" ]
	then
		DRY=--dry-run
		shift
		continue
	fi

	echo "Unrecognized parameter $1 (skipping)"
	shift
done

if [ -z $DIR ]
then
	DIR="."
fi

echo patch to be used: $PATCH_FILE
echo directories to be checked: $DIR
echo dry-run $DRY

for FILE in `find $DIR -type f -iname "*\.cc" -o -iname "*\.h" -o -iname "*\.c"`
do
	echo -n "Patching $FILE "
#	cat $PATCH_FILE | sed -e "s/\(^---\)\s\(.*\)\s.*/\1 ${FILE}/" -e "s/\(^+++\)\s\(.*\)\s.*/\1 ${FILE}/" | patch -p0 --dry-run && echo ok || echo failed

	cat $PATCH_FILE | patch -p0 $DRY $FILE >& /dev/null && echo -e "\tok" || echo -e "\tfailed"
done
