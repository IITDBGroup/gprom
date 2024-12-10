AC_LANG([C])
################################################################################
# DEFINTIONS OF HELPER FUNCTIONS
# Check whether we compile 32 or 64 bit code
AC_DEFUN([AC_MACHINE_ARCH],
[
	AC_CHECK_SIZEOF([void*])
	AC_MSG_CHECKING([checking whether to use 32bit or 64bit])
	if test "x$ac_cv_sizeof_voidp" == x4; then
		MYARCH=32
	else
		MYARCH=64
	fi
	AC_MSG_RESULT([${MYARCH}])
	AC_SUBST([COMPILER_OSARCH],${MYARCH})
	AC_DEFINE_UNQUOTED([COMPILER_OSARCH],["${MYARCH}"],[32/64 bit architecture])
])
# OCI location
AC_DEFUN([AC_OCI_LOCATION],
[AC_ARG_WITH(oci, [  --with-oci], , )
AC_MSG_CHECKING([for user provided OCI location])
if test "x$with_oci" = x; then
    with_oci=/usr/local/oracle
    if test -d "$with_oci"; then
		LDFLAGS="$LDFLAGS -L$with_oci"
    	CPPFLAGS="$CPPFLAGS -I$with_oci/include"
    	AC_MSG_RESULT([yes, found at $with_oci])
    else
    	AC_MSG_RESULT([not set])
	fi
else
	if test -d "$with_oci"; then
		LDFLAGS="$LDFLAGS -L$with_oci"
    	CPPFLAGS="$CPPFLAGS -I$with_oci/include"
    	AC_MSG_RESULT([yes, found at $with_oci])
	else
    	AC_MSG_RESULT([no])
    	AC_MSG_WARN(no OCI library found in location $with_oci)
	fi
fi
])
# OCI library location
AC_DEFUN([AC_OCI_LIB_LOCATION],
[AC_ARG_WITH(oci-lib, [  --with-oci-lib], , )
AC_MSG_CHECKING([for user provided OCI library location])
if test "x$with_oci_lib" != x; then
	if test -d "$with_oci_lib"; then
		LDFLAGS="$LDFLAGS -L$with_oci_lib"
    	AC_MSG_RESULT([yes, found at $with_oci_lib])
	else
    	AC_MSG_RESULT([no])
    	AC_MSG_WARN(no library for OCI found in location $with_oci_lib)
	fi
else
	AC_MSG_RESULT([not set])
fi
])
# OCI header location
AC_DEFUN([AC_OCIH_LOCATION],
[AC_ARG_WITH(oci-headers, [  --with-oci-headers], , )
AC_MSG_CHECKING([for user provided OCI header location])
if test "x$with_oci_headers" != x; then
	if test -f "$with_oci_headers/oci.h"; then
    	CPPFLAGS="$CPPFLAGS -I$with_oci_headers/include"
    	AC_MSG_RESULT([yes, found at $with_oci_headers])
	else
    	AC_MSG_RESULT([no])
    	AC_MSG_WARN(no headers for OCI found in location $with_oci_headers)
	fi
else
	AC_MSG_RESULT([not set])
fi
])
# OCILIB location
AC_DEFUN([AC_OCILIB_LOCATION],
[AC_ARG_WITH(ocilib, [  --with-ocilib], , )
AC_MSG_CHECKING([for user provided OCILIB location])
if test "x$with_ocilib" != x; then
	if test -d "$with_ocilib"; then
		LDFLAGS="$LDFLAGS -L$with_ocilib"
    	CPPFLAGS="$CPPFLAGS -I$with_ocilib/../include"
    	AC_MSG_RESULT([yes, $with_ocilib])
	else
    	AC_MSG_RESULT([no])
    	AC_MSG_WARN(no OCI library found in location $with_ocilib)
	fi
else
	AC_MSG_RESULT([not set])
fi
])
# z3 libz3 library location
AC_DEFUN([AC_LIBZ3_LOCATION],
[AC_ARG_WITH(libz3, [  --with-libz3], , )
AC_MSG_CHECKING([for user provided libz3 location])
if test "x$with_libz3" != x; then
	if test -d "$with_libz3"; then
		LDFLAGS="$LDFLAGS -L$with_libz3"
    	CPPFLAGS="$CPPFLAGS -I$with_libz3/../include"
    	AC_MSG_RESULT([yes, $with_libz3])
	else
    	AC_MSG_RESULT([no])
    	AC_MSG_WARN(no libz3 library found in location $with_libz3)
	fi
else
	AC_MSG_RESULT([not set])
fi
])
# All Oracle Checks
AC_DEFUN([AC_ORACLE_ALL_CHECKS],
[
# check if user has provided us with non-standard locations for OCI/OCILIB libraries
  AC_OCI_LOCATION
  AC_OCIH_LOCATION
  AC_OCI_LIB_LOCATION
  AC_OCILIB_LOCATION
  AC_CHECK_HEADERS([ocilib.h],
# do nothing if header exists
    [],
# fail if header not found
    [AC_MSG_ERROR([missing Oracle ocilib.h header])]
    )
# check for the libraries
  AC_CHECK_LIB([clntsh],[OCIInitialize],[],[
    AC_MSG_ERROR([missing OCI library - if you want to compile without Oracle backend support use --disable-oracle])
  ])
  AC_CHECK_LIB([occi],[OCIHandleFree],[],[
  	AC_MSG_ERROR([missing OCI library - if you want to compile without Oracle backend support use --disable-oracle])
  ])
  AC_CHECK_LIB([ocilib],[OCI_Cleanup],[],[
    AC_MSG_ERROR([missing OCILIB library - if you want to compile without Oracle backend support use --disable-oracle])
  ])
])
# ODBC header location
AC_DEFUN([AC_ODBC_HEADER_LOCATION],
[AC_ARG_WITH(odbc-headers, [  --with-odbc-headers], , )
AC_MSG_CHECKING([for user provided odbc header location])
if test "x$with_odbc_headers" != x; then
	if test -f "$with_odbc_headers/sql.h"; then
    	CPPFLAGS="$CPPFLAGS -I$with_odbc_headers"
    	AC_MSG_RESULT([yes, found at $with_odbc_headers])
	else
    	AC_MSG_RESULT([no])
    	AC_MSG_WARN(no headers for ODBC found in location $with_ODBC_headers)
	fi
else
	AC_MSG_RESULT([not set])
fi
])
# ODBC library location
AC_DEFUN([AC_ODBC_LOCATION],
[AC_ARG_WITH(odbc, [  --with-odbc], , )
AC_MSG_CHECKING([for user provided odbc location])
if test "x$with_odbc" != x; then
	if test -d "$with_odbc"; then
		LDFLAGS="$LDFLAGS -L$with_odbc"
    	CPPFLAGS="$CPPFLAGS -I$with_odbc/../include"
    	AC_MSG_RESULT([yes, $with_odbc])
	else
    	AC_MSG_RESULT([no])
    	AC_MSG_WARN(no odbc library found in location $with_odbc)
	fi
else
	AC_MSG_RESULT([not set])
fi
])
# All ODBC Checks
AC_DEFUN([AC_ODBC_ALL_CHECKS],
[
# check if user has provided us with a non-standard location for Postgres's libpq library
  AC_ODBC_LOCATION
  AC_ODBC_HEADER_LOCATION
  AC_CHECK_HEADERS([sql.h],
# nothing when found
    [],
# fail if not found
    [AC_MSG_ERROR([Unable to find ODBC sql.h])]
  )
  AC_CHECK_HEADERS([sqlext.h],
# nothing when found
    [],
# fail if not found
    [AC_MSG_ERROR([Unable to find ODBC sqlext.h])]
  )
# check for the libpq library
  AC_CHECK_LIB([odbc],[SQLAllocEnv],[],[
    AC_MSG_ERROR([missing ODBC library - if you want to compile without ODBC backend support use --disable-odbc])
  ])
]
)
# MSSQL header location
AC_DEFUN([AC_MSSQL_HEADER_LOCATION],
[AC_ARG_WITH(mssql-headers, [  --with-mssql-headers], , )
AC_MSG_CHECKING([for user provided mssql header location])
if test "x$with_mssql_headers" != x; then
	if test -f "$with_mssql_headers/msodbcsql.h"; then
    	AC_MSG_RESULT([yes, found at $with_mssql_headers])
	else
    	AC_MSG_RESULT([no])
    	AC_MSG_WARN(no headers for MSSQL found in location $with_mssql_headers)
	fi
else
	AC_MSG_RESULT([not set])
fi
])
# MSSQL library location
AC_DEFUN([AC_MSSQL_LOCATION],
[AC_ARG_WITH(mssql, [  --with-mssql], , )
AC_MSG_CHECKING([for user provided MSSQL location])
if test "x$with_mssql" != x; then
	if test -f "$with_mssql/msodbcsql.h"; then
    	AC_MSG_RESULT([yes, $with_mssql])
	else
    	AC_MSG_RESULT([no])
    	AC_MSG_WARN(no mssql library found in location $with_mssql)
	fi
else
	AC_MSG_RESULT([not set])
fi
])
# All MSSQL Checks
AC_DEFUN([AC_MSSQL_ALL_CHECKS],
[
# check if user has provided us with a non-standard location for Postgres's libpq library
  AC_MSSQL_HEADER_LOCATION
#  AC_CHECK_HEADERS([msodbcsql.h],
# nothing when found
#    [],
# fail if not found
#    [AC_MSG_ERROR([Unable to find MSSSQL msodbcsql.h])]
#  )
# check for the libpq library
#  AC_CHECK_LIB([libmsodbcsql],[bcp_gettypenameA],[],[
#    AC_MSG_ERROR([missing MSSQL ODBC library - if you want to compile without MSSQL backend support use --disable-mssql])
#  ])
]
)
# Postgres libpq header location
AC_DEFUN([AC_LIBPQ_HEADER_LOCATION],
[AC_ARG_WITH(libpq-headers, [  --with-libpq-headers], , )
AC_MSG_CHECKING([for user provided libpq header location])
if test "x$with_libpq_headers" != x; then
	if test -f "$with_libpq_headers/libpq-fe.h"; then
    	CPPFLAGS="$CPPFLAGS -I$with_libpq_headers"
    	AC_MSG_RESULT([yes, found at $with_libpq_headers])
	else
    	AC_MSG_RESULT([no])
    	AC_MSG_WARN(no headers for Postgres libpq found in location $with_libpq_headers)
	fi
else
	AC_MSG_RESULT([not set])
fi
])
# Postgres libpq library location
AC_DEFUN([AC_LIBPQ_LOCATION],
[AC_ARG_WITH(libpq, [  --with-libpq], , )
AC_MSG_CHECKING([for user provided libpq location])
if test "x$with_libpq" != x; then
	if test -d "$with_libpq"; then
		LDFLAGS="$LDFLAGS -L$with_libpq"
    	CPPFLAGS="$CPPFLAGS -I$with_libpq/../include"
    	AC_MSG_RESULT([yes, $with_libpq])
	else
    	AC_MSG_RESULT([no])
    	AC_MSG_WARN(no libpq library found in location $with_libpq)
	fi
else
	AC_MSG_RESULT([not set])
fi
])
# Duckdb header location
AC_DEFUN([AC_DUCKDB_HEADER_LOCATION],
[AC_ARG_WITH(duckdb-headers, [  --with-duckdb-headers], , )
AC_MSG_CHECKING([for user provided duckdb header location])
if test "x$with_duckdb_headers" != x; then
	if test -f "$with_duckdb_headers/duckdb.h"; then
    	CPPFLAGS="$CPPFLAGS -I$with_duckdb_headers"
    	AC_MSG_RESULT([yes, found at $with_duckdb_headers])
	else
    	AC_MSG_RESULT([no])
    	AC_MSG_WARN(no headers for DuckDB found in location $with_duckdb_headers)
	fi
else
	AC_MSG_RESULT([not set])
fi
])
# Duckdb library location
AC_DEFUN([AC_DUCKDB_LOCATION],
[AC_ARG_WITH(duckdb-lib, [  --with-duckdb-lib], , )
AC_MSG_CHECKING([for user provided duckdb location])
if test "x$with_duckdb_lib" != x; then
	if test -d "$with_duckdb_lib"; then
		LDFLAGS="$LDFLAGS -L$with_duckdb_lib"
    	CPPFLAGS="$CPPFLAGS -I$with_duckdb_lib"
    	AC_MSG_RESULT([yes, $with_duckdb_lib])
	else
    	AC_MSG_RESULT([no])
    	AC_MSG_WARN(no duckdb library found in location $with_duckb_lib)
	fi
else
	AC_MSG_RESULT([not set])
fi
])

