#!/bin/sh

# To turn off security features use:
# docker run --security-opt seccomp:unconfined ...
# example would be to strace an executable to find its dependencies

docker build -f DockerfileWithCompilerChecks --force-rm --target gcc5_compile_test -t stillwater/universal:gcc5_compile_test .
docker build -f DockerfileWithCompilerChecks --force-rm --target clang_compile_test -t stillwater/universal:clang_compile_test .
