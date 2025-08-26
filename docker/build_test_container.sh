#!/usr/bin/env bash
set -euo pipefail
export DOCKER_BUILDKIT=1
export COMPOSE_DOCKER_CLI_BUILD=1

# script to create a test container with a specific compiler and build target
# Usage: ./build_test_container.sh COMPILER TARGET
# Examples:
#        ./build_test_container.sh gcc10                         will create a gcc10 dev environment with BUILD_ALL=ON
#	 ./build_test_container.sh clang13 BUILD_NUMBER_LNS     will create a clang13 dev environment with BUILD_NUMBER_LNS=ON

# To turn off security features use:
# docker run --security-opt seccomp:unconfined ...
# example would be to strace an executable to find its dependencies

MAJOR=v3
MINOR=86
VERSION="$MAJOR.$MINOR"

if [[ $# -eq 0 ]]; then
	# default is to build with Clang 15
	docker build --force-rm -t "stillwater/universal:${VERSION}-test" -t stillwater/universal:latest-test -f Dockerfile.clang15 ..
else 
	# pick up the compiler to use
	COMPILER=$1
	if [[ $# -eq 2 ]]; then
		TARGET=$2
	else
		TARGET=BUILD_ALL
	fi
	docker build --force-rm -t "stillwater/universal:${VERSION}-test" --build-arg "target=$TARGET" -f "Dockerfile.$COMPILER" ..
fi