# All Postgres Checks
AC_DEFUN([AC_POSTGRES_ALL_CHECKS],
[
# check if user has provided us with a non-standard location for Postgres's libpq library
  AC_LIBPQ_LOCATION
  AC_LIBPQ_HEADER_LOCATION
  AC_CHECK_HEADERS([libpq-fe.h],
# nothing when found
    [],
# fail if not found
    [AC_MSG_ERROR([Unable to find PostgreSQL libpq-fe.h])]
  )
# check for the libpq library
  AC_CHECK_LIB([pq],[PQconndefaults],[],[
    AC_MSG_ERROR([missing libpq library - if you want to compile without Postgres backend support use --disable-postgres])
  ])
]
)
# MonetDB Mapi header location
AC_DEFUN([AC_MAPIH_LOCATION],
[AC_ARG_WITH(mapi-headers, [  --with-mapi-headers], , )
AC_MSG_CHECKING([for user provided mapi header location])
if test "x$with_mapi_headers" != x; then
	if test -f "$with_mapi_headers/mapi.h"; then
    	CPPFLAGS="$CPPFLAGS -I$with_mapi_headers"
    	AC_MSG_RESULT([yes, found at $with_mapi_headers])
	else
    	AC_MSG_RESULT([no])
    	AC_MSG_WARN(no headers for MonetDB mapi found in location $with_mapi_headers)
	fi
else
    # monetdb install in monetdb subdir of include directory
    CPPFLAGS="$CPPFLAGS -I/usr/lib/monetdb"
	AC_MSG_RESULT([not set])
fi
])
# MonetDB Mapi library location
AC_DEFUN([AC_MAPI_LOCATION],
[AC_ARG_WITH(mapi, [  --with-mapi-lib], , )
AC_MSG_CHECKING([for user provided MonetDB mapi library location])
if test "x$with_mapi_lib" != x; then
	if test -d "$with_mapi_lib"; then
		LDFLAGS="$LDFLAGS -L$with_mapi_lib"
    	AC_MSG_RESULT([yes, $with_mapi_lib])
	else
    	AC_MSG_RESULT([no])
    	AC_MSG_WARN(no libmapi library found in location $with_mapi_lib)
	fi
else
	AC_MSG_RESULT([not set])
fi
])
# All Monetdb Checks
AC_DEFUN([AC_MONETDB_ALL_CHECKS],
[
# user has provided us with a non-standard location for MonetDB Mapi library
  AC_MAPI_LOCATION
  AC_MAPIH_LOCATION
  AC_CHECK_HEADERS([mapi.h],
# do nothing if header exists
    [],
# fail if header does not exists
    [AC_MSG_ERROR([Unable to find MonetDB mapi.h header])]
  )
# check for the mapi library
  AC_CHECK_LIB([mapi],[mapi_unquote],[],[
    AC_MSG_ERROR([missing mapi library - if you want to compile without MonetDB backend support use --disable-monetdb])
  ])
])
# All SQLite Checks
AC_DEFUN([AC_SQLITE_ALL_CHECKS],
[
    AC_CHECK_LIB([sqlite3],[sqlite3_open],[],[
    	AC_MSG_ERROR([missing libsqlite3 library - if you want to compile without SQLite backend support use --disable-sqlite])
	])
])
# All DuckDB Checks
AC_DEFUN([AC_DUCKDB_ALL_CHECKS],
[
  AC_DUCKDB_LOCATION
  AC_DUCKDB_HEADER_LOCATION
  AC_CHECK_HEADERS([duckdb.h],
# nothing when found
    [],
# fail if not found
    [AC_MSG_ERROR([Unable to find DuckDB duckdb.h])]
  )
  AC_CHECK_LIB([duckdb],[duckdb_open],[],[
    	AC_MSG_ERROR([missing libduckdb library - if you want to compile without DuckDB backend support use --disable-duckdb])
	])
])
# cplex library location
AC_DEFUN([AC_CPLEX_LOCATION],
[
AC_ARG_WITH(cplex, [  --with-cplex], , )
AC_MSG_CHECKING([for user provided cplex location])
if test "x$with_cplex" != x; then
	if test -d "$with_cplex"; then
		# TODO - create actual mapping from archtiecture detected by configure to cplex representation of architectures (e.g., x64-64 instead of x64_64)
		CPLEXSYSTEM=x86-${MYARCH}_${OSTYPE}
		CPLEXLIBFORMAT=static_pic
		LDFLAGS="$LDFLAGS -L$with_cplex/lib/$CPLEXSYSTEM/$CPLEXLIBFORMAT"
    	CPPFLAGS="$CPPFLAGS -I$with_cplex/include"
    	#CFLAGS="$CFLAGS -m64 -fPIC"
    	AC_MSG_RESULT([yes, $with_cplex])
	else
    	AC_MSG_RESULT([no])
    	AC_MSG_WARN(no cplex library found in location $with_cplex)
	fi
else
	AC_MSG_RESULT([not set])
fi
])
# Activate/Deactivate logging
AC_DEFUN([AC_LOGGING_ACTIVE],
[
AC_MSG_CHECKING([should be compiled with logging])
AC_ARG_ENABLE(logging, AS_HELP_STRING([--disable-logging],[Disable logging]))

AS_IF([test "x$enable_logging" == "xno"], [
  AC_DEFINE([DISABLE_LOGGING],[],[Disable logging])
  AC_MSG_RESULT([no])
],
[
  AC_MSG_RESULT([yes])
])
])
# Link with google perftools libary
AC_DEFUN([AC_LINK_PERFTOOLS],
[AC_ARG_ENABLE(link-perftools, AS_HELP_STRING([--enable-link-perftools],[Link against google perftools library]))
AC_MSG_CHECKING([should be linked with google profile library])

AS_IF([test "x$enable_link_perftools" == "xyes"], [
  AC_MSG_RESULT([yes])
],[
  AC_MSG_RESULT([no])
])

AS_IF([test "x$enable_link_perftools" == "xyes"], [
	AC_CHECK_LIB([profiler],[ProfilerStart],[],[AC_MSG_ERROR([missing google perftools libprofiler])])
])
])
# Activate gprof profiling
AC_DEFUN([AC_ENABLE_GPROF],
[AC_ARG_ENABLE(gprof, AS_HELP_STRING([--enable-gprof],[activate profiling with gprof]))
AC_MSG_CHECKING([activate profiling with gprof])

AS_IF([test "x$enable_gprof" == "xyes"], [
  AC_MSG_RESULT([yes])
  CPPFLAGS="$CPPFLAGS -pg"
],[
  AC_MSG_RESULT([no])
])
])
# Activate cplex support
AC_DEFUN([AC_ENABLE_CPLEX],
[
AC_ARG_ENABLE(cplex, AS_HELP_STRING([--enable-cplex],[activate support for cplex]))
AC_MSG_CHECKING([activate linking against cplex])

AS_IF([test "x$enable_cplex" == "xyes"], [
  AC_MSG_RESULT([yes])
  #CPPFLAGS="$CPPFLAGS -pg"
],[
  AC_MSG_RESULT([no])
])
])
# Activate z3 support
AC_DEFUN([AC_ENABLE_Z3],
[
AC_ARG_ENABLE(Z3, AS_HELP_STRING([--enable-z3],[activate support for Z3]))
AC_MSG_CHECKING([activate linking against Z3])
AS_IF([test "x$enable_z3" == "xyes"], [
  AC_MSG_RESULT([yes])
],[
  AC_MSG_RESULT([no])
])
])
# enable or disable assertions
AC_DEFUN([AC_ASSERT_ACTIVE],
[
AC_MSG_CHECKING([should be compiled with assertion support])
AC_ARG_ENABLE(assert, AS_HELP_STRING([--disable-assert],[Disable assertions]))

AS_IF([test "x$enable_assert" == "xno"], [
  AC_DEFINE([DISABLE_ASSERT],[],[Disable assert])
  AC_MSG_RESULT([no])
],
[
  AC_MSG_RESULT([yes])
])
])
# enable or disable debugging symbols
AC_DEFUN([AC_DEBUG_ACTIVE],
[
AC_MSG_CHECKING([should be compiled with debug symbols])
AC_ARG_ENABLE(debug, AS_HELP_STRING([--disable-debug],[Disable compilation with debugging symbols]))

AS_IF([test "x$enable_debug" == "xno"], [
  AC_DEFINE([DISABLE_DEBUG],[],[Disable debug])
  AC_MSG_RESULT([no])
],
[
  AC_MSG_RESULT([yes])
  CPPFLAGS="$CPPFLAGS -g"
])
])
# compile with or without timing support
AC_DEFUN([AC_TIMING_ACTIVE],
[
AC_MSG_CHECKING([should be compiled with timing instrumentation])
AC_ARG_ENABLE(timing, AS_HELP_STRING([--disable-timing],[Disable compilation with timing instrumentation]))

AS_IF([test "x$enable_timing" == "xno"], [
  AC_DEFINE([DISABLE_TIMING],[],[Disable timing])
  AC_MSG_RESULT([no])
],
[
  AC_MSG_RESULT([yes])
])
])
# compile with SQLite support
AC_DEFUN([AC_ENABLE_SQLITE],
[
AC_MSG_CHECKING([should be compiled with support for SQLite backends])
AC_ARG_ENABLE(sqlite, AS_HELP_STRING([--disable-sqlite],[Disable compilation with support for SQLite backends]))

AS_IF([test "x$enable_sqlite" == "xyes"], [
  AC_MSG_RESULT([yes])
],
[
  AC_DEFINE([DISABLE_SQLITE],[],[Disable support for SQLite backends])
  AC_MSG_RESULT([no])
])
])
# compile with DuckDB support
AC_DEFUN([AC_ENABLE_DUCKDB],
[
AC_MSG_CHECKING([should be compiled with support for DuckDB backends])
AC_ARG_ENABLE(duckdb, AS_HELP_STRING([--disable-duckdb],[Disable compilation with support for DuckDB backends]))

AS_IF([test "x$enable_duckdb" == "xyes"], [
  AC_MSG_RESULT([yes])
],
[
  AC_DEFINE([DISABLE_DUCKDB],[],[Disable support for DuckDB backends])
  AC_MSG_RESULT([no])
])
])
# compile with MonetDB support
AC_DEFUN([AC_ENABLE_MONETDB],
[
AC_MSG_CHECKING([should be compiled with support for MonetDB backends])
AC_ARG_ENABLE(monetdb, AS_HELP_STRING([--disable-monetdb],[Disable compilation with support for MonetDB backends]))

AS_IF([test "x$enable_monetdb" == "xyes"], [
  AC_MSG_RESULT([yes])
],
[
  AC_MSG_RESULT([no])
  AC_DEFINE([DISABLE_MONETDB],[],[Disable support for MonetDB backends])
])
])
# compile with ODBC support
AC_DEFUN([AC_ENABLE_ODBC],
[
AC_MSG_CHECKING([should be compiled with support for ODBC backends])
AC_ARG_ENABLE(odbc, AS_HELP_STRING([--disable-odbc],[Disable compilation with support for ODBC backends]))

AS_IF([test "x$enable_odbc" == "xyes"], [
  AC_MSG_RESULT([yes])
],
[
  AC_DEFINE([DISABLE_ODBC],[],[Disable support for ODBC backends])
  AC_MSG_RESULT([no])
])
])
# compile with MSSQL support
AC_DEFUN([AC_ENABLE_MSSQL],
[
AC_MSG_CHECKING([should be compiled with support for MSSQL ODBC backends])
AC_ARG_ENABLE(mssql, AS_HELP_STRING([--enable-mssql],[Enable compilation with support for MSSQL ODBC backends]))

AS_IF([test "x$enable_mssql" == "xyes"], [
  AC_DEFINE([HAVE_MSSQL_ODBC_DRIVER],[1],[Have MSSQL ODBC driver])
  AC_MSG_RESULT([yes])
],
[
  AC_DEFINE([DISABLE_MSSQL],[],[Disable support for ODBC MSSQL backends])
  AC_MSG_RESULT([no])
])
])
# compile with Postgres support
AC_DEFUN([AC_ENABLE_POSTGRES],
[
AC_MSG_CHECKING([should be compiled with support for Postgres backends])
AC_ARG_ENABLE(postgres, AS_HELP_STRING([--disable-postgres],[Disable compilation with support for Postgres backends]))

AS_IF([test "x$enable_postgres" == "xyes"], [
  AC_MSG_RESULT([yes])
],
[
  AC_DEFINE([DISABLE_POSTGRES],[],[Disable support for Postgres backends])
  AC_MSG_RESULT([no])
])
])
# compile with Oracle support
AC_DEFUN([AC_ENABLE_ORACLE],
[
AC_MSG_CHECKING([should be compiled with support for Oracle backends])
AC_ARG_ENABLE(oracle, AS_HELP_STRING([--enable-oracle],[Disable compilation with support for Oracle backends]))

AS_IF([test "x$enable_oracle" == "xyes"], [
  AC_MSG_RESULT([yes])
],
[
  AC_DEFINE([DISABLE_ORACLE],[],[Disable support for Oracle backends])
  AC_MSG_RESULT([no])
])
])
# enable/disable Java bindings
AC_DEFUN([AC_ENABLE_JAVA_BINDING],
[
AC_MSG_CHECKING([build with Java bindings including JDBC driver])
AC_ARG_ENABLE(java, AS_HELP_STRING([--enable-java],[Enable building of Java bindings and tests]))

AS_IF([test "x$enable_java" == "xyes"], [
  AC_MSG_RESULT([yes])
],
[
  AC_DEFINE([DISABLE_JAVA],[],[Disable building of Java binding])
  AC_MSG_RESULT([no])
])
AM_CONDITIONAL(JAVABINDING, test "x$enable_java" = "xyes")
])
# enable/disable readline support
AC_DEFUN([AC_ENABLE_READLINE],
[
AC_MSG_CHECKING([should support readline library in CLI interface])
AC_ARG_ENABLE(readline, AS_HELP_STRING([--disable-readline],[Disable compilation with support for readline library]))

AS_IF([test "x$enable_readline" == "xno"], [
  AC_DEFINE([DISABLE_READLINE],[],[Disable support for readline])
  AC_MSG_RESULT([no])
],
[
  AC_MSG_RESULT([yes])
])
])
# set custom compiler flags
AC_DEFUN([AC_SET_CUSTOM_CFLAGS],
[
AC_MSG_CHECKING([set custom C compiler flags])
AS_IF([test "x$enable_debug" == "xno"],
[
	GPROM_CFLAGS_DEBUG="-g"
	GPROM_CFLAGS_OPT="-O3"
	GPROM_CFLAGS_ERR="-Wall"
],
[
	GPROM_CFLAGS_DEBUG="-g"
	GPROM_CFLAGS_ERR="-Wall -Werror"
	GPROM_CFLAGS_OPT="-O0"
])
GPROM_CFLAGS="${GPROM_CFLAGS_DEBUG} ${GPROM_CFLAGS_OPT} ${GPROM_CFLAGS_ERR} ${GPROM_EXTRA_CFLAGS}"
AC_SUBST([GPROM_CFLAGS],[$GPROM_CFLAGS])
AC_SUBST([GPROM_CFLAGS_OPT],[$GPROM_CFLAGS_OPT])
AC_SUBST([GPROM_CFLAGS_ERROR],[$GPROM_CFLAGS_ERROR])
AC_MSG_RESULT($GPROM_CFLAGS)
])
################################################################################
# Init autoconf
AC_INIT([GProM],[1.0.3],[bglavic@iit.edu])
# CFLAGS from user or set to nothing
AS_IF([test "x$CFLAGS" != "x"],
[
	userCFLAGS="$CFLAGS"
],
[]
)
: ${CFLAGS=""}
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE([-Wall foreign tar-pax])
AC_CONFIG_MACRO_DIR([m4])
AC_SUBST([ACLOCAL_AMFLAGS],[-I m4])
################################################################################
# DETERMINE OS AND ARCHITECTURE SETTINGS AND EXPOSE THEM
AC_CANONICAL_HOST
AS_CASE([$host_os],
   [*linux*], [OSTYPE=linux],
   [*darwin*], [OSTYPE=osx],
   [*windows*], [OSTYPE=windows],
   [*mingw*], [OSTYPE=windows],
   [OSTYPE=unknown]
)
AS_IF([test "x$OSTYPE" == "xwindows"], [
  AC_DEFINE([OS_WINDOWS],[],[Compiling for windows])
  AC_DEFINE([GPROM_BUILD_DLL],[],[Compiling for windows, build dll])
], [])
AC_SUBST([OSTYPE])
AC_SUBST([OSDETAILS],$host_os)
AC_SUBST([OSARCH],$host_cpu)
AC_DEFINE_UNQUOTED([OS_TYPE],["${OSTYPE}"],[type of operating system])
AC_DEFINE_UNQUOTED([OS_DETAILS],["${host_os}"],[OS version])
AC_DEFINE_UNQUOTED([OS_ARCH],["${OSARCH}"],[OS architecture])
AC_MACHINE_ARCH
################################################################################
# CHECK FOR C COMPILER AND FEATURES, BINUTILS, AND BISON + FLEX
#CFLAGS="${GPROM_CFLAGS}"
AC_PROG_CC
# restore user CFLAGS
#CFLAGS=""
#AS_IF([test "x$userCFLAGS" == "x"],
#[
#	CFLAGS=""
#],
#[
#	CFLAGS="$userCFLAGS"
#]
#)
# CAN INLINE FUNCTIONS?
AC_C_INLINE
################################################################################
# SET OUTPUT VARIABLE FOR TOP SOURCE DIR
#mysrcdir="$(pushd "${srcdir}" > /dev/null; pwd)"
#AC_DEFINE_UNQUOTED([GPROM_TOP_SRCDIR],["${mysrcdir}"], ["absolute path to top source dir"])
gprom_srcdir="$(pushd "${srcdir}" > /dev/null; pwd)"
AC_SUBST([GPROM_TOP_SRCDIR],[$gprom_srcdir])
################################################################################
# CHECK FOR PROGRAM BINARIES AND FEATURES
AM_PROG_AR
AC_PROG_YACC
BISON_CMD="`AS_ECHO("$YACC") | sed 's/-y//g'`"
AC_SUBST([BISON], [${BISON_CMD}])
AC_PROG_LEX
#TODO check that versions are sufficient
# check for pandoc and test whether we can use it for man -> html translation of the documentation
AC_CHECK_PROG([pandoc],[pandoc],[yes])
AS_IF([ test "x$pandoc" == "xyes" ],
[
AC_CACHE_CHECK([checking for pandoc that supports man],
               [ac_cv_path_PANDOC],
               [AC_PATH_PROGS_FEATURE_CHECK([PANDOC], [pandoc],
                    [[$ac_path_PANDOC -f man ./doc/gprom.man > /dev/null ;
                      test "x$?" = x0 \
                      && ac_cv_path_PANDOC=$ac_path_PANDOC ac_path_PANDOC_found=:]],
                    [AC_MSG_WARN([could not find pandoc that supports the man format may not be able to build documentation])])
               ])
],
[]
)
AC_SUBST([PANDOC], [$ac_cv_path_PANDOC])
AM_CONDITIONAL([HAVE_PROG_PANDOC], [test "x$pandoc" = xyes])
AM_CONDITIONAL([HAVE_PROG_PANDOC_WITH_MAN], [test "x$ac_cv_path_PANDOC" != "x"])
AC_CHECK_PROG([rman],[rman],[yes])
AM_CONDITIONAL([HAVE_PROG_RMAN], [test "x$rman" = xyes])
# set tool for building documentation
if test "x$pandoc" = "xyes" -a "x$ac_cv_path_PANDOC" != "x"; then
   AC_SUBST([DOCTOOL], ["pandoc"])
