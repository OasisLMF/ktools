if DEBUG
AM_CFLAGS = -g3 -O0
AM_CXXFLAGS = -g3 -O0
else
AM_CFLAGS = -O3
AM_CXXFLAGS = -O3
endif

all: gulcalc
 
objects := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

#getmodel: getmodel.cpp
#	g++ -Wall -fexceptions -std=c++11 -g  -O0 -o $@ getmodel.cpp 


%.o: %.cpp
	g++ -Wall -Wno-unused-result -fexceptions  -std=c++11  AM_CFLAGS -c $<  


gulcalc : $(objects)
	g++ -o gulcalc  $(objects) -static



clean:
	-rm -f *.o
	-rm -f gulcalc 

install:
	cp gulcalc ~/bin

uninstall:
	-rm -f ~/bin/gulcalc

.PHONY: all clean install

