#!/usr/bin/env bash

# script to create a GCC builder container to compile and test Universal with GCC

# pick up the build target: default argument is set in the Dockerfile
TARGET=$1

# To turn off security features use:
# docker run --security-opt seccomp:unconfined ...
# example would be to strace an executable to find its dependencies

if [[ $# == 0 ]]; then
	docker build --force-rm -t stillwater/universal:test -f ../Dockerfile.testWithgcc10.3 ..
else
      echo "docker build --force-rm -t stillwater/universal:test --build-arg target=$TARGET -f ../Dockerfile.testWithgcc10.3 .."
	docker build --force-rm -t stillwater/universal:test --build-arg target=$TARGET -f ../Dockerfile.testWithgcc10.3 ..
fi
