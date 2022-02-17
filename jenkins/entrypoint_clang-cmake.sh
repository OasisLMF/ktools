#!/bin/bash

export CC=/usr/local/bin/clang
export CXX=/usr/local/bin/clang++

LOG_BUILD='/tmp/log/ktools-cmake-build.log'
ARCH_TARGET=$(uname --m)
BIN_TARGET='/var/ktools'
TAR_TARGET="/var/ktools/tar/Linux-cmake_$ARCH_TARGET.tar.gz"
mkdir -p $(dirname $LOG_BUILD)
mkdir -p $(dirname $TAR_TARGET)

set -e
rm -rf CMakeCache.txt

# Cmake Build 
/bin/cmake .
make | tee /tmp/log/ktools-cmake-build.log
make test | tee /tmp/log/ktools-cmake-build.log


set +exu
ALL_KTESTS_PASS=$(cat $LOG_BUILD | grep -ci '100% tests passed')
set -exu

if [ $ALL_KTESTS_PASS -ne 1 ]; then
    echo "Error Detected in Ktools install"
    exit 1
else
    echo "Ktools installed successfully"

#    # Create tar
#    make install 
#    cd $BIN_TARGET/bin
#    tar -zcvf $TAR_TARGET ./
#    exit 0
fi

