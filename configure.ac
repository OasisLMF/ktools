#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([ktools],  m4_esyscmd([tr -d '\n' < VERSION.txt]), [support@oasislmf.org])
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


dnl Enable static linking
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


:${CXXFLAGS="$CXXFLAGSX $CXXFLAGSY"}

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

# Check for pkg-config
AC_MSG_CHECKING([for pkg-config])
PKGCONFIG=no
pkg-config --version > /dev/null 2>&1 && PKGCONFIG=yes
AC_MSG_RESULT([$PKGCONFIG])
AS_IF([test "x$PKGCONFIG" = "xno"],
      [AC_MSG_ERROR([cannot find pkg-config. Please install pkg-config and run ./configure again.])
])

# Check for arrow and parquet libraries
# Provide option to disable search for parquet libraries for debugging
AC_ARG_ENABLE([parquet],
	      [AS_HELP_STRING[--disable-parquet], [disable search for Parquet libraries]],
	      [enable_parquet=no], [enable_parquet=yes])
AH_TEMPLATE([HAVE_PARQUET])
AS_IF([test "x$enable_parquet" = "xyes"],
      [AS_IF([test "x$enable_static" = "xyes"],
	     [PKG_CHECK_MODULES_STATIC([PARQUET], [arrow parquet],
				       [HAVE_PARQUET=yes], [HAVE_PARQUET=no])
             LIBPTHREAD_LINK="-Wl,--whole-archive -lpthread -Wl,--no-whole-archive"
	     ],
	     [PKG_CHECK_MODULES([PARQUET], [arrow parquet], [HAVE_PARQUET=yes],
				[HAVE_PARQUET=no])
             LIBPTHREAD_LINK=""
	     ])
      AC_SUBST(LIBPTHREAD_LINK)
      ])
AS_IF([test "$HAVE_PARQUET" = "yes"],
      [AC_DEFINE([HAVE_PARQUET], [1],
		 [Define to 1 if you have Apache arrow (-larrow) and parquet (-lparquet) libraries])
      ])
AM_CONDITIONAL([HAVE_PARQUET], [test "$HAVE_PARQUET" = "yes"])

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
		 src/placalc/Makefile
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
                 src/vulnerabilitytobin/Makefile
                 src/vulnerabilitytocsv/Makefile
                 src/footprinttobin/Makefile
                 src/footprinttocsv/Makefile
                 src/periodstobin/Makefile
                 src/periodstocsv/Makefile
		 src/returnperiodtobin/Makefile
		 src/returnperiodtocsv/Makefile
		 src/kat/Makefile
		 src/katparquet/Makefile
		 src/footprintconv/Makefile
		 src/fmprofilebinconv/Makefile
		 src/validatedamagebin/Makefile
		 src/validatefootprint/Makefile
		 src/validateoasisfiles/Makefile
		 src/validatevulnerability/Makefile
		 src/summarycalctobin/Makefile
		 src/summaryindex/Makefile
                 ])
AC_OUTPUT

rm -f  ktest/cwbld
rm -f  ktest/uwbld

