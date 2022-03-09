# Our binary is produced here
bin_PROGRAMS = eltcalc 
eltcalc_SOURCES = eltcalc.cpp main.cpp

if HAVE_PARQUET
eltcalc_CFLAGS = $(PARQUET_CFLAGS)
eltcalc_LDADD = $(PARQUET_LIBS) @LIBPTHREAD_LINK@
eltcalc_SOURCES += ../include/useparquet.cpp
endif
