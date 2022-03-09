# Our binary is produced here
bin_PROGRAMS = summarycalctocsv 
summarycalctocsv_SOURCES = summarycalctocsv.cpp

if HAVE_PARQUET
summarycalctocsv_CFLAGS = $(PARQUET_CFLAGS)
summarycalctocsv_LDADD = $(PARQUET_LIBS) @LIBPTHREAD_LINK@
summarycalctocsv_SOURCES += ../include/useparquet.cpp
endif
