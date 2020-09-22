#!/bin/bash

set -e

if [ "$1" == "install_deps_ubuntu_bionic" ]; then
    if [ -f docker/scripts/source.list ]; then
        sudo cp -f docker/scripts/sources.list /etc/apt/sources.list
    fi
    sudo apt-get update
    sudo apt-get install -y cmake build-essential git qt5-default qtdeclarative5-dev libqt5websockets5-dev
elif [ "$1" == "bootstrap" ]; then
    rm -rf bld
    mkdir bld
    cd bld 
    cmake ..
elif [ "$1" == "rebootstrap" ]; then
    cd bld && cmake ..
elif [ "$1" == "build" ]; then
    cd bld
    make -j4 $2
else
    echo "undefined command: $1"
    exit 1
fi
