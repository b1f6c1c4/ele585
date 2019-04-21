#                                               -*- Autoconf -*-
AC_PREREQ([2.64])

AC_INIT([sn-mpi],
        [m4_esyscmd_s([./get-version])],
        [b1f6c1c4@gmail.com])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror tar-ustar])
AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES(yes)])
m4_pattern_allow([AM_PROG_AR])
AM_PROG_AR
AX_CXX_COMPILE_STDCXX(17)

# Configure libtool
LT_INIT

#######################
### Checks

# Checks for programs.
CFLAGS='-O3'
CXXFLAGS='-O3'
AC_PROG_CXX
AM_PROG_CC_C_O
AC_PROG_LN_S

ACX_MPI([], [AC_MSG_ERROR(could not find mpi library for --with-mpi)])
MPI_CXXFLAGS=`mpic++ --showme:compile`
MPI_LDFLAGS=`mpic++ --showme:link`
AC_SUBST(MPI_CXXFLAGS)
AC_SUBST(MPI_LDFLAGS)

AC_SUBST([MORE_CXXFLAGS])
AC_SUBST([MORE_LDFLAGS])
AC_OUTPUT

cat <<EOF

------------------ Summary ------------------
 $PACKAGE_NAME version $PACKAGE_VERSION
  Prefix.........: $prefix
  CXX Compiler...: $CXX $MORE_CFLAGS $MORE_CXXFLAGS $CFLAGS $CXXFLAGS $MPI_CXXFLAGS $MPI_LDFLAGS
  Linker.........: $LD $MORE_LDFLAGS $LDFLAGS $LIBS
---------------------------------------------

Check the above options and compile with:
 ${MAKE-make}

EOF
