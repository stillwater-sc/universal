#!/usr/bin/env bash

# script to create a GCC build container to compile and test Universal with GCC
# Usage: ./build_dev_container.sh [COMPILER]
#        COMPILER is one of [gcc10, gcc11, gcc12, clang12, clang13, clang14]
# 
# To turn off security features use:
# docker run --security-opt seccomp:unconfined ...
# example would be to strace an executable to find its dependencies

if [[ $# == 0 ]]; then
	# default is GCC10
	docker build --force-rm -t stillwater/universal:gcc10.builder -f Dockerfile.gcc10.builder .
else
	# pick up the compiler to use
	COMPILER=$1
	docker build --force-rm -t "stillwater/universal:$COMPILER.builder" -f "Dockerfile.$COMPILER.builder" .
fi
