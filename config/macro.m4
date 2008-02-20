dnl Macro for detecting executable binary
dnl Macro check in $PATH, in supplied list and optionally check for correct version of the binary
dnl Macro provides --with-BINARYNAME=BIN parameter to specify what should be used at "the command"
dnl Parameters:
dnl  $1 - name of the binary (BINARYNAME)
dnl  $2 - names of command to call
dnl  $3 - extra paths in addition to those from $PATH to look for the command, separated by spaces
dnl  $4 - code used to check if the binary is correct. Code will have name of the binary in $testbin.
dnl   If the binary is wrong, code should set $testbin to empty string
dnl  $5 - extra help text used for --with-BINARYNAME switch
AC_DEFUN(CHK_EXECUTABLE,[
 dnl supply command to override autodetection
 AC_ARG_WITH($1,
  AS_HELP_STRING([--with-$1=BIN],[Use specific $1 binary with the absolute path. $5]),
  [$1_command="$withval"],
  [$1_command=""]
 )

 $1[]_bin=""
 allpaths=""
 dnl TODO: path with spaces in it could break it if not quoted properly
 sppaths=`echo $[]PATH|sed 's/:/ /g'`
 for names in $2
 do
  for paths in $3 $[]sppaths
  do
   allpaths="$[]allpaths $[]paths/$[]names"
  done
 done
 dnl TODO: if command specified, do not look somewhere else?
 for i in $[]$1_command $[]allpaths
 do
  if test -x $[]i
  then
   testbin=$[]i
   dnl use the check code to test the binary
   $4
   $1[]_bin=$[]testbin
   dnl check if the binary is accepted by the check code
   if test -n "$[]testbin"; 
   then 
    break;
   else
    AC_MSG_RESULT(found $[]i but it failed check)
    AC_MSG_CHECKING(for another)
   fi
  fi
 done
])
