#!/bin/sh

# To turn off security features use:
# docker run --security-opt seccomp:unconfined ...
# example would be to strace an executable to find its dependencies

docker build --force-rm -t stillwater/universal:3.32.1 -t stillwater/universal:latest ..
