AC_DEFUN(CHK_ENV_VAR,[
 if test "x$[]$1" = x; then
  AC_MSG_ERROR($2)
 fi
])
AC_DEFUN(CHK_ENV_VAR_WARN,[
 if test "x$[]$1" = x; then
  AC_MSG_WARN($2)
 fi
])
AC_DEFUN(CHK_ENV_VAR_NOTICE,[
 if test "x$[]$1" = x; then
  AC_MSG_NOTICE($2)
 fi
])

AC_DEFUN(CHK_OR_DEFINE_VAR, [ 
	if test -z "$[]$1"
	then
		$1=$2
		AC_MSG_NOTICE(Using default $1=$2)
	else
		AC_MSG_NOTICE(Using provided $1=$[]$1)
	fi
	])
