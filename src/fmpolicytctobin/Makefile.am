# Our binary is produced here
bin_PROGRAMS = fmpolicytctobin 
fmpolicytctobin_SOURCES = fmpolicytctobin.cpp 

if DEBUG
AM_CXXFLAGS = -std=c++11 -Wno-unused-result -g -O3
else
AM_CXXFLAGS = -std=c++11 -Wno-unused-result -ffunction-sections -fdata-sections -O3
AM_LDFLAGS = -Wl,--gc-sections -Wl,--strip-all
endif
