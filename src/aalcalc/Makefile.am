# Our binary is produced here
bin_PROGRAMS = aalcalc 
aalcalc_SOURCES = aalcalc.cpp main.cpp summaryindex.cpp

if HAVE_PARQUET
aalcalc_CFLAGS= $(PARQUET_CFLAGS)
aalcalc_LDADD = $(PARQUET_LIBS) @WL_WHOLE_ARCHIVE_PREFIX@ @LIBPTHREAD_LINK@ @WL_NO_WHOLE_ARCHIVE_SUFFIX@
aalcalc_SOURCES += ../include/useparquet.cpp
endif
