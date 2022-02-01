#!/bin/bash

# OSX compatibility
ln -s /usr/bin/shasum5.18 /usr/local/bin/sha1sum

# Set env
set -eux
BIN_TARGET=$(pwd)
TAR_TARGET='Darwin_x86_64.tar.gz'

# Config
./autogen.sh
./configure --enable-osx --enable-o3 --prefix=$BIN_TARGET 

# Build
make check
make install

# Package
cd $BIN_TARGET/bin
tar -zcvf $BIN_TARGET/$TAR_TARGET ./
cd $BIN_TARGET
