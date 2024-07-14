#!/usr/bin/env bash

# script to generate the docker build containers with specific compilers installed
# precondition: successful docker login so that the docker push can succeed

# GCC compiler containers
docker build --target gcc9builder -t stillwater/builders:gcc9builder -f Dockerfile.gcc9builder .
docker push stillwater/builders:gcc9builder
docker build --target gcc10builder -t stillwater/builders:gcc10builder -f Dockerfile.gcc10builder .
docker push stillwater/builders:gcc10builder
docker build --target gcc11builder -t stillwater/builders:gcc11builder -f Dockerfile.gcc11builder .
docker push stillwater/builders:gcc11builder
docker build --target gcc12builder -t stillwater/builders:gcc12builder -f Dockerfile.gcc12builder .
docker push stillwater/builders:gcc12builder
docker build --target gcc13builder -t stillwater/builders:gcc13builder -f Dockerfile.gcc13builder .
docker push stillwater/builders:gcc13builder
