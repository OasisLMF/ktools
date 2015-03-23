all: fmtocsv 

fmtocsv: fmtocsv.cpp
	g++ -Wall -Wno-write-strings -Wno-unused-result -fexceptions  -O3 -o $@ fmtocsv.cpp -static

clean:
	-rm -f fmtocsv 

install:
	cp fmtocsv ~/bin

uninstall:
	-rm -f ~/bin/fmtocsv


.PHONY: all clean install