else
    if test "x$rman" = "xyes"; then
       AC_SUBST([DOCTOOL], ["rman"])
    else
       AC_SUBST([DOCTOOL], ["none"])
    fi
fi
# INIT LIBTOOL
LT_INIT([win32-dll])
################################################################################
# DATA TYPES
AC_TYPE_SIZE_T
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_LONG_LONG_INT
################################################################################
# HEADERS
AC_CHECK_HEADERS([stddef.h])
AC_CHECK_HEADERS([libintl.h])
AC_CHECK_HEADERS([stdarg.h])
AC_CHECK_HEADERS([time.h])
AC_CHECK_HEADERS([limits.h])
AC_CHECK_HEADERS([regex.h])
AC_CHECK_HEADERS([setjmp.h])
AC_CHECK_HEADERS([signal.h])
AC_CHECK_HEADERS([float.h])
AC_CHECK_HEADERS([pthread.h])
AC_CHECK_HEADERS([math.h])
AC_CHECK_HEADERS([pwd.h])
#AC_CHECK_HEADERS([readline/readline.h])
#AC_CHECK_HEADERS([readline/history.h])
AC_HEADER_ASSERT
################################################################################
# CHECK FOR ADDITIONAL TYPES IN DISCOVERED HEADERS
AC_CHECK_TYPES([ptrdiff_t])
AC_CHECK_TYPES([sigjmp_buf], [], [], [
AC_INCLUDES_DEFAULT
#include <setjmp.h>
])
################################################################################
# MEMORY ALLOCATION STUFF
AC_CHECK_HEADERS([malloc.h])
AC_CHECK_FUNCS([malloc])
AC_CHECK_FUNCS([alloca])
AC_CHECK_FUNCS([realloc])
# removed, because they cause trouble with cross-compilation
#AC_FUNC_ALLOCA
#AC_FUNC_MALLOC
#AC_FUNC_REALLOC
# CHECK FOR STD C FUNCS
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([strdup])
AC_CHECK_FUNCS([strcmp])
#AC_CHECK_FUNCS([sigsetjmp])
AC_CHECK_FUNCS([strsep])
################################################################################
# Check which backends should be supported
AC_ENABLE_ORACLE
AC_ENABLE_POSTGRES
AC_ENABLE_SQLITE
AC_ENABLE_DUCKDB
AC_ENABLE_MONETDB
AC_ENABLE_ODBC
AC_ENABLE_MSSQL
################################################################################
# Check which language bindings to build
AC_ENABLE_JAVA_BINDING
################################################################################
# Enable readline in CLI
AC_ENABLE_READLINE
################################################################################
# check for libm
AC_CHECK_LIB([m],[pow],[],[
  AC_MSG_ERROR([missing libm library (searching for pow function)])
])
################################################################################
# check for libpthread if target is not windows
AS_IF([test "x$OSTYPE" == "xwindows"],
[
  AC_MSG_WARN([compiling for windows target do not attempt to link against libpthread])
],
[
AC_CHECK_LIB([pthread],[pthread_mutex_lock],[],[
  AC_MSG_ERROR([missing libpthread library (searching for pthread_mutex_lock function)])
])
])
################################################################################
# Enable cplex
AC_ENABLE_CPLEX
################################################################################
# cplex library
AS_CASE(["$enable_cplex"],
  [yes], [
# check if user has provided us with non-standard locations for CPLEX libraries
  AC_CPLEX_LOCATION
# check for the libraries
  AC_CHECK_LIB([cplex],[CPXopenCPLEX],[],[
    AC_MSG_ERROR([missing cplex library - if you want to compile without cplex support do not use --enable-cplex])
  ])
  ],
  # No and default
  [no], [],
  []
)
################################################################################
# Readline
AC_CHECK_LIB([readline],[readline],[],[
AS_IF([test "x$enable_readline" == "xno"], [
  AC_MSG_WARN([missing readline library])
],
[
  AC_MSG_ERROR([missing readline library - if you want to compile without readline support use --disable-readline])
])
])
################################################################################
# CPlex
#AC_CHECK_LIB([cplex],[CPXopenCPLEX],[],[
#  AC_MSG_ERROR([missing cplex library])
#])
################################################################################
# Oracle access libraries (OCI and OCILIB)
AS_CASE(["$enable_oracle"],
  [yes], [AC_ORACLE_ALL_CHECKS],
  [no], [],
  []
)
################################################################################
# Postgres C client library (libpq)
AS_CASE(["$enable_postgres"],
  [yes], [AC_POSTGRES_ALL_CHECKS],
  # Deactivated
  [no], [],
  # Default (inactive)
  []
)
################################################################################
# Z3 library (libz3)
AS_CASE(["$enable_z3"],
  [yes], [
# check if user has provided us with a non-standard location for libz3 library
  AC_LIBZ3_LOCATION
  AC_CHECK_HEADERS([z3.h])
# check for the libz3 library
  AC_CHECK_LIB([z3],[Z3_get_version],[],[
    AC_MSG_ERROR([missing libz3 library - if you want to compile without z3 support use --disable-z3])
  ])
  ],
  # Deactivated
  [no], [],
  # Default (inactive)
  []
)
################################################################################
# SQlite library (libsqlite3)
# check for the sqlite3 library
AS_CASE(["$enable_sqlite"],
	[yes], [AC_SQLITE_ALL_CHECKS],
    # Deactivated
    [no], [],
    # Default (inactive)
	[]
)
################################################################################
# DuckDB library (libduckdb)
# check for the duckdb library
AS_CASE(["$enable_duckdb"],
	[yes], [AC_DUCKDB_ALL_CHECKS],
    # Deactivated
    [no], [],
    # Default (inactive)
	[]
)
################################################################################
# MonetDB C client library (mapi)
AS_CASE(["$enable_monetdb"],
  [yes], [AC_MONETDB_ALL_CHECKS],
# Deactivated
  [no], [],
# Default (inactive)
  []
)
################################################################################
# ODBC
AS_CASE(["$enable_odbc"],
  [yes], [AC_ODBC_ALL_CHECKS],
# Deactivated
  [no], [],
# Default (inactive)
  []
)
################################################################################
# MSSQL ODBC driver
AS_CASE(["$enable_mssql"],
  [yes], [AC_MSSQL_ALL_CHECKS],
# Deactivated
  [no], [],
# Default (inactive)
  []
)
################################################################################
# enable/disable features
AC_LOGGING_ACTIVE
AC_LINK_PERFTOOLS
AC_ENABLE_GPROF
AC_ASSERT_ACTIVE
AC_DEBUG_ACTIVE
AC_TIMING_ACTIVE
################################################################################
# set compiler flags
AC_SET_CUSTOM_CFLAGS
################################################################################
# OUTPUT HEADER
AC_CONFIG_HEADERS([config.h])
################################################################################
# DERIVED FILES - MAINLY MAKEFILES
AC_CONFIG_FILES([
	Makefile
	doc/Makefile
	include/Makefile
	src/Makefile
	src/analysis_and_translate/Makefile
	src/command_line/Makefile
	src/configuration/Makefile
	src/exception/Makefile
	src/execution/Makefile
	src/instrumentation/Makefile
	src/interfaces/Makefile
	src/interfaces/jdbc/Makefile
	src/log/Makefile
	src/mem_manager/Makefile
	src/metadata_lookup/Makefile
	src/model/Makefile
	src/model/datalog/Makefile
	src/model/expression/Makefile
	src/model/helperfunction/Makefile
	src/model/list/Makefile
	src/model/bitset/Makefile
    src/model/integrity_constraints/Makefile
	src/model/node/Makefile
	src/model/query_block/Makefile
	src/model/query_operator/Makefile
	src/model/rpq/Makefile
	src/model/set/Makefile
    src/model/graph/Makefile
    src/operator_optimizer/Makefile
    src/parameterized_queries/Makefile
	src/parser/Makefile
	src/provenance_rewriter/Makefile
    src/provenance_rewriter/coarse_grained/Makefile
	src/provenance_rewriter/game_provenance/Makefile
	src/provenance_rewriter/semiring_combiner/Makefile
	src/provenance_rewriter/uncertainty_rewrites/Makefile
	src/provenance_rewriter/pi_cs_rewrites/Makefile
	src/provenance_rewriter/transformation_rewrites/Makefile
	src/provenance_rewriter/update_and_transaction/Makefile
	src/provenance_rewriter/summarization_rewrites/Makefile
	src/provenance_rewriter/xml_rewrites/Makefile
	src/provenance_rewriter/lateral_rewrites/Makefile
	src/provenance_rewriter/unnest_rewrites/Makefile
    src/provenance_rewriter/datalog_lineage/Makefile
    src/provenance_rewriter/semantic_optimization/Makefile
    src/provenance_sketches/Makefile
	src/rpq/Makefile
	src/sql_serializer/Makefile
	src/symbolic_eval/Makefile
	src/temporal_queries/Makefile
	src/utility/Makefile
	src/libgprom/Makefile
	test/Makefile
	javatest/Makefile
])
AC_OUTPUT


