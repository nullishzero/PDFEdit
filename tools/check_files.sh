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

###############################################################################
# Helper script for CPPUnit testing
#
# Primary goal of this script is to provide easy to review results from our
# CPPunit test cases and fast regression testings.
# Features:
# 	- runs only one test case at the time. This is especially useful when
#	  some of test cases fails/crashes, we can just log this issue and
#	  go on with other tests
#	- suppresses all output and redirects it to the log files (splitting
#	  standard and error outputs)
#	- enables auto detection of all test suites defined in CPPunit modules
#	- clean overview of results. There is list of all documents for which
#	  at least one test suite failed followed by detailed per file overview
#	  of failed suites
#	- enables fine grained control over test binary, its parameters, test
#	  suites to be checked and others (please run ./check_files.sh -h for
#	  more detailed information or look for examples).
#
# Examples:
# 1) We want to simply run all test suites from kernel test for all pdf 
#    documents in the testset directory (we are in pdfedit root tree):
#  
#	$ cd src/tests/kernel && \
#		$PDFEDIT_ROOT/check_files.sh $PDFEDIT_ROOT/testset/*.pdf
#
#    Same can be done like this from $PDFEDIT_ROOT
#	$ tools/check_files.sh -b src/tests/kernel/test_bin -d src/tests/kernel\
#		$PDFEDIT_ROOT/tests/*.pdf
#
# 2) We want to run specific test suits (lets say TEST_CPDF and TEST_STREAM)
#
#	$ cd src/tests/kernel && \
#		$PDFEDIT_ROOT/check_files.sh -t "TEST_CPDF TEST_STREAM" \
#		$PDFEDIT_ROOT/testset/*.pdf
#
#    Note that test suits have to be quoted for -t parameter.
#
# 3) We want to set debug level for test binary to have more information
#
#	$ cd src/tests/kernel && \
#		$PDFEDIT_ROOT/check_files.sh -p "-debug_level 5 -all" \
#		$PDFEDIT_ROOT/testset/*.pdf
#
#    Note that test binary parameters to be quoted for -p parameter.
#
# 4) We want to keep log files apart from current directory
#
#	$ [ -d /tmp/pdfedit.logs ] && mkdir /tmp/pdfedit.logs
#	$ cd src/tests/kernel && \
#		-l /tmp/pdfedit.logs \
#		$PDFEDIT_ROOT/testset/*.pdf
#
###############################################################################

print_help()
{
	echo "`basename $0` [-h] [-b test_binary] [-d dir_with_test|-t test_names] [-l log_dir] [-p test_params] test_doc+"
	echo where
	echo -e "\t-h\tPrints this help"
	echo -e "\t-b test_binary\tSpecifies CPPUNIT binary to run (./test_bin \n\
		by default)"
	echo -e "\t-d dir_with_test\tSpecifies directory with CPPUNIT modules. \n\
		This directory is searched for all defined test cases (*.cc and *.h
		files with CPPUNIT_TEST_SUITE_NAMED_REGISTRATION). Note that \n\
		this parameter is suppressed by -t test_names"
	echo -e "\t-t test_names\tSpecifies explicitly which test cases to run.\n\
		Note that more than one test cases have to be properly quoted. \n\
		This parameter also suppress -d parameter because it is more \n\
		specific on which test cases are run and thus we don't have to\n\
		search for them in CPPUNIT modules"
	echo -e "\t-l log_dir\tSpecifies directory where log files are placed.\n\
		Normally, all output is suppress during tests and standard output\n\
		is redirected to testdoc_testname.log and error to testdoc_testname.err\n\
		file placed in log_dir. Default value is current directory."
	echo -e "\t-p test_params\tSpecifies additional parameters for test binary.\n\
		This can be used for finer grained control over test binary \n\
		(how much verbose etc)"
	echo -e "\nAll other parameters are treated as documents to be tested.\n\
		If a parameter is not regular file, it is ignored."
	exit 0
}

# directory with test modules
TEST_DIR=./

# CPPUNIT binary
# can be overwritten by -b parameter
TEST_BIN="./test_bin"

# parameters for TEST_BIN
TEST_PARAMS=

# Documents to be tested
# Filled by files specified in command line
TEST_DOC=""

# Directory where to put result logs
# Can be overwritten by -l parameter
LOG_DIR=./

# Checks all used variables
check_vars()
{
	if [ ! -x "$TEST_BIN" ] 
	then
		echo "Test binary \"$TEST_BIN\" not found"
		exit 1
	fi

	if [ ! -d "$TEST_DIR" ] 
	then
		echo "test directory \"$TEST_DIR\" is not a directory"
		exit 1
	fi

	if [ ! -d "$LOG_DIR" ] 
	then
		echo "log directory \"$LOG_DIR\" is not a directory"
		exit 1
	fi
}

# Parse command line 
# files have precedence over parameters
while getopts hb:d:l:p:t: OPT
do
	case $OPT in
		h ) print_help ;;
		b ) TEST_BIN=$OPTARG 
		;;
		d ) TEST_DIR="$OPTARG/"
		;;
		l ) LOG_DIR="$OPTARG/"
		;;
		p ) TEST_PARAMS=$OPTARG;;
		t ) TEST_NAMES=$OPTARG;;
		* ) "Unrecognized parameter \"$param\" - ignoring" ;;
	esac
done

check_vars

# All parameters which are not recognized as options should
# be test documents
if [ $OPTIND -le $# ]
then
	shift $((OPTIND-1))
	for i in $*
	do
		if [ -f $i ]
		then	
			TEST_DOC="$TEST_DOC $i"
		else
			echo "\"$i\" is not a file - ignoring"
		fi
	done
fi

# get names of all registered (for CPPUNIT) tests
if [ -z "$TEST_NAMES" ]
then
	TEST_NAMES=`grep CPPUNIT_TEST_SUITE_NAMED_REGISTRATION ${TEST_DIR}*.cc ${TEST_DIR}*.h | sed 's/.*\"\(.*\)\".*/\1/'`
fi

# for debugging
#echo TEST_BIN $TEST_BIN
#echo TEST_PARAMS $TEST_PARAMS
#echo TEST_DIR $TEST_DIR
#echo TEST_DOC $TEST_DOC
#echo TEST_NAMES $TEST_NAMES
#echo LOG_DIR $LOG_DIR

if [ -z "$TEST_DOC" ]
then
	echo "No test document specified"
	exit 0	
fi

# cumulative description of failed tests
# It has following format:
# document:testname\n
# for each failed test
FAILED_TESTS=""
for file_name in $TEST_DOC
do
	file_log_name=`basename ${file_name}`
	for test_name in $TEST_NAMES
	do
		$TEST_BIN $TEST_PARAMS $test_name $file_name \
			> ${LOG_DIR}${file_log_name}_${test_name}.log\
			2> ${LOG_DIR}${file_log_name}_${test_name}.err \
			|| FAILED_TESTS="${FAILED_TESTS}$file_name:$test_name\n"
	done
done

echo Failure summary:
if [ -z $FAILED_TESTS ]
then
	echo No failure
	exit 0
fi

echo -n Files: 
FILES=`echo -e $FAILED_TESTS | cut -d: -f1 | sort -u`
echo $FILES
echo -e "\nDetailed:"

for i in $FILES
do
	echo File:$i
	echo Tests:
	echo -e $FAILED_TESTS | grep $i | cut -d: -f2
	echo
done

# At least one error occurred
exit 1
