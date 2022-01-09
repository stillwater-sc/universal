#
# multi-stage build
# docker build --target builder -t stillwater/universal:builder will just build a builder container
# docker build --target release -t stillwater/universal:release will just build a release container

# BUILDER stage
FROM gcc:10.3 as builder
LABEL Theodore Omtzigt
# create a build environment
RUN apt-get update && apt-get install -y --no-install-recommends -V \
    apt-utils \
    build-essential \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# install a specific cmake version
RUN set -ex \
  && for key in C6C265324BBEBDC350B513D02D2CEF1034921684; do \
    gpg --keyserver hkp://keyserver.ubuntu.com --recv-keys "$key" ; \
  done

ENV CMAKE_VERSION 3.20.5

RUN set -ex \
  && curl -fsSLO --compressed https://cmake.org/files/v3.20/cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz \
  && curl -fsSLO https://cmake.org/files/v3.20/cmake-${CMAKE_VERSION}-SHA-256.txt.asc \
  && curl -fsSLO https://cmake.org/files/v3.20/cmake-${CMAKE_VERSION}-SHA-256.txt \
  && gpg --verify cmake-${CMAKE_VERSION}-SHA-256.txt.asc cmake-${CMAKE_VERSION}-SHA-256.txt \
  && grep "cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz\$" cmake-${CMAKE_VERSION}-SHA-256.txt | sha256sum -c - \
  && tar xzf cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz -C /usr/local --strip-components=1 --no-same-owner \
  && rm -rf cmake-${CMAKE_VERSION}*

# create and use user stillwater
RUN useradd -ms /bin/bash stillwater
USER stillwater

# make certain you have a good .dockerignore file installed so that this layer isn't ginormous
COPY --chown=stillwater:stillwater . /home/stillwater/universal
# print contextual information of the container at this state for visual inspection
RUN ls -la /home/stillwater/universal && cmake -version 

# set up the cmake/make environment to issue the build commands
RUN mkdir -p /home/stillwater/universal/build 
WORKDIR /home/stillwater/universal/build
# test RUN statement to speed-up CI testing
#RUN cmake -DBUILD_VALIDATION_HW=ON -DBUILD_CMD_LINE_TOOLS=ON -DBUILD_DEMONSTRATION=OFF .. && make
# full RUN statement to execute full regression test suite
# default is SANITY regression level: -DBUILD_REGRESSION_LEVEL_[1,2,3,4]=ON 
# or -DBUILD_REGRESSION_STRESS=ON for stress testing
RUN cmake -DBUILD_ALL=ON .. && make

# the command 'make test' is run as part of the CI test pipeline of the release container

# add a command that when you run the container without a command, it produces something meaningful
CMD ["echo", "Universal Numbers Library Builder Version 3.51.1"]


# RELEASE stage
#FROM alpine:latest as release    # hitting a segfault during startup of some playground programs
#FROM debian:buster-slim as release
FROM ubuntu:20.04 as release
LABEL Theodore Omtzigt

#RUN apk add --no-cache libc6-compat libstdc++ cmake make bash gawk sed grep bc coreutils
RUN apt-get update && apt-get update -y && apt-get install -y --no-install-recommends \
    make \
    && apt-get clean
# create and use user stillwater
RUN useradd -ms /bin/bash stillwater
USER stillwater

# copy cmake enviroment needed for testing
COPY --from=builder /usr/local/bin/cmake /usr/local/bin/
COPY --from=builder /usr/local/bin/ctest /usr/local/bin/
# copy information material
COPY --from=builder /home/stillwater/universal/*.md /home/stillwater/universal/
# copy the docs
COPY --chown=stillwater:stillwater --from=builder /home/stillwater/universal/docs /home/stillwater/universal/docs
# no need to copy CMakeLists.txt as you don't have a compiler in this container 
# and thus 'make -j 8' won't work anyway, only 'make test' which doesn't need CmakeLists.txt
#COPY --from=builder /home/stillwater/universal/CMakeLists.txt /home/stillwater/universal/

# after building, the test executables are organized in the build directory under stillwater
# ctest gets its configuration for CTestTestfile.cmake files. There is one at the root of the build tree
# and one for each directory that contains test executables.
# This way we can execute _make test_ in the test stage of the CI/CD pipeline as well as part of an interactive invocation
COPY --chown=stillwater:stillwater --from=builder /home/stillwater/universal/build /home/stillwater/universal/build

# copy the CLI tools to /usr/local/bin so they are immediately usable
COPY --from=builder /home/stillwater/universal/build/tools/cmd/areal /usr/local/bin/
COPY --from=builder /home/stillwater/universal/build/tools/cmd/double /usr/local/bin/
COPY --from=builder /home/stillwater/universal/build/tools/cmd/fixpnt /usr/local/bin/
COPY --from=builder /home/stillwater/universal/build/tools/cmd/float /usr/local/bin/
COPY --from=builder /home/stillwater/universal/build/tools/cmd/float2posit /usr/local/bin/
COPY --from=builder /home/stillwater/universal/build/tools/cmd/ieee /usr/local/bin/
COPY --from=builder /home/stillwater/universal/build/tools/cmd/lns /usr/local/bin/
COPY --from=builder /home/stillwater/universal/build/tools/cmd/longdouble /usr/local/bin/
COPY --from=builder /home/stillwater/universal/build/tools/cmd/plimits /usr/local/bin/
COPY --from=builder /home/stillwater/universal/build/tools/cmd/posit /usr/local/bin/
COPY --from=builder /home/stillwater/universal/build/tools/cmd/prop* /usr/local/bin/
COPY --from=builder /home/stillwater/universal/build/tools/cmd/signedint /usr/local/bin/
COPY --from=builder /home/stillwater/universal/build/tools/cmd/unsignedint /usr/local/bin/
COPY --from=builder /home/stillwater/universal/build/validation/hw/* /usr/local/bin/

# double check we have all the executables of interest
#RUN find /home/stillwater/universal/build

# until we can figure out how to direct CodeShip to use this dir in the steps.yml file
WORKDIR /home/stillwater/universal/build 

# the command 'make test' is run as part of the CI test pipeline of this release container

CMD ["echo", "Universal Numbers Library Version 3.51.1"]
