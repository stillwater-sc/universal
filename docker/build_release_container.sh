#!/usr/bin/env bash
set -euo pipefail
export DOCKER_BUILDKIT=1
export COMPOSE_DOCKER_CLI_BUILD=1

# script to create a release container with a specific compiler
# Usage: ./build_release_container.sh [COMPILER] [VERSION]
# Examples:
#    ./build_release_container.sh              will create release with gcc13 (default), auto-detect version
#    ./build_release_container.sh clang17      will create release with clang17, auto-detect version
#    ./build_release_container.sh gcc13 v3.94  will create release with gcc13, version v3.94

# NOTE: clang15-18 require seccomp workaround due to Debian Bookworm restrictions
# in the silkeh/clang base images. This script handles that automatically.

# Get script and repo directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Version detection: command-line > git tag > CMakeLists.txt
get_version() {
    # 1. Check for command-line argument
    if [[ -n "${1:-}" ]]; then
        echo "$1"
        return
    fi

    # 2. Check for git tag on HEAD
    local git_tag
    git_tag=$(git -C "$REPO_ROOT" describe --tags --exact-match HEAD 2>/dev/null || true)
    if [[ -n "$git_tag" ]]; then
        echo "$git_tag"
        return
    fi

    # 3. Extract from CMakeLists.txt
    local major minor
    major=$(grep 'set(UNIVERSAL_VERSION_MAJOR' "$REPO_ROOT/CMakeLists.txt" | grep -o '[0-9]\+')
    minor=$(grep 'set(UNIVERSAL_VERSION_MINOR' "$REPO_ROOT/CMakeLists.txt" | grep -o '[0-9]\+')
    echo "v${major}.${minor}"
}

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
cmake -DUNIVERSAL_BUILD_ALL=ON -DUNIVERSAL_BUILD_CMD_LINE_TOOLS=ON -DUNIVERSAL_BUILD_DEMONSTRATION=OFF ..
make -j\$(nproc)
"

    # Commit the builder container as intermediate image
    echo ">>> Step 2: Committing builder container as intermediate image..."
    docker commit "$temp_builder" "$intermediate_image"
    docker rm "$temp_builder"

    # Step 3: Build release stage from ubuntu, copying from intermediate
    echo ">>> Step 3: Building release stage..."

    # Create a temporary Dockerfile for release stage only (in docker dir to avoid /tmp permission issues)
    local temp_dockerfile="${SCRIPT_DIR}/.Dockerfile.${compiler}.release.tmp"
    cat > "$temp_dockerfile" << 'RELEASE_EOF'
# RELEASE stage for clang builds requiring seccomp workaround
ARG INTERMEDIATE_IMAGE
FROM ${INTERMEDIATE_IMAGE} AS builder

# RELEASE stage
FROM ubuntu:26.04 AS release
LABEL maintainer="Theodore Omtzigt"

# remove problematic APT post-invoke hook from base image
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
COMPILER="${1:-gcc13}"
VERSION_ARG="${2:-}"

# Get version (from arg, git tag, or CMakeLists.txt)
VERSION=$(get_version "$VERSION_ARG")

echo "Building release container:"
echo "  Compiler: $COMPILER"
echo "  Version:  $VERSION"

df="Dockerfile.$COMPILER"
[[ -f "$df" ]] || { echo "Dockerfile '$df' not found"; exit 1; }

if needs_seccomp_workaround "$COMPILER"; then
    build_with_seccomp_workaround "$COMPILER"
else
    build_standard "$COMPILER"
fi
