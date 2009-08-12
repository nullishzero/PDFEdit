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

LOGFILE=/tmp/fake_install_tools.log

usage()
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
