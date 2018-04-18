FROM gcc:7
MAINTAINER Theodore Omtzigt

RUN apt-get update && apt-get install -y build-essential apt-utils \
    cmake \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# make certain you have a good .dockerignore file installed so that this tree is >1GB
COPY . /usr/src/universal
WORKDIR /usr/src/universal
RUN ls -la /usr/src/universal && cmake -version 
RUN mkdir build && mkdir bin
RUN cd build && pwd && cmake .. && make && make test 
