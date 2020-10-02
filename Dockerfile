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
#RUN cmake -DBUILD_CMD_LINE_TOOLS=ON -DBUILD_EDUCATION=OFF -DBUILD_APPLICATIONS=OFF -DBUILD_PLAYGROUND=OFF .. && make
# full RUN statement to execute full regression test suite
RUN cmake -DBUILD_CI_CHECK=ON .. && make

# the command 'make test' is run as part of the CI test pipeline of the release container

# add a command that when you run the container without a command, it produces something meaningful
CMD ["echo", "Universal Numbers Library Builder Version 2.1.41"]


# RELEASE stage
#FROM alpine:latest as release    # hitting a segfault during startup of some playground programs
FROM debian:buster-slim as release
LABEL Theodore Omtzigt

#RUN apk add --no-cache libc6-compat libstdc++ make cmake bash gawk sed grep bc coreutils
RUN apt-get update && apt-get install -y --no-install-recommends \
    make \
    cmake \
    && apt-get clean
# create and use user stillwater
RUN useradd -ms /bin/bash stillwater
USER stillwater

# copy information material
COPY --from=builder /home/stillwater/universal/*.md /home/stillwater/universal/
# copy the docs
COPY --from=builder /home/stillwater/universal/docs /home/stillwater/universal/docs
# no need to copy CMakeLists.txt as you don't have a compiler in this container 
# and thus 'make -j 8' won't work anyway, only 'make test' which doesn't need CmakeLists.txt
#COPY --from=builder /home/stillwater/universal/CMakeLists.txt /home/stillwater/universal/

# after building, the test executables are organized in the build directory under stillwater
# ctest gets its configuration for CTestTestfile.cmake files. There is one at the root of the build tree
# and one for each directory that contains test executables.
# This way we can execute _make test_ in the test stage of the CI/CD pipeline as well as part of an interactive invocation
COPY --from=builder /home/stillwater/universal/build /home/stillwater/universal/build

# copy the CLI tools to /usr/local/bin so they are immediately usable
COPY --from=builder /home/stillwater/universal/build/tools/cmd/prop* /usr/local/bin/
COPY --from=builder /home/stillwater/universal/build/tools/cmd/comp* /usr/local/bin/
COPY --from=builder /home/stillwater/universal/build/tools/cmd/float2posit /usr/local/bin/

# double check we have all the executables of interest
#RUN find /home/stillwater/universal/build

# until we can figure out how to direct CodeShip to use this dir in the steps.yml file
WORKDIR /home/stillwater/universal/build 

# the command 'make test' is run as part of the CI test pipeline of this release container

CMD ["echo", "Universal Numbers Library Version 2.1.41"]
