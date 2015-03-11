all: getmodel 

getmodel: getmodel.cpp
	g++ -Wall -Wno-write-strings -Wno-unused-result -fexceptions  -O3 -o $@ getmodel.cpp -static

clean:
	-rm -f getmodel 

install:
	cp getmodel ~/bin

uninstall:
	-rm -f ~/bin/getmodel

.PHONY: all clean install

