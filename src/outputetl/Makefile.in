all: outputetl 

outputetl: outputetl.cpp
	g++ -Wall -Wno-write-strings -Wno-unused-result -fexceptions  -O3 -o $@ outputetl.cpp -static

clean:
	-rm -f outputetl 

install:
	cp outputetl ~/bin

uninstall:
	-rm -f ~/bin/outputetl

.PHONY: all clean install

