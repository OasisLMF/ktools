all: cdftocsv 

cdftocsv: cdftocsv.cpp
	g++ -Wall -Wno-write-strings -Wno-unused-result -fexceptions  -O3 -o $@ cdftocsv.cpp -static

clean:
	-rm -f cdftocsv 

install:
	cp cdftocsv ~/bin

uninstall:
	-rm -f ~/bin/cdftocsv

.PHONY: all clean install

