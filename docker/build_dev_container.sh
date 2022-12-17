#!/usr/bin/env bash

# script to create a build container to compile and test Universal with gcc or clang
# Usage: ./build_dev_container.sh [COMPILER]
#        COMPILER is one of [gcc10, gcc11, gcc12, clang12, clang13, clang14]
# 
# To turn off security features use:
# docker run --security-opt seccomp:unconfined ...
# example would be to strace an executable to find its dependencies

if [[ $# == 0 ]]; then
	# default is clang++14
	docker build --force-rm -t stillwater/universal:clang14.builder -f Dockerfile.clang14.builder .
else
	# pick up the compiler to use
	COMPILER=$1
	docker build --force-rm -t "stillwater/universal:$COMPILER.builder" -f "Dockerfile.$COMPILER.builder" .
fi
