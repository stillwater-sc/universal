#!/usr/bin/env bash

# script to generate the docker build containers with specific compilers installed
# precondition: successful docker login so that the docker push can succeed

# GCC compiler containers
docker build --target gcc9builder -t stillwater/universal:gcc9builder -f Dockerfile.gcc9builder .
docker push stillwater/universal:gcc9builder
docker build --target gcc10builder -t stillwater/universal:gcc10builder -f Dockerfile.gcc10builder .
docker push stillwater/universal:gcc10builder
docker build --target gcc11builder -t stillwater/universal:gcc11builder -f Dockerfile.gcc11builder .
docker push stillwater/universal:gcc11builder
docker build --target gcc12builder -t stillwater/universal:gcc12builder -f Dockerfile.gcc12builder .
docker push stillwater/universal:gcc12builder
docker build --target gcc13builder -t stillwater/universal:gcc13builder -f Dockerfile.gcc13builder .
docker push stillwater/universal:gcc13builder
