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
    	AC_WARN([no OCI library found in location $with_oci])
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
    	AC_WARN([no library for OCI found in location $with_oci_lib])
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
    	AC_WARN([no headers for OCI found in location $with_oci_headers])
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
    	AC_WARN([no OCI library found in location $with_ocilib])
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
    	AC_WARN([no libpq library found in location $with_libpq])
	fi
else
	AC_MSG_RESULT([not set])
fi
])
# MonetDB Mapi library location
AC_DEFUN([AC_MAPI_LOCATION],
[AC_ARG_WITH(mapi, [  --with-mapi], , )
AC_MSG_CHECKING([for user provided MonetDB mapi library location])
if test "x$with_mapi" != x; then
	if test -d "$with_mapi"; then
		LDFLAGS="$LDFLAGS -L$with_mapi"
    	CPPFLAGS="$CPPFLAGS -I$with_mapi/../include"
    	AC_MSG_RESULT([yes, $with_mapi])
	else
    	AC_MSG_RESULT([no])
    	AC_WARN([no libpq library found in location $with_mapi])
	fi
else
	AC_MSG_RESULT([not set])
fi
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
    	AC_WARN([no cplex library found in location $with_cplex])
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

AS_IF([test "x$enable_sqlite" == "xno"], [
  AC_DEFINE([DISABLE_SQLITE],[],[Disable support for SQLite backends])
  AC_MSG_RESULT([no])
],
[
  AC_MSG_RESULT([yes])
])
])
# compile with MonetDB support
AC_DEFUN([AC_ENABLE_MONETDB],
[
AC_MSG_CHECKING([should be compiled with support for MonetDB backends])
AC_ARG_ENABLE(monetdb, AS_HELP_STRING([--disable-monetdb],[Disable compilation with support for MonetDB backends]))

AS_IF([test "x$enable_monetdb" == "xno"], [
  AC_DEFINE([DISABLE_MONETDB],[],[Disable support for MonetDB backends])
  AC_MSG_RESULT([no])
],
[
  AC_MSG_RESULT([yes])
])
])
# compile with Postgres support
AC_DEFUN([AC_ENABLE_POSTGRES],
[
AC_MSG_CHECKING([should be compiled with support for Postgres backends])
AC_ARG_ENABLE(postgres, AS_HELP_STRING([--disable-postgres],[Disable compilation with support for Postgres backends]))

AS_IF([test "x$enable_postgres" == "xno"], [
  AC_DEFINE([DISABLE_POSTGRES],[],[Disable support for Postgres backends])
  AC_MSG_RESULT([no])
],
[
  AC_MSG_RESULT([yes])
])
])
# compile with Oracle support
AC_DEFUN([AC_ENABLE_ORACLE],
[
AC_MSG_CHECKING([should be compiled with support for Oracle backends])
AC_ARG_ENABLE(oracle, AS_HELP_STRING([--disable-oracle],[Disable compilation with support for Oracle backends]))

AS_IF([test "x$enable_oracle" == "xno"], [
  AC_DEFINE([DISABLE_ORACLE],[],[Disable support for Oracle backends])
  AC_MSG_RESULT([no])
],
[
  AC_MSG_RESULT([yes])
])
])
# enable/disable Java bindings
AC_DEFUN([AC_ENABLE_JAVA_BINDING],
[
AC_MSG_CHECKING([build with Java bindings including JDBC driver])
AC_ARG_ENABLE(java, AS_HELP_STRING([--disable-java],[Disable building of Java bindings]))

AS_IF([test "x$enable_java" == "xno"], [
  AC_DEFINE([DISABLE_JAVA],[],[Disable building of Java binding])
  AC_MSG_RESULT([no])
],
[
  AC_MSG_RESULT([yes])
])
AM_CONDITIONAL(JAVABINDING, test "x$enable_java" != "xno")
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
GPROM_CFLAGS="${GPROM_CFLAGS_DEBUG} ${GPROM_CFLAGS_OPT} ${GPROM_CFLAGS_ERR}"
AC_SUBST([GPROM_CFLAGS],[$GPROM_CFLAGS])
AC_SUBST([GPROM_CFLAGS_OPT],[$GPROM_CFLAGS_OPT])
AC_SUBST([GPROM_CFLAGS_ERROR],[$GPROM_CFLAGS_ERROR])
AC_MSG_RESULT($GPROM_CFLAGS)
])
################################################################################
# Init autoconf
AC_INIT([GProM],[1.0.0],[bglavic@iit.edu])
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
AC_PROG_CC_STDC
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
mysrcdir="$(pushd "${srcdir}" > /dev/null; pwd)"
AC_DEFINE_UNQUOTED([GPROM_TOP_SRCDIR],["${mysrcdir}"], ["absolute path to top source dir"])
GPROM_SRCDIR="$(pushd "${srcdir}" > /dev/null; pwd)"
################################################################################
# CHECK FOR PROGRAM BINARIES
AM_PROG_AR
AC_PROG_YACC
BISON_CMD="`AS_ECHO("$YACC") | sed 's/-y//g'`" 
AC_SUBST([BISON], [${BISON_CMD}])
AC_PROG_LEX
AC_CHECK_PROG([pandoc],[pandoc],[yes])
AM_CONDITIONAL([HAVE_PROG_PANDOC], [test "x$pandoc" = xyes])
AC_CHECK_PROG([rman],[rman],[yes])
AM_CONDITIONAL([HAVE_PROG_RMAN], [test "x$rman" = xyes])
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
AC_ENABLE_MONETDB
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
  [no], [],
  []
)
################################################################################
# Oracle access libraries (OCI and OCILIB)
AS_CASE(["$enable_oracle"], 
  [yes], [
# check if user has provided us with non-standard locations for OCI/OCILIB libraries
  AC_OCI_LOCATION
  AC_OCIH_LOCATION
  AC_OCI_LIB_LOCATION
  AC_OCILIB_LOCATION
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
  ],
  [no], [],
  [
# check if user has provided us with non-standard locations for OCI/OCILIB libraries
  AC_OCI_LOCATION
  AC_OCIH_LOCATION
  AC_OCI_LIB_LOCATION
  AC_OCILIB_LOCATION
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
  ]
])
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
# Postgres C client library (libpq)
AS_CASE(["$enable_postgres"], 
  [yes], [
# check if user has provided us with a non-standard location for Postgres's libpq library
  AC_LIBPQ_LOCATION
  AC_CHECK_HEADERS([libpq-fe.h])
# check for the libpq library
  AC_CHECK_LIB([pq],[PQconndefaults],[],[
    AC_MSG_ERROR([missing libpq library - if you want to compile without Postgres backend support use --disable-postgres])
  ])
  ],
  [no], [],
  [
# check if user has provided us with a non-standard location for Postgres's libpq library
  AC_LIBPQ_LOCATION
  AC_CHECK_HEADERS([libpq-fe.h])
# check for the libpq library
  AC_CHECK_LIB([pq],[PQconndefaults],[],[
    AC_MSG_ERROR([missing libpq library - if you want to compile without Postgres backend support use --disable-postgres])
  ])  
  ]
)
################################################################################
# SQlite library (libsqlite3)
# check for the sqlite3 library
AS_CASE(["$enable_sqlite"], 
	[yes], [
	AC_CHECK_LIB([sqlite3],[sqlite3_open],[],[
    	AC_MSG_ERROR([missing libsqlite3 library - if you want to compile without SQLite backend support use --disable-sqlite])
	])
	],
	[no], [], 
	[
	AC_CHECK_LIB([sqlite3],[sqlite3_open],[],[
    	AC_MSG_ERROR([missing libsqlite3 library - if you want to compile without SQLite backend support use --disable-sqlite])
	])
	]
)
################################################################################
# MonetDB C client library (mapi)
AS_CASE(["$enable_monetdb"], 
  [yes], [
# user has provided us with a non-standard location for MonetDB Mapi library
  AC_MAPI_LOCATION
  AC_CHECK_HEADERS([monetdb/mapi.h])
# check for the mapi library
  AC_CHECK_LIB([mapi],[mapi_unquote],[],[
    AC_MSG_ERROR([missing mapi library - if you want to compile without MonetDB backend support use --disable-monetdb])
  ])
  ],
  [no], [],
  [
# user has not provided us with a non-standard location for MonetDB Mapi library
  AC_MAPI_LOCATION
  AC_CHECK_HEADERS([monetdb/mapi.h])
# check for the libpq library
  AC_CHECK_LIB([mapi],[mapi_unquote],[],[
    AC_MSG_ERROR([missing mapi library - if you want to compile without MonetDB backend support use --disable-monetdb])
  ])  
  ]
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
	src/model/node/Makefile
	src/model/query_block/Makefile
	src/model/query_operator/Makefile
	src/model/rpq/Makefile
	src/model/set/Makefile
    src/operator_optimizer/Makefile
	src/parser/Makefile
	src/provenance_rewriter/Makefile
	src/provenance_rewriter/game_provenance/Makefile
	src/provenance_rewriter/semiring_combiner/Makefile
	src/provenance_rewriter/uncertainty_rewrites/Makefile
	src/provenance_rewriter/pi_cs_rewrites/Makefile
	src/provenance_rewriter/transformation_rewrites/Makefile
	src/provenance_rewriter/update_and_transaction/Makefile
	src/provenance_rewriter/summarization_rewrites/Makefile
	src/provenance_rewriter/xml_rewrites/Makefile
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
  Top src dir........: $GPROM_SRCDIR
  Debug Build........: ${enable_debug:=yes}
  Profiling..........: google perftools: ${enable_link_perftools:=no}, gprof: ${enable_gprof:=no} 
  Features...........: Assertions:${enable_assert:=no} Logging:${enable_logging:=yes} Timing:${enable_timing:=yes} cplex:${enable_cplex:=no}
  Backends supported.: Oracle - ${enable_oracle:=yes}, Postgres - ${enable_postgres:=yes}, SQLite - ${enable_sqlite:=yes}, MonetDB - ${enable_monetdb:=yes}
  Language bindings..: Java - ${enable_java:=yes}
  Readline support...: ${enable_readline:=yes}
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
