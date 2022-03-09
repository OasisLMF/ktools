# Our binary is produced here
bin_PROGRAMS = eltcalc 
eltcalc_SOURCES = eltcalc.cpp main.cpp

if HAVE_PARQUET
eltcalc_CFLAGS = $(PARQUET_CFLAGS)
eltcalc_LDADD = $(PARQUET_LIBS) @WL_WHOLE_ARCHIVE_PREFIX@ @LIBPTHREAD_LINK@ @WL_NO_WHOLE_ARCHIVE_SUFFIX@
eltcalc_SOURCES += ../include/useparquet.cpp
endif
