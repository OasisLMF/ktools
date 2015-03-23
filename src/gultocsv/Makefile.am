all: gultocsv 

gultocsv: gultocsv.cpp
	g++ -Wall -Wno-write-strings -Wno-unused-result -fexceptions  -O3 -o $@ gultocsv.cpp -static

clean-local:
	-rm -f gultocsv 

uninstall-local:
	-rm -f ~/bin/gultocsv


.PHONY: all clean install

