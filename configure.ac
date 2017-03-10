#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([cchat], [1.0], [laitaur@foxmail.com])
AM_INIT_AUTOMAKE(cchat, 1.0, laitaur@foxmail.com)
AC_CONFIG_SRCDIR([src/cchat.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([curses], [pthread])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h netdb.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset select socket strerror])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
