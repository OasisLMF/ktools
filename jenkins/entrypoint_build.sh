#!/bin/bash

LOG_BUILD='/tmp/log/ktools-build.log'
mkdir -p $(dirname $LOG_BUILD)

./autogen.sh
./configure 
make check | tee /tmp/log/ktools-build.log

set +exu
KTOOLS_BUILD_FAILED=$(cat $LOG_BUILD | grep -ci 'Ktools build failed')
ALL_KTESTS_PASS=$(cat $LOG_BUILD | grep -ci 'All tests passed.')
set -exu

if [ $KTOOLS_BUILD_FAILED -ne 0 ] || [ $ALL_KTESTS_PASS -ne 1 ]; then
    echo "Error Detected in Ktools install"
    exit 1
else
    echo "Ktools installed successfully"
    exit 0
fi
