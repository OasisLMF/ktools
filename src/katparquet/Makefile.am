# Our binary is produced here
bin_PROGRAMS = katparquet
katparquet_SOURCES = main.cpp

if HAVE_PARQUET
katparquet_CFLAGS = $(PARQUET_CFLAGS)
katparquet_LDADD = $(PARQUET_LIBS) @LIBPTHREAD_LINK@
katparquet_SOURCES += katparquet.cpp ../include/oasisparquet.cpp
endif
