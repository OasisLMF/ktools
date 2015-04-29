#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ktools], [0.4.0], [support@oasislmf.org])
AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

: ${CXXFLAGS="-std=c++11 -Wno-unused-result -g -O3"}

LDFLAGS=-static

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AX_CXX_COMPILE_STDCXX_11

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h inttypes.h memory.h stdlib.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT64_T

# Checks for library functions.
AC_FUNC_FSEEKO
AC_CHECK_FUNCS([sqrt])

AC_CONFIG_FILES([Makefile
		 src/Makefile
                 src/cdftocsv/Makefile
                 src/eve/Makefile
                 src/fmcalc/Makefile
                 src/fmdatatocsv/Makefile
                 src/fmtocsv/Makefile
                 src/getmodel/Makefile
                 src/gulcalc/Makefile
                 src/gultocsv/Makefile
                 src/outputcalc/Makefile])
AC_OUTPUT
