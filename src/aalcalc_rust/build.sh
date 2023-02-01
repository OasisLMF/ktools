#!/usr/bin/env bash

SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"
cd $SCRIPTPATH

cargo build --release

rm aalcalc_rust
cp ./target/release/aalcalc_rust ./aalcalc_rust
