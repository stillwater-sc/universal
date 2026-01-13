#!/usr/bin/env bash
set -euo pipefail
export DOCKER_BUILDKIT=1
export COMPOSE_DOCKER_CLI_BUILD=1

# script to create a release container with a specific compiler
# Usage: ./build_release_container.sh [COMPILER]
# Examples:
#    ./build_release_container.sh              will create release with gcc13 (default)
#    ./build_release_container.sh clang17      will create release with clang17

# NOTE: clang15-18 require seccomp workaround due to Debian Bookworm restrictions
# in the silkeh/clang base images. This script handles that automatically.

MAJOR=v3
MINOR=91
VERSION="$MAJOR.$MINOR"

# List of compilers that need seccomp workaround
SECCOMP_COMPILERS="clang15 clang16 clang17 clang18"

needs_seccomp_workaround() {
    local compiler=$1
    for c in $SECCOMP_COMPILERS; do
        [[ "$c" == "$compiler" ]] && return 0
    done
    return 1
}

build_standard() {
    local compiler=$1
    local df="Dockerfile.$compiler"
    echo "=== Building release container with $compiler (standard docker build) ==="
    docker build --pull --force-rm -t "stillwater/universal:$VERSION" -t stillwater/universal:latest -f "$df" ..
}

build_with_seccomp_workaround() {
    local compiler=$1
    local version="${compiler#clang}"  # Extract version number (e.g., "17" from "clang17")
    local builder_image="stillwater/builders:${compiler}builder"
    local temp_builder="universal-${compiler}-builder-temp"
    local intermediate_image="universal-${compiler}-builder-intermediate"

    echo "=== Building release container with $compiler (seccomp workaround) ==="

    # Clean up any previous temp containers
    docker rm -f "$temp_builder" 2>/dev/null || true
    docker rmi -f "$intermediate_image" 2>/dev/null || true

    # Step 1: Build in container with seccomp disabled
    echo ">>> Step 1: Building in container with seccomp=unconfined..."

    # Run the build inside the container (mount source as read-only, copy inside)
    docker run --security-opt seccomp=unconfined \
        --name "$temp_builder" \
        -v "$(cd .. && pwd)":/src:ro \
        "$builder_image" \
        bash -c "
set -ex
cp -r /src /home/stillwater/universal
chown -R stillwater:stillwater /home/stillwater/universal
cd /home/stillwater/universal
# Clean any existing build artifacts from host
rm -rf build build_clang build_test
ls -la
cmake --version
mkdir -p build
cd build
cmake -DBUILD_ALL=ON -DBUILD_CMD_LINE_TOOLS=ON -DBUILD_DEMONSTRATION=OFF ..
make -j\$(nproc)
"

    # Commit the builder container as intermediate image
    echo ">>> Step 2: Committing builder container as intermediate image..."
    docker commit "$temp_builder" "$intermediate_image"
    docker rm "$temp_builder"

    # Step 3: Build release stage from ubuntu, copying from intermediate
    echo ">>> Step 3: Building release stage..."

    # Create a temporary Dockerfile for release stage only (in docker dir to avoid /tmp permission issues)
    local script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    local temp_dockerfile="${script_dir}/.Dockerfile.${compiler}.release.tmp"
    cat > "$temp_dockerfile" << 'RELEASE_EOF'
# RELEASE stage for clang builds requiring seccomp workaround
ARG INTERMEDIATE_IMAGE
FROM ${INTERMEDIATE_IMAGE} AS builder

# RELEASE stage
FROM ubuntu:24.04 AS release
LABEL maintainer="Theodore Omtzigt"

# remove problematic APT post-invoke hook from ubuntu:24.04 base image
RUN rm -f /etc/apt/apt.conf.d/*clean* \
    && apt-get update && apt-get install -y --no-install-recommends \
    make \
    && apt-get clean

# create and use user stillwater
RUN useradd -ms /bin/bash stillwater
USER stillwater

# copy cmake environment needed for testing
COPY --from=builder /usr/local/bin/cmake /usr/local/bin/
COPY --from=builder /usr/local/bin/ctest /usr/local/bin/
# copy information material
COPY --from=builder /home/stillwater/universal/*.md /home/stillwater/universal/
# copy the docs
COPY --chown=stillwater:stillwater --from=builder /home/stillwater/universal/docs /home/stillwater/universal/docs

# after building, the test executables are organized in the build directory under stillwater
COPY --chown=stillwater:stillwater --from=builder /home/stillwater/universal/build /home/stillwater/universal/build

WORKDIR /home/stillwater/universal/build

RELEASE_EOF

    # Add the CMD line with the correct compiler name
    echo "CMD [\"echo\", \"Universal Numbers Clang${version} Test Container\"]" >> "$temp_dockerfile"

    # Build release container from the intermediate image
    docker build --build-arg "INTERMEDIATE_IMAGE=$intermediate_image" \
        -t "stillwater/universal:$VERSION" \
        -t stillwater/universal:latest \
        -f "$temp_dockerfile" \
        ..

    # Cleanup
    rm -f "$temp_dockerfile"
    docker rmi "$intermediate_image" 2>/dev/null || true

    echo "=== Successfully built stillwater/universal:$VERSION with $compiler ==="
}

# Main script logic
if [[ $# -eq 0 ]]; then
    # default is to build with GCC 13
    COMPILER="gcc13"
else
    COMPILER=$1
fi

df="Dockerfile.$COMPILER"
[[ -f "$df" ]] || { echo "Dockerfile '$df' not found"; exit 1; }

if needs_seccomp_workaround "$COMPILER"; then
    build_with_seccomp_workaround "$COMPILER"
else
    build_standard "$COMPILER"
fi
