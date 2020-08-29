#
# multi-stage build
# docker build --target builder -t stillwater/universal:builder will just build a builder container
# docker build --target release -t stillwater/universal:release will just build a release container

# BUILDER stage
FROM gcc:7 as builder
LABEL Theodore Omtzigt
# create a cmake build environment
RUN apt-get update && apt-get install -y --no-install-recommends \
    apt-utils \
    build-essential \
    cmake \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# make certain you have a good .dockerignore file installed so that this layer isn't ginormous
COPY . /usr/src/universal
# print contextual information of the container at this state for visual inspection
RUN ls -la /usr/src/universal && cmake -version 

# set up the cmake/make environment to issue the build commands
RUN mkdir build 
WORKDIR /usr/src/universal/build
# test RUN statement to speed-up CI testing
#RUN cmake -DBUILD_CMD_LINE_TOOLS=ON -DBUILD_EDUCATION_EXAMPLES=OFF -DBUILD_APPLICATION_EXAMPLES=OFF -DBUILD_PLAYGROUND=OFF .. && make
RUN cmake -DBUILD_CI_CHECK=ON .. && make

# the command 'make test' is run as part of the test pipeline

# add a command that when you run the container without a command, it produces something meaningful
CMD ["echo", "Universal Numbers Library Builder Version 4.0.0"]


# RELEASE stage
#FROM alpine:latest as release    # hitting a segfault during startup of some playground programs
FROM debian:buster-slim as release
LABEL Theodore Omtzigt

#RUN apk add --no-cache libc6-compat libstdc++ make cmake bash gawk sed grep bc coreutils
RUN apt-get update && apt-get install -y --no-install-recommends \
    make \
    cmake \
    && apt-get clean

RUN useradd -ms /bin/bash stillwater
USER stillwater

# after building, the test executables are organized in the build directory under root
# ctest gets its configuration for CTestTestfile.cmake files. There is one at the root of the build tree
# and one for each directory that contains test executables.
# This way we can execute _make test_ in the test stage of the CI/CD pipeline
COPY --from=builder /usr/src/universal/build /home/stillwater/universal/build

# copy the CLI tools to /usr/local/bin so they are immediately usable
COPY --from=builder /usr/src/universal/build/tools/cmd/prop* /usr/local/bin/
COPY --from=builder /usr/src/universal/build/tools/cmd/comp* /usr/local/bin/
COPY --from=builder /usr/src/universal/build/tools/cmd/convert /usr/local/bin/

# double check we have all the executables of interest
#RUN find /home/stillwater/universal/build

# until we can figure out how to direct CodeShip to use this dir in the steps.yml file
WORKDIR /home/stillwater/universal/build 

CMD ["echo", "Universal Numbers Library Version 4.0.0"]
