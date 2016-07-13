AC_INIT(ec2seed, 0.1)

AM_INIT_AUTOMAKE()

AC_PROG_CC

dnl Make sure we have the Linux headers we need
AC_CHECK_HEADERS([linux/random.h linux/types.h])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

AC_OUTPUT()