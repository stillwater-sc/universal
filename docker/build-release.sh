#!/usr/bin/env bash
# Build and optionally push the Universal release Docker image.
#
# Usage:
#   ./docker/build-release.sh              # build with auto-detected version tag
#   ./docker/build-release.sh --push       # build and push to Docker Hub
set -euo pipefail

REPO="stillwater/universal"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# Auto-detect version from the latest git tag, falling back to CMakeLists.txt
VERSION=$(git -C "$ROOT_DIR" describe --tags --abbrev=0 2>/dev/null || \
          grep -oP 'project\(.*VERSION\s+\K[0-9]+\.[0-9]+\.[0-9]+' "$ROOT_DIR/CMakeLists.txt" || \
          echo "latest")

echo "Building $REPO:$VERSION"

docker build \
    -t "$REPO:$VERSION" \
    -t "$REPO:latest" \
    -f "$SCRIPT_DIR/Dockerfile.release" \
    "$ROOT_DIR"

if [[ "${1:-}" == "--push" ]]; then
    docker push "$REPO:$VERSION"
    docker push "$REPO:latest"
    echo "Pushed $REPO:$VERSION and $REPO:latest"
fi
