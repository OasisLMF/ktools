# Our binary is produced here
bin_PROGRAMS = aalcalc 
aalcalc_SOURCES = aalcalc.cpp main.cpp summaryindex.cpp

if HAVE_PARQUET
aalcalc_CFLAGS= $(PARQUET_CFLAGS)
aalcalc_LDADD = $(PARQUET_LIBS) @LIBPTHREAD_LINK@
aalcalc_SOURCES += ../include/useparquet.cpp
endif
