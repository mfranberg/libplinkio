#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([libplinkio], [0.2], [mattias.franberg@googlemail.com])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_SRCDIR([src/fam.c])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AM_CONDITIONAL(GCC, test "$GCC" = yes)

# Checks for libraries.
LT_INIT

# Python 
AM_PATH_PYTHON([2.7])
AC_ARG_VAR([PYTHON_INCLUDE], [Include flags for python, bypassing python-config])
AC_ARG_VAR([PYTHON_CONFIG], [Path to python-config])
AS_IF([test -z "$PYTHON_INCLUDE"], [
    AS_IF([test -z "$PYTHON_CONFIG"], [
        AC_PATH_PROGS([PYTHON_CONFIG],
                      [python$PYTHON_VERSION-config python-config],
                      [no],
                      [`dirname $PYTHON`])
        AS_IF([test "$PYTHON_CONFIG" = no], [AC_MSG_ERROR([cannot find python-config for $PYTHON.])])
    ])
    AC_MSG_CHECKING([python include flags])
    PYTHON_INCLUDE=`$PYTHON_CONFIG --includes`
    AC_MSG_RESULT([$PYTHON_INCLUDE])
])


# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stddef.h stdlib.h string.h endian.h machine/endian.h sys/endian.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([bzero memmove memset strdup strtol strtod])

# Optionally enable tests
AC_ARG_ENABLE(tests, 
              [AC_HELP_STRING([--enable-tests], [Build unit tests (default is yes)])],
              [case "${enableval}" in
                    yes | no ) WITH_TESTS="${enableval}" ;;
                    *) AC_MSG_ERROR(bad value ${enableval} for --enable-tests) ;;
                    esac],
              [WITH_TESTS="yes"])
AM_CONDITIONAL(WITH_TESTS, test x$WITH_TESTS = xyes)

if test "$WITH_TESTS" = "yes"; then
    AC_CONFIG_SUBDIRS([libs/cmockery])
fi

AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile py-plinkio/Makefile])

AC_OUTPUT

