#!/usr/bin/env bash

# script to generate the docker build containers with specific compilers installed
# precondition: successful docker login so that the docker push can succeed

# CLang compiler build containers
docker build --target clang11builder -t stillwater/universal:clang11builder -f Dockerfile.clang11builder .
docker push stillwater/universal:clang11builder
docker build --target clang12builder -t stillwater/universal:clang12builder -f Dockerfile.clang12builder .
docker push stillwater/universal:clang12builder
docker build --target clang13builder -t stillwater/universal:clang13builder -f Dockerfile.clang13builder .
docker push stillwater/universal:clang13builder
docker build --target clang14builder -t stillwater/universal:clang14builder -f Dockerfile.clang14builder .
docker push stillwater/universal:clang14builder
docker build --target clang15builder -t stillwater/universal:clang15builder -f Dockerfile.clang15builder .
docker push stillwater/universal:clang15builder
docker build --target clang16builder -t stillwater/universal:clang16builder -f Dockerfile.clang16builder .
docker push stillwater/universal:clang16builder
