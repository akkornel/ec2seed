AC_INIT(EC2 Instance Seeder, 0.1, [], ec2seed, https://github.com/akkornel/ec2seed)

AM_INIT_AUTOMAKE(foreign)

AC_PROG_CC

dnl Make sure we have the Linux headers we need
AC_CHECK_HEADERS([linux/random.h linux/types.h])

dnl Check for some more headers that we need
AC_CHECK_HEADERS([assert.h fcntl.h getopt.h unistd.h sys/ioctl.h])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

AC_OUTPUT()
