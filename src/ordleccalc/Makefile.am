# Our binary is produced here
bin_PROGRAMS = ordleccalc 
ordleccalc_SOURCES = ../leccalc/leccalc.cpp ../leccalc/aggreports.cpp main.cpp

if HAVE_PARQUET
ordleccalc_CFLAGS = $(PARQUET_CFLAGS)
ordleccalc_CPPFLAGS = -DORD_OUTPUT
ordleccalc_LDADD = $(PARQUET_LIBS)
ordleccalc_SOURCES += ../include/useparquet.cpp
endif
