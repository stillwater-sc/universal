#!/usr/bin/env bash

# script to create a Clang builder container to compile and test Universal with Clang
# Usage: ./clang_builder.sh BUILD_ALL
#        ./clang_builder.sh BUILD_APPLICATIONS
#        etc. for each available build target
# 
# to be target specific, the Dockerfile turns off the default configuration that builds the demonstration programs

# pick up the build target: default argument is set in the Dockerfile
TARGET=$1

# To turn off security features use:
# docker run --security-opt seccomp:unconfined ...
# example would be to strace an executable to find its dependencies

if [[ $# == 0 ]]; then
	docker build --force-rm -t stillwater/universal:test -f ../Dockerfile.testWithClang14 ..
else
      echo "docker build --force-rm -t stillwater/universal:test --build-arg target=$TARGET -f ../Dockerfile.testWithClang14 .."
	docker build --force-rm -t stillwater/universal:test --build-arg "target=$TARGET" -f ../Dockerfile.testWithClang14 ..
fi
