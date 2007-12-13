#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(lyriczilla, 0.2, BUG-REPORT-ADDRESS)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

AC_ARG_ENABLE(debug, [AC_HELP_STRING([--enable-debug],
        [compile with debugging support])], , enable_debug=no)

if test "x$enable_debug" = "xyes" ; then
        AC_DEFINE(DEBUG, 1, [Define if debugging is enabled.])
fi

#AC_PROG_LIBTOOL

# Checks for programs.
#AC_PROG_CXX
AC_PROG_CC
#AC_PROG_CPP
AC_PROG_INSTALL
#AC_PROG_LN_S
AC_PROG_MAKE_SET
#AC_PROG_RANLIB

#AC_CHECK_LIB([pthread], [pthread_create], , [AC_ERROR(pthread is required.)])

# Checks for libraries.
PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.12.3, , [AC_ERROR(GTK+ (>=2.12.3) is required.)])
PKG_CHECK_MODULES(dbus, dbus-1 >= 1.1.2, , [AC_ERROR(dbus (>=1.1.2) is required.)])
PKG_CHECK_MODULES(dbus_glib, dbus-glib-1 >= 0.74, , [AC_ERROR(dbus-glib (>=0.74) is required.)])
PKG_CHECK_MODULES(audacious, audacious >= 1.4.2, , [AC_ERROR(audacious (>=1.4.3) is required.)])


# Checks for header files.
#AC_HEADER_STDC
#AC_CHECK_HEADERS([string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 data/Makefile
                 lyriczilla/Makefile
                 audacious/Makefile])
AC_OUTPUT
