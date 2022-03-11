# Our binary is produced here
bin_PROGRAMS = pltcalc 
pltcalc_SOURCES = pltcalc.cpp main.cpp 

if HAVE_PARQUET
pltcalc_CFLAGS = $(PARQUET_CFLAGS)
pltcalc_LDADD = $(PARQUET_LIBS) @LIBPTHREAD_LINK@
pltcalc_SOURCES += ../include/useparquet.cpp
endif
