#!/usr/bin/env bash
#
# install script for cmake on linux-x86_64 environments
# requires sudo privilege

set -x

CMAKE_VERSION=4.2.1
CMAKE_MAJOR=4.2

mkdir temp
cd temp
curl -fsSLO --compressed https://cmake.org/files/v${CMAKE_MAJOR}/cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz 
curl -fsSLO https://cmake.org/files/v${CMAKE_MAJOR}/cmake-${CMAKE_VERSION}-SHA-256.txt.asc 
curl -fsSLO https://cmake.org/files/v${CMAKE_MAJOR}/cmake-${CMAKE_VERSION}-SHA-256.txt 
gpg --verify cmake-${CMAKE_VERSION}-SHA-256.txt.asc cmake-${CMAKE_VERSION}-SHA-256.txt 
grep "cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz\$" cmake-${CMAKE_VERSION}-SHA-256.txt | sha256sum -c - 
tar xzf cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz -C /usr/local --strip-components=1 --no-same-owner
cd ..
rm -rf temp
