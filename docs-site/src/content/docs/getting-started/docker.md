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
