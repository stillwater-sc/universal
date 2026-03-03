---
title: Docker Quick Start
description: Experiment with Universal using Docker without building from source
sidebar:
  order: 2
---

If you want to experiment with Universal's number system tools and test suites without cloning and building the source code, use the Docker container:

```bash
docker pull stillwater/universal
docker run -it --rm stillwater/universal bash
```

Once inside the container, all tools and test executables are pre-built and ready to use:

```text
stillwater@b3e6708fd732:~/universal/build$ ls
CMakeCache.txt       Makefile      cmake-uninstall.cmake  playground  universal-config-version.cmake
CMakeFiles           applications  cmake_install.cmake    tests       universal-config.cmake
CTestTestfile.cmake  c_api         education              tools       universal-targets.cmake
```

## Try the Command-Line Tools

```bash
# Inspect IEEE-754 floating-point values
ieee 1.5

# Explore posit representations
posit 1.5

# Compare number systems
propieee 1.5
```

See [Command-Line Tools](../getting-started/command-line-tools/) for a full reference.

## Development Container

For a full development environment with both GCC and Clang pre-installed, use the VS Code Dev Container.

### Prerequisites

- [Docker](https://www.docker.com/)
- [VS Code](https://code.visualstudio.com/) with the [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension

### Getting started

1. Clone the repository and open it in VS Code.
2. When prompted, select **Reopen in Container** (or run the command `Dev Containers: Reopen in Container`).
3. Once the container builds, open the CMake Tools preset selector and choose a preset such as `gcc-debug` or `clang-release`.
4. Build and debug as usual with the CMake Tools extension.

### Switching compilers

CMake presets handle compiler selection automatically. Choose `gcc-*` presets for GCC or `clang-*` presets for Clang — no manual toolchain configuration needed.
