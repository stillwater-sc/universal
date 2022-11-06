#
# Dockerfile to create the builder container for compiling and testing Universal
# docker build --target clang14builder -t stillwater/universal:clang14builder 

# BUILDER stage
FROM silkeh/clang:12 as clang12builder
LABEL Theodore Omtzigt
# create a build environment
RUN apt-get update && apt-get install -y --no-install-recommends -V \
    apt-utils \
    build-essential \
    curl \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# install a specific cmake version
RUN set -ex \
  && for key in C6C265324BBEBDC350B513D02D2CEF1034921684; do \
    gpg --keyserver hkp://keyserver.ubuntu.com --recv-keys "$key" ; \
  done

ENV CMAKE_DIR v3.23
ENV CMAKE_VERSION 3.23.1

RUN set -ex \
  && curl -fsSLO --compressed https://cmake.org/files/${CMAKE_DIR}/cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz \
  && curl -fsSLO https://cmake.org/files/${CMAKE_DIR}/cmake-${CMAKE_VERSION}-SHA-256.txt.asc \
  && curl -fsSLO https://cmake.org/files/${CMAKE_DIR}/cmake-${CMAKE_VERSION}-SHA-256.txt \
  && gpg --verify cmake-${CMAKE_VERSION}-SHA-256.txt.asc cmake-${CMAKE_VERSION}-SHA-256.txt \
  && grep "cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz\$" cmake-${CMAKE_VERSION}-SHA-256.txt | sha256sum -c - \
  && tar xzf cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz -C /usr/local --strip-components=1 --no-same-owner \
  && rm -rf cmake-${CMAKE_VERSION}*

# create and use user stillwater
RUN useradd -ms /bin/bash stillwater
USER stillwater

WORKDIR /home/stillwater

# add a command that when you run the container without a command, it produces something meaningful
ENV CONTAINER_ID "Universal Numbers Library Builder V3 Clang 12"
CMD ["/usr/bin/env", "bash"]
