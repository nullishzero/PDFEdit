AC_DEFUN(CHK_ENV_VAR,[
 if test "x$[]$1" = x; then
  AC_MSG_ERROR($2)
 fi
])
