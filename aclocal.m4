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
