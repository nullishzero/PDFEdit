#!/bin/bash
# PDFedit - free program for PDF document manipulation.
# Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
#                                        Jozef Misutka,
#                                        Martin Petricek
#                   Former team members: Miroslav Jahoda
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program (in doc/LICENSE.GPL); if not, write to the 
# Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
# MA  02111-1307  USA
#
# Project is hosted on http://sourceforge.net/projects/pdfedit

PROG_NAME=`basename $0`

usage()
{
	echo "$PROG_NAME usage:"
	echo -e "\t$PROG_NAME patch_file [-dry] [-revert] files*"
	echo -e "\n\tpatch_file\tpatch to apply."
	echo -e "\tfiles\tfiles to be used"
	echo -e "\t-dry\tdry run - don't do any changes just try what would be done."
	echo -e "\t-revert\trevers given patch"
	exit 1
}

if [ $# -lt 1 ]
then
       usage
fi	

PATCH_FILE=$1
FILES=
DRY=
REVERT=

shift
for param in $*
do
	if [ "i$1" = "i-dry" ]
	then
		DRY=--dry-run
		shift
		continue
	fi

	if [ "i$1" = "i-revert" ]
	then
		REVERT=-R
		shift
		continue
	fi

	if [ -f $1 ]
	then
		FILES="$FILES $1"
		shift
		continue
	fi

	echo "Unrecognized parameter $1 (skipping)"
	shift
done

echo patch to be used: $PATCH_FILE
echo files to be patched: $FILES
[ -n "$DRY" ] && echo dry-run
[ -n "$REVERT" ] && echo reverting

for FILE in $FILES
do
	echo -n "Patching $FILE "
#	cat $PATCH_FILE | sed -e "s/\(^---\)\s\(.*\)\s.*/\1 ${FILE}/" -e "s/\(^+++\)\s\(.*\)\s.*/\1 ${FILE}/" | patch -p0 --dry-run && echo ok || echo failed

	patch <$PATCH_FILE -p0 $DRY $REVERT $FILE >& /dev/null && echo -e "\tok" || echo -e "\tfailed"
done
