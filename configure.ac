AC_INIT(EC2 Instance Seeder, 0.1, [], ec2seed, https://github.com/akkornel/ec2seed)

dnl We need autoconf 2.60 to get AC_PROG_CC_C99
AC_PREREQ([2.60])

AM_INIT_AUTOMAKE(foreign)

AC_PROG_CC

dnl We need C99 for the bool type
AC_PROG_CC_C99

dnl Make sure we have the Linux headers we need
AC_CHECK_HEADERS([linux/random.h linux/types.h])

dnl Check for some more headers that we need
AC_CHECK_HEADERS([assert.h fcntl.h getopt.h unistd.h sys/ioctl.h])

dnl Make sure we have the json-c module
PKG_CHECK_MODULES([JSON], [json-c])

dnl We need libcurl 7.37.0+ for proper POST handling with redirects
PKG_CHECK_MODULES([CURL], [libcurl >= 7.35.0])

dnl We need libunistring to handle UTF-8 responses
AC_CHECK_LIB(unistring, u8_check, LIBUNISTRING_FOUND="yes")
if test x"$LIBUNISTRING_FOUND" != x"yes" ; then
	AC_MSG_ERROR([Please install the libunistring library.])
fi

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

AC_OUTPUT()
