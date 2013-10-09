AC_INIT([tprov],[0.0.1],[bglavic@iit.edu])
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_MACRO_DIR([m4])
AC_SUBST([ACLOCAL_AMFLAGS],[-I m4])
################################################################################
# CHECK FOR C COMPILER AND FEATURES, BINUTILS, AND BISON + FLEX
AC_PROG_CC
AC_PROG_CC_STDC
# CAN INLINE FUNCTIONS?
AC_C_INLINE
AM_PROG_AR
AC_PROG_YACC
AC_PROG_LEX
# INIT LIBTOOL
LT_INIT
################################################################################
# DEFINTIONS OF HELPER FUNCTIONS FOR GETTING USER OPTIONS
AC_DEFUN([AC_OCI_LOCATION],
[AC_ARG_WITH(oci, [  --with-oci], , )
AC_MSG_CHECKING([for OCILOC])

# sensible default if nothing specified
if test "x$with_oci" = x; then
    with_oci=/usr/local/oracle
fi

if test -d "$with_oci"; then
	LDFLAGS="$LDFLAGS -L$with_oci"
    CPPFLAGS="$CPPFLAGS -I$with_oci/../rdbms/public"
    AC_MSG_RESULT([yes, $with_oci])
else
    AC_MSG_RESULT([no])
    AC_WARN([no library found in location $with_oci for oracle library])
fi
])

AC_DEFUN([AC_OCILIB_LOCATION],
[AC_ARG_WITH(ocilib, [  --with-ocilib], , )
AC_MSG_CHECKING([for OCILIBLOC])

# sensible default if nothing specified
if test "x$with_ocilib" = x; then
    with_ocilib=/usr/local/lib
fi

if test -d "$with_ocilib"; then
	LDFLAGS="$LDFLAGS -L$with_ocilib"
    CPPFLAGS="$CPPFLAGS -I$with_ocilib/../include"
    AC_MSG_RESULT([yes, $with_ocilib])
else
    AC_MSG_RESULT([no])
    AC_WARN([no library found in location $with_ocilib for ocilib library])
fi
])


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
################################################################################
# MEMORY ALLOCATION STUFF
AC_CHECK_HEADERS([malloc.h])
AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_FUNC_REALLOC
# CHECK FOR STD C FUNCS
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([strdup])
################################################################################
# Oracle access libraries (OCI and OCILIB)
# check if user has provided us with non-standard locations for OCI libraries
AC_OCI_LOCATION
AC_OCILIB_LOCATION
# check for the libraries
AC_CHECK_LIB([clntsh],[OCIInitialize],[],[AC_MSG_WARN([missing OCI library])])
AC_CHECK_LIB([occi],[OCIHandleFree],[],[AC_MSG_WARN([missing OCI library])])
AC_CHECK_LIB([ocilib],[OCI_Cleanup],[],[AC_MSG_WARN([missing OCILIB])])
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
	src/log/Makefile
	src/mem_manager/Makefile
	src/metadata_lookup/Makefile
	src/model/Makefile
	src/model/expression/Makefile
	src/model/helperfunction/Makefile
	src/model/list/Makefile
	src/model/node/Makefile
	src/model/query_block/Makefile
	src/model/query_operator/Makefile
	src/operator_optimizer/Makefile
	src/parser/Makefile
	src/provenance_rewriter/Makefile
	src/provenance_rewriter/pi_cs_rewrites/Makefile
	src/provenance_rewriter/transformation_rewrites/Makefile
	src/sql_serializer/Makefile
	test/Makefile
])
AC_OUTPUT
