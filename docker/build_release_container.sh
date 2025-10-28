#!/usr/bin/env bash
set -euo pipefail
export DOCKER_BUILDKIT=1
export COMPOSE_DOCKER_CLI_BUILD=1

# To turn off security features use:
# docker run --security-opt seccomp:unconfined ...
# example would be to strace an executable to find its dependencies

MAJOR=v3
MINOR=88
VERSION="$MAJOR.$MINOR"

if [[ $# -eq 0 ]]; then
	# default is to build with GCC 13
	docker build --pull --force-rm -t "stillwater/universal:$VERSION" -t stillwater/universal:latest -f "Dockerfile.gcc13" ..
else 
	# pick up the compiler to use
	COMPILER=$1
	df="Dockerfile.$COMPILER"
	[[ -f "$df" ]] || { echo "Dockerfile '$df' not found"; exit 1; }
	docker build --pull --force-rm -t "stillwater/universal:$VERSION" -t stillwater/universal:latest -f "$df" ..
fi
