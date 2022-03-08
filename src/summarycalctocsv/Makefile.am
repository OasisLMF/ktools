# Our binary is produced here
bin_PROGRAMS = summarycalctocsv 
summarycalctocsv_SOURCES = summarycalctocsv.cpp

if HAVE_PARQUET
summarycalctocsv_CFLAGS = $(PARQUET_CFLAGS)
summarycalctocsv_LDADD = $(PARQUET_LIBS) @WL_WHOLE_ARCHIVE_PREFIX@ @LIBPTHREAD_LINK@ @WL_NO_WHOLE_ARCHIVE_SUFFIX@
summarycalctocsv_SOURCES += ../include/useparquet.cpp
endif
