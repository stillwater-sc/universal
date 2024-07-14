#!/usr/bin/env bash

# script to create a build container to compile and test Universal with gcc or clang
# Usage: ./build_dev_container.sh [COMPILER]
#        COMPILER is one of [gcc10, gcc11, gcc12, clang12, clang13, clang14 clang15 clang16]
# 
# To turn off security features use:
# docker run --security-opt seccomp:unconfined ...
# example would be to strace an executable to find its dependencies

if [[ $# == 0 ]]; then
	# default is clang++15
	docker build --force-rm -t stillwater/builders:clang15builder -f Dockerfile.clang15builder .
else
	# pick up the compiler to use, one of [gcc9 gcc10 gcc11 gcc12 clang11 clang12 clang13 clang14 clang15 clang16]
	COMPILER=$1
	docker build --force-rm -t "stillwater/builders:${COMPILER}builder" -f "Dockerfile.${COMPILER}builder" .
fi
