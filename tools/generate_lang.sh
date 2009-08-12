#!/bin/sh
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

NAME=`basename $0`

usage()
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
