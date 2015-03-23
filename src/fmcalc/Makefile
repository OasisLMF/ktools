all: fmcalc 

fmcalc: fmcalc.cpp
	g++ -Wall -Wno-write-strings -g -Wno-unused-result -fexceptions -std=c++11  -O3 -o $@ fmcalc.cpp -static

clean:
	-rm -f fmcalc 

install:
	cp fmcalc ~/bin

uninstall:
	-rm -f ~/bin/fmcalc


.PHONY: all clean install

