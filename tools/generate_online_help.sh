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

print_usage()
{
	echo
	echo Usage:
	echo
	echo "`basename $0` doc_dir gui_dir [force]"
	echo
	echo doc_dir - path to the documentation directory
	echo gui_dir - path to the gui directory
	echo force - optional parameter which can be used to force regeneration even
	echo         if help is already created
	echo
	echo Generates online help for gui application.
	exit 1
}

if [ $# -lt 2 ]
then
	echo Not enough parameters.
	print_usage
fi

DOC_DIR=$1
GUI_DIR=$2

if [ ! -d ${DOC_DIR} -o ! -d ${GUI_DIR} ]
then 
	echo Bad parameters.
	print_usage
fi

# Check whether documentation is already generated and
# exit if so (unless we are forced - 3rd parameter is 
# present
if [ -f ${GUI_DIR}/help/pdfedit.html ]
then
	if [ $# -ge 3 ]
	then
		echo Online help seems to be generated, but we are forced
		echo to generate it again.
		# TODO remove current help
	else
		echo Online help seems to be generated.
		exit 0
	fi
fi

cd ${DOC_DIR}/user 
# TODO: Perhaps correctly determine how to invoke GNU make (or fix the Makefile)
# BSD make will not work for generating online help
gmake online_help || make online_help
