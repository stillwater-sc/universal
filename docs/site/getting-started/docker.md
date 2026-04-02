---
title: Docker Quick Start
description: Experiment with Universal using Docker without building from source
sidebar:
  order: 2
---

The Docker image provides a ready-to-use C++ development environment with compilers, cmake, the Universal library headers pre-installed, and command-line tools (including `ucalc`) on the PATH.

```bash
docker pull stillwater/universal
docker run -it --rm stillwater/universal bash
```

Write and compile programs using Universal directly inside the container:

```cpp
// hello.cpp
#include <universal/number/posit/posit.hpp>
#include <iostream>
int main() {
    sw::universal::posit<16,2> p = 1.5;
    std::cout << p << '\n';
}
```

```bash
g++ -std=c++20 -o hello hello.cpp && ./hello
```

## Try the Command-Line Tools

```bash
# Interactive mixed-precision calculator
ucalc
ucalc> type posit32; show 1.5
ucalc> compare 0.1
ucalc> type bfloat16; precision

# Inspect IEEE-754 floating-point values
ieee 1.5

# One-shot ucalc commands
ucalc "type fp16; sweep sin(x) for x in [0, 3.14, 6]"
```

See [ucalc](/universal/ucalc/) for the full interactive calculator documentation, or [Command-Line Tools](../getting-started/command-line-tools/) for the standalone inspection utilities.

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

CMake presets handle compiler selection automatically. Choose `gcc-*` presets for GCC or `clang-*` presets for Clang -- no manual toolchain configuration needed.
