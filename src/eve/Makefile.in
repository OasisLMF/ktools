all: eve 

eve: eve.cpp
	g++ -Wall -Wno-write-strings -Wno-unused-result  -fexceptions   -O3 -o $@ eve.cpp -static

clean:
	-rm -f eve 

install:
	cp eve ~/bin

uninstall:
	-rm -f ~/bin/eve

.PHONY: all clean install

