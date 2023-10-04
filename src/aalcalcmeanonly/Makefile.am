# Our binary is produced here
bin_PROGRAMS = aalcalcmeanonly
aalcalcmeanonly_SOURCES = aalcalcmeanonly.cpp main.cpp

if HAVE_PARQUET
aalcalcmeanonly_CFLAGS= $(PARQUET_CFLAGS)
aalcalcmeanonly_LDADD = $(PARQUET_LIBS) @LIBPTHREAD_LINK@
aalcalcmeanonly_SOURCES += ../include/oasisparquet.cpp
endif
