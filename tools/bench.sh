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

usage()
{
	PROG=`basename $0`
	echo $PROG usage:
	echo 
	echo "$PROG benchmarks file1 [file2 ...] "
	echo "where"
	echo -e "\tbenchmarks name of benchmark(s) to run. Multiple benchmarks have"
	echo -e "\t\tto be quoted properly to fit in the first argument."
	echo -e "\tfileN\tone or more files to use as benchmarks parameters"
	echo 
	echo "Script will create one directory for each benchmarks with the benchmark"
	echo "name and _results suffix which will contain file_info and file_result"
	echo "files."
	echo "The first one contains information about document, the later results from"
	echo "the benchmarks"
	exit 1
}

check_and_create_output_dir()
{
	OUTDIR="$1"
	if [ -d "$OUTDIR" ]
	then
		echo "\"$OUTDIR\" directory already exists"
		exit 1
	fi

	mkdir "$OUTDIR"
}

if [ $# -lt 2 ]
then
	usage
fi

BENCH="$1"
BENCH_SUFFIX="_results"
ATEMPTS=3
shift

for b in $BENCH
do
	OUT="${b}$BENCH_SUFFIX"
	if [ ! -x "$b" ]
	then
		echo "\"$b\" is not a executable. Skipping"
		continue
	fi
	check_and_create_output_dir "$OUT"
	echo "Running $b"
	for f in $*
	do
		if [ ! -f "$f" ]
		then
			echo "\"$f\" is not a file. Skipping"
			continue
		fi
		echo -en "\t$f "
		OUTNAME=`basename "$f"`
		./file_info "$f" > "$OUT/${OUTNAME}_info" || continue
		for i in `seq $ATEMPTS`
		do
			"./$b" "$f" >> "$OUT/${OUTNAME}${BENCH_SUFFIX}"
			echo -n "$i "
		done
		echo
	done
done
