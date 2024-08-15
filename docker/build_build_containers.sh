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

# CLang compiler containers
docker build --target clang11builder -t stillwater/builders:clang11builder -f Dockerfile.clang11builder .
docker push stillwater/builders:clang11builder
docker build --target clang12builder -t stillwater/builders:clang12builder -f Dockerfile.clang12builder .
docker push stillwater/builders:clang12builder
docker build --target clang13builder -t stillwater/builders:clang13builder -f Dockerfile.clang13builder .
docker push stillwater/builders:clang13builder
docker build --target clang14builder -t stillwater/builders:clang14builder -f Dockerfile.clang14builder .
docker push stillwater/builders:clang14builder
docker build --target clang15builder -t stillwater/builders:clang15builder -f Dockerfile.clang15builder .
docker push stillwater/builders:clang15builder
docker build --target clang16builder -t stillwater/builders:clang16builder -f Dockerfile.clang16builder .
docker push stillwater/builders:clang16builder
docker build --target clang17builder -t stillwater/builders:clang17builder -f Dockerfile.clang17builder .
docker push stillwater/builders:clang17builder
docker build --target clang18builder -t stillwater/builders:clang18builder -f Dockerfile.clang18builder .
docker push stillwater/builders:clang18builder
