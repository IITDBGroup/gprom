AC_INIT([gprom], [1.0])
AM_INIT_AUTOMAKE([foreign])
AC_PROG_CC
AC_PROG_CXX
LT_INIT

# Check for standard headers
AC_CHECK_HEADERS([stddef.h setjmp.h string.h])

# Check for functions
AC_CHECK_FUNCS([strcmp sigsetjmp strsep])

# Check for PostgreSQL library
AC_CHECK_LIB([pq], [PQconnectdb], [HAVE_LIBPQ=1], [HAVE_LIBPQ=0])
# Check for libpq-fe.h in standard locations
AC_CHECK_HEADER([postgresql/libpq-fe.h], 
    [HAVE_LIBPQ_FE_H=1],
    [AC_CHECK_HEADER([libpq-fe.h], 
        [HAVE_LIBPQ_FE_H=1], 
        [HAVE_LIBPQ_FE_H=0])])
if test "$HAVE_LIBPQ" = "1" && test "$HAVE_LIBPQ_FE_H" = "1"; then
    AC_DEFINE([HAVE_LIBPQ], [1], [Have PostgreSQL libpq library])
    AC_DEFINE([HAVE_LIBPQ_FE_H], [1], [Have PostgreSQL libpq-fe.h header])
fi

# Check for Java (optional)
AC_ARG_ENABLE([java],
    [AS_HELP_STRING([--enable-java], [Enable Java bindings])],
    [enable_java=$enableval],
    [enable_java=no])

if test "x$enable_java" = "xyes"; then
    AC_CHECK_PROG([JAVA], [java], [java], [no])
    if test "x$JAVA" = "xno"; then
        AC_MSG_ERROR([Java not found but --enable-java was specified])
    fi
    JAVABINDING=yes
else
    JAVABINDING=no
fi

AM_CONDITIONAL([JAVABINDING], [test "x$JAVABINDING" = "xyes"])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