echo "
  ($PACKAGE_NAME) version $PACKAGE_VERSION
  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  OS.................: $OSTYPE-$OSARCH ($OSDETAILS)
  Prefix.............: $prefix
  Top src dir........: $GPROM_TOP_SRCDIR
  Debug Build........: ${enable_debug:=yes}
  Profiling..........: google perftools: ${enable_link_perftools:=no}, gprof: ${enable_gprof:=no}
  Features...........: Assertions:${enable_assert:=no} Logging:${enable_logging:=yes} Timing:${enable_timing:=yes} cplex:${enable_cplex:=no} Z3:${enable_z3:=no}
  Backends supported.: Oracle - ${enable_oracle:=no}, Postgres - ${enable_postgres:=no}, SQLite - ${enable_sqlite:=no}, DuckDB - ${enable_duckdb:=no}, MonetDB - ${enable_monetdb:=no}, ODBC - ${enable_odbc:=no}, MSSQL - ${enable_mssql:=no}
  Language bindings..: Java - ${enable_java:=no}
  Readline support...: ${enable_readline:=yes}
  Documentation......: Tool to build documentation: $DOCTOOL
  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  CFLAGS ........: $CFLAGS
  CPPFLAGS ......: $CPPFLAGS
  GPROM_CFLAGS...: $GPROM_CFLAGS
  LDFLAGS........: $LDFLAGS
  C Compiler.....: $C $CPPFLAGS $GPROM_CFLAGS $CFLAGS
  Linker.........: $LD $LDFLAGS $LIBS
  Bison..........: $BISON
  Flex...........: $LEX
"