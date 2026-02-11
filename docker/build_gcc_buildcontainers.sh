#!/usr/bin/env bash
set -euo pipefail

# script to generate the docker build containers with specific compilers installed
# precondition: successful docker login so that the docker push can succeed
#
# NOTE: gcc:10.5+ base images (Debian Bookworm) have seccomp restrictions that prevent
# LZMA decompression during apt-get install. For these versions, we use
# docker run --security-opt seccomp=unconfined + docker commit as a workaround.

CMAKE_DIR="v4.2"
CMAKE_VERSION="4.2.1"

# Function to build GCC containers (standard docker build)
build_gcc_standard() {
    local version=$1
    echo "=== Building gcc${version}builder (standard) ==="
    docker build --memory=4g --target gcc${version}builder -t stillwater/builders:gcc${version}builder -f Dockerfile.gcc${version}builder .
    docker push stillwater/builders:gcc${version}builder
}

# Function to build GCC containers that need seccomp workaround (gcc 13+, Debian Bookworm)
build_gcc_seccomp_workaround() {
    local version=$1
    local gcc_tag=$2
    local container_name="gcc${version}builder-temp"
    echo "=== Building gcc${version}builder (seccomp workaround) ==="

    # Remove any existing temp container
    docker rm -f "$container_name" 2>/dev/null || true

    # Build using docker run with seccomp disabled
    docker run --security-opt seccomp=unconfined --name "$container_name" gcc:${gcc_tag} bash -c "
set -ex
rm -f /etc/apt/apt.conf.d/*clean*
apt-get update
apt-get install -y --no-install-recommends -V apt-utils build-essential curl vim gdb
apt-get clean
rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
gpg --keyserver hkp://keyserver.ubuntu.com --recv-keys C6C265324BBEBDC350B513D02D2CEF1034921684
curl -fsSLO --compressed https://cmake.org/files/${CMAKE_DIR}/cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz
curl -fsSLO https://cmake.org/files/${CMAKE_DIR}/cmake-${CMAKE_VERSION}-SHA-256.txt.asc
curl -fsSLO https://cmake.org/files/${CMAKE_DIR}/cmake-${CMAKE_VERSION}-SHA-256.txt
gpg --verify cmake-${CMAKE_VERSION}-SHA-256.txt.asc cmake-${CMAKE_VERSION}-SHA-256.txt
grep 'cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz' cmake-${CMAKE_VERSION}-SHA-256.txt | sha256sum -c -
tar xzf cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz -C /usr/local --strip-components=1 --no-same-owner
rm -rf cmake-${CMAKE_VERSION}*
useradd -ms /bin/bash stillwater
"

    # Commit container as image with proper metadata
    docker commit \
        --change 'LABEL maintainer="Theodore Omtzigt"' \
        --change 'USER stillwater' \
        --change 'WORKDIR /home/stillwater' \
        --change "ENV CONTAINER_ID=\"Stillwater GCC ${version} Builder\"" \
        --change 'CMD ["/usr/bin/env", "bash"]' \
        "$container_name" stillwater/builders:gcc${version}builder

    # Clean up temp container
    docker rm "$container_name"

    # Push to registry
    docker push stillwater/builders:gcc${version}builder
    echo "=== gcc${version}builder complete ==="
}

# GCC compiler containers
# GCC 9: standard docker build (Debian Buster)
build_gcc_standard 9

# GCC 10-13: need seccomp workaround (Debian Bookworm-based images)
build_gcc_seccomp_workaround 10 10.5
build_gcc_seccomp_workaround 11 11.5
build_gcc_seccomp_workaround 12 12.5
build_gcc_seccomp_workaround 13 13.4
