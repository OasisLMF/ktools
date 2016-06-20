#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ktools], [0.4.2], [support@oasislmf.org])
AM_INIT_AUTOMAKE

AC_MSG_CHECKING([Check for /usr/bin/diff ])
if [[ -f /usr/bin/diff ]];
then
	AC_MSG_RESULT([yes])
else
	AC_MSG_RESULT([no])
	AC_MSG_NOTICE([ /usr/bin/diff not found - cannot do make check])
fi




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


AC_CONFIG_FILES([
		 Makefile
		 ktest/Makefile
		 src/Makefile
                 src/cdftocsv/Makefile
                 src/eve/Makefile
		 src/damagebintocsv/Makefile
		 src/damagebintobin/Makefile
                 src/fmcalc/Makefile
                 src/fmprofiletocsv/Makefile
                 src/fmprofiletobin/Makefile
                 src/fmpolicytctocsv/Makefile
                 src/fmpolicytctobin/Makefile
		 src/fmprogrammetobin/Makefile
		 src/fmprogrammetocsv/Makefile
                 src/fmtocsv/Makefile
                 src/randtocsv/Makefile
                 src/randtobin/Makefile
                 src/evetocsv/Makefile
                 src/evetobin/Makefile
                 src/gultobin/Makefile
                 src/getmodel/Makefile
                 src/gulcalc/Makefile
                 src/gultocsv/Makefile
                 src/itemtobin/Makefile
                 src/itemtocsv/Makefile
                 src/coveragetobin/Makefile
                 src/coveragetocsv/Makefile
                 src/summarycalc/Makefile
                 src/fmxreftocsv/Makefile
                 src/fmxreftobin/Makefile
                 src/summarycalctocsv/Makefile
                 src/eltcalc/Makefile
                 src/pltcalc/Makefile
                 src/gulsummaryxreftobin/Makefile
                 src/gulsummaryxreftocsv/Makefile
                 src/fmsummaryxreftobin/Makefile
                 src/fmsummaryxreftocsv/Makefile
                 src/occurrencetobin/Makefile
                 src/occurrencetocsv/Makefile
                 src/leccalc/Makefile
                 src/aalcalc/Makefile
                 src/aalcalctocsv/Makefile
                 src/aalsummary/Makefile
                 src/vulnerabilitytobin/Makefile
                 src/vulnerabilitytocsv/Makefile
                 src/footprinttobin/Makefile
                 src/footprinttocsv/Makefile
                 ])
AC_OUTPUT

rm -f  ktest/cwbld
rm -f  ktest/uwbld

