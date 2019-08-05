#!/bin/bash

# Set env
set -eux
BIN_TARGET=$(realpath ./)
TAR_TARGET='Darwin_x86_64.tar.gz'
#export CC=clang
#export CXX=clang++

# Config
./autogen.sh
#./configure --enable-static --prefix=$BIN_TARGET
./configure --prefix=$BIN_TARGET
ln -s /usr/bin/shasum5.18 /usr/local/bin/sha1sum

# Build
make check
make install

# Package 
cd $BIN_TARGET/bin
tar -zcvf $BIN_TARGET/$TAR_TARGET ./
cd $BIN_TARGET
