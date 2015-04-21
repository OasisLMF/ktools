all: fmdatatocsv 

fmdatatocsv: fmdatatocsv.cpp
	g++ -Wall -Wno-write-strings -Wno-unused-result -fexceptions  -O3 -o $@ fmdatatocsv.cpp -static

clean:
	-rm -f fmdatatocsv 

install:
	cp fmdatatocsv ~/bin

uninstall:
	-rm -f ~/bin/fmdatatocsv


.PHONY: all clean install

