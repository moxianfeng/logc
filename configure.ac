#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE #1
AC_CONFIG_SRCDIR([logc.c])
AC_CONFIG_HEADERS([config.h])

AC_PROG_LIBTOOL

AC_PROG_LIBTOOL

#AC_ARG_ENABLE(
#[CFLAGS="${CFLAGS} -DBINARY_BYTES_PER_LINE=${enableval}"],
#AC_ARG(linebytes, 
#    AS_HELP_STRING([--linebytes], [Binary log bytes per line [32]]),
#    CFLAGS="${CFLAGS} -DBINARY_BYTES_PER_LINE=${withval}",
#    CFLAGS="${CFLAGS} -DBINARY_BYTES_PER_LINE=",
#)
#AC_ARG_VAR(["--linebytes"], 
#    AS_HELP_STRING([--linebytes], [Binary log bytes per line [32]])
#)

AC_ARG_VAR(BINARY_COLS, [how many byes be formmated in one line for log binary])

#CFLAGS="${CFLAGS} -DBINARY_COLS=32"
#AC_SUBST([CFLAGS])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
#AC_CHECK_LIB([])

# Checks for header files.


# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_OUTPUT(Makefile)
