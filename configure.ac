#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([ktools],  m4_esyscmd([tr -d '\n' < VERSION]), [support@oasislmf.org])
#AM_INIT_AUTOMAKE([gnu])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_CONFIG_MACRO_DIR([m4])

AC_LANG([C])
AC_LANG([C++])

AC_MSG_CHECKING([Check for /usr/bin/md5sum ])
if [[ -f /usr/bin/md5sum ]];
then
	AC_MSG_RESULT([yes])
else
	AC_MSG_RESULT([no])
	AC_MSG_NOTICE([ /usr/bin/md5sum not found - cannot do make check])
fi

AC_ARG_ENABLE(debug,
AS_HELP_STRING([--enable-debug],
               [enable debugging, default: no]),
[case "${enableval}" in
             yes) debug=true ;;
             no)  debug=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])

dnl Enable static linking
AC_ARG_ENABLE([static],
    AS_HELP_STRING([--enable-static], [Enable static link]))


AS_IF([test "x$enable_static" = "xyes"], [
	LDFLAGS=-static
])


dnl Enable osx compatibility 
AC_ARG_ENABLE([osx],
    AS_HELP_STRING([--enable-osx], [Enable OSX build]))

AS_IF([test "x$enable_osx" = "xyes"], [
	CXXFLAGSX="-std=c++11 -D__unix  -Wno-unused-result -g"
],
[
	CXXFLAGSX="-std=c++11 -Wno-unused-result -fthreadsafe-statics -frtti -fno-omit-frame-pointer "
#	LDFLAGS="-Wl,--gc-sections -Wl,--strip-all"
]
)

dnl Optimization level for O3
AC_ARG_ENABLE([o3],
    AS_HELP_STRING([--enable-o3], [Enable O3 build]))

AS_IF([test "x$enable_o3" = "xyes"], [
	CXXFLAGSY="-O3"
],
[
	CXXFLAGSY="-O2"
]
)

dnl Enable osx compatibility (M1 arm) 
AC_ARG_ENABLE([arm],
    AS_HELP_STRING([--enable-osx-arm], [Set target for ARM]))

AS_IF([test "x$enable_osx_arm" = "xyes"], [
	#CXXFLAGSZ="-target arm64-apple-macos11"
	#CXXFLAGSZ="-target aarch64-apple-darwin"
	CXXFLAGSZ="-arch aarch64"
],
[
	CXXFLAGSZ=""
]
)

:${CXXFLAGS="$CXXFLAGSX $CXXFLAGSY $CXXFLAGSZ"}

AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")

# For debug
# AC_SUBST([AM_CXXFLAGS], [-std=c++11 -Wno-unused-result -g -O3])

#AC_SUBST([AM_CXXFLAGS], [-std=c++11 -Wno-unused-result -ffunction-sections -fdata-sections -O3])
#AC_SUBST([AM_LDFLAGS], [-Wl,--gc-sections -Wl,--strip-all])


#if [[ -f ./fred ]];
#then
#	:${CXXFLAGS="-std=c++11 -Wno-unused-result -ffunction-sections -fdata-sections -O3"}
#	:${LDFLAGS="-Wl,--gc-sections -Wl,--strip-all"}
#else
#	:${CXXFLAGS="-std=c++11 -D__unix -Wno-unused-result -g -O3"}	
#fi



AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])


# Checks for programs.
AC_PROG_CXX([c++ g++])
AC_PROG_CC([cc gcc])
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AX_CXX_COMPILE_STDCXX_11

AX_CHECK_ZLIB()
# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h inttypes.h memory.h stdlib.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_INT64_T


AC_CONFIG_FILES([
		 Makefile
		 ktest/Makefile
		 src/Makefile
                 src/cdftocsv/Makefile
		 src/crossvalidation/Makefile
                 src/eve/Makefile
		 src/damagebintocsv/Makefile
		 src/damagebintobin/Makefile
		 src/ensembletobin/Makefile
		 src/ensembletocsv/Makefile
                 src/fmcalc/Makefile
                 src/fmprofiletocsv/Makefile
                 src/fmprofiletobin/Makefile
                 src/fmpolicytctocsv/Makefile
                 src/fmpolicytctobin/Makefile
		 src/fmprogrammetobin/Makefile
		 src/fmprogrammetocsv/Makefile
                 src/fmtocsv/Makefile
		 src/quantiletocsv/Makefile
		 src/quantiletobin/Makefile
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
		 src/ordleccalc/Makefile
                 src/aalcalc/Makefile
                 src/aalcalctocsv/Makefile
                 src/vulnerabilitytobin/Makefile
                 src/vulnerabilitytocsv/Makefile
                 src/footprinttobin/Makefile
                 src/footprinttocsv/Makefile
                 src/periodstobin/Makefile
                 src/periodstocsv/Makefile
		 src/returnperiodtobin/Makefile
		 src/returnperiodtocsv/Makefile
		 src/kat/Makefile
		 src/footprintconv/Makefile
		 src/fmprofilebinconv/Makefile
		 src/validateitems/Makefile
		 src/validatedamagebin/Makefile
		 src/validatefootprint/Makefile
		 src/validateoasisfiles/Makefile
		 src/validatevulnerability/Makefile
		 src/summarycalctobin/Makefile
		 src/summaryindex/Makefile
		 src/kunit/Makefile
                 ])
AC_OUTPUT

rm -f  ktest/cwbld
rm -f  ktest/uwbld

