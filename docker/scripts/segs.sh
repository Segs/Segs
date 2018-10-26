#!/bin/bash

set -e

if [ "$1" == "install_deps_ubuntu" ]; then
    apt-get update
    apt-get install -y cmake build-essential git qt5-default qtdeclarative5-dev
elif [ "$1" == "bootstrap" ]; then
    rm -rf bld
    mkdir bld
    cd bld 
    cmake ..
elif [ "$1" == "build" ]; then
    cd bld
    make -j4 $2
else
    echo "undefined command: $1"
fi
