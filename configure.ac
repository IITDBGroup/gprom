################################################################################
################################################################################
# DEFINTIONS OF HELPER FUNCTIONS FOR GETTING USER OPTIONS
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
# libpq library location
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
[AC_ARG_ENABLE(link-perftools, AS_HELP_STRING([--enable-perftools],[Link against google perftools library]))
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
AC_INIT([GProM],[0.0.1],[bglavic@iit.edu])
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE([-Wall foreign])
AC_CONFIG_MACRO_DIR([m4])
AC_SUBST([ACLOCAL_AMFLAGS],[-I m4])
################################################################################
# CHECK FOR C COMPILER AND FEATURES, BINUTILS, AND BISON + FLEX
AS_IF([test "x$CFLAGS" != "x"],
[
	userCFLAGS="$CFLAGS"	
],
[]
) 
AC_PROG_CC
# restore user CFLAGS
AS_IF([test "x$userCFLAGS" == "x"],
[
	CFLAGS=""
],
[
	CFLAGS="$userCFLAGS"
]
) 
AC_PROG_CC_STDC
# CAN INLINE FUNCTIONS?
AC_C_INLINE
AM_PROG_AR
AC_PROG_YACC
AC_PROG_LEX
# INIT LIBTOOL
LT_INIT
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
AC_CHECK_TYPES([ptrdiff_t])
################################################################################
# HEADERS
AC_CHECK_HEADERS([stddef.h])
AC_CHECK_HEADERS([libintl.h])
AC_CHECK_HEADERS([stdarg.h])
AC_CHECK_HEADERS([time.h])
AC_CHECK_HEADERS([limits.h])
AC_CHECK_HEADERS([regex.h])
AC_CHECK_HEADERS([setjmp.h])
AC_HEADER_ASSERT
################################################################################
# MEMORY ALLOCATION STUFF
AC_CHECK_HEADERS([malloc.h])
AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_FUNC_REALLOC
# CHECK FOR STD C FUNCS
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([strdup])
AC_CHECK_FUNCS([strcmp])
################################################################################
# Check which backends should be supported
AC_ENABLE_ORACLE
AC_ENABLE_POSTGRES
################################################################################
# Oracle access libraries (OCI and OCILIB)
# check if user has provided us with non-standard locations for OCI/OCILIB libraries
AC_OCI_LOCATION
AC_OCIH_LOCATION
AC_OCI_LIB_LOCATION
AC_OCILIB_LOCATION
# check for the libraries
AC_CHECK_LIB([clntsh],[OCIInitialize],[],[
AS_IF([test "x$enable_oracle" == "xno"], [
  AC_MSG_WARN([missing OCI library])
],
[
  AC_MSG_ERROR([missing OCI library - if you want to compile without Oracle backend support use --disable-oracle])
])
])
AC_CHECK_LIB([occi],[OCIHandleFree],[],[
AS_IF([test "x$enable_oracle" == "xno"], [
  AC_MSG_WARN([missing OCI library])
],
[
  AC_MSG_ERROR([missing OCI library - if you want to compile without Oracle backend support use --disable-oracle])
])
])
AC_CHECK_LIB([ocilib],[OCI_Cleanup],[],[
AS_IF([test "x$enable_oracle" == "xno"], [
  AC_MSG_RESULT([missing OCILIB library])
],
[
  AC_MSG_ERROR([missing OCILIB library - if you want to compile without Oracle backend support use --disable-oracle])
])
])
################################################################################
# Postgres access library (libpq)
# check if user has provided us with a non-standard location for Postgres's libpq library
AC_LIBPQ_LOCATION
AC_CHECK_HEADERS([libpq-fe.h])
# check for the libpq library
AC_CHECK_LIB([pq],[PQconndefaults],[],[
AS_IF([test "x$enable_postgres" == "xno"], [
  AC_MSG_WARN([missing libpq library])
],
[
  AC_MSG_ERROR([missing libpq library - if you want to compile without Oracle backend support use --disable-postgres])
])
])
################################################################################
# enable/disable features
AC_LOGGING_ACTIVE
AC_LINK_PERFTOOLS
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
	src/Makefile
	src/analysis_and_translate/Makefile
	src/command_line/Makefile
	src/configuration/Makefile
	src/execution/Makefile
	src/instrumentation/Makefile
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
	src/model/set/Makefile
    src/operator_optimizer/Makefile
	src/parser/Makefile
	src/provenance_rewriter/Makefile
	src/provenance_rewriter/game_provenance/Makefile
	src/provenance_rewriter/pi_cs_rewrites/Makefile
	src/provenance_rewriter/transformation_rewrites/Makefile
	src/provenance_rewriter/update_and_transaction/Makefile
	src/sql_serializer/Makefile
	src/utility/Makefile
	test/Makefile
])
AC_OUTPUT

echo "
  ($PACKAGE_NAME) version $PACKAGE_VERSION
  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  Prefix.............: $prefix
  Debug Build........: ${enable_debug:=yes}
  Link with GNU perftools: ${enable_link_perftools:=no}
  Features...........: Assertions:${enable_assert:=no} Logging:${enable_logging:=yes} Timing:${enable_timing:=yes}
  Backends supported.: Oracle - ${enable_oracle:=yes}, Postgres - ${enable_postgres:=yes}
  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
  CFLAGS ........: $CFLAGS
  CPPFLAGS ......: $CPPFLAGS
  GPROM_CFLAGS...: $GPROM_CFLAGS
  LDFLAGS........: $LDFLAGS
  C Compiler.....: $C $CPPFLAGS $GPROM_CFLAGS $CFLAGS
  Linker.........: $LD $LDFLAGS $LIBS
"