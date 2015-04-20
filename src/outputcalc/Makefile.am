all: outputcalc 

outputcalc: outputcalc.cpp
	g++ -Wall -Wno-write-strings -Wno-unused-result -fexceptions -std=c++11  -O3 -o $@ outputcalc.cpp -static

clean:
	-rm -f outputcalc 

install:
	cp outputcalc ~/bin

uninstall:
	-rm -f ~/bin/outputcalc

.PHONY: all clean install

