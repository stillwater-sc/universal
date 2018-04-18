FROM gcc:7
MAINTAINER Theodore Omtzigt

RUN apt-get update && apt-get install -y build-essential apt-utils \
    cmake \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# make certain you have a good .dockerignore file installed so that this tree is >1GB
COPY . /usr/src/universal
WORKDIR /usr/src/universal

# print contextual information of the container at this state
RUN ls -la /usr/src/universal && cmake -version 
# set up the cmake/make environment
RUN mkdir build 
RUN cd build && pwd && cmake .. && make

# actual command 'make test' is run as part of the test pipeline
