# Cross-compilation on Ubuntu

Setting up a cross-compilation environment for ARM and PowerPC on an x86 Ubuntu system is typically done by installing pre-built cross-compiler toolchains from the Ubuntu repositories. This is the recommended approach for standard Linux targets.

-----

## Installing Cross-Compilers for ARM and PowerPC

You can install the necessary GCC cross-compilers and their associated binary utilities (binutils) directly using the `apt` package manager.

### 1. Update Package List

First, ensure your package list is up-to-date:

```bash
sudo apt update
```

### 2. Install ARM Cross-Compilers

For **ARM** (32-bit and 64-bit Linux targets), you'll install the following packages:

| Architecture | Target Triple | Compiler Package | Command |
| :--- | :--- | :--- | :--- |
| **ARM 32-bit (Hard-Float)** | `arm-linux-gnueabihf` | `gcc-arm-linux-gnueabihf` | `sudo apt install gcc-arm-linux-gnueabihf` |
| **ARM 64-bit (AArch64)** | `aarch64-linux-gnu` | `gcc-aarch64-linux-gnu` | `sudo apt install gcc-aarch64-linux-gnu` |

You can install both with one command:

```bash
sudo apt install gcc-arm-linux-gnueabihf gcc-aarch64-linux-gnu
```

### 3. Install PowerPC Cross-Compilers

For **PowerPC** (32-bit and 64-bit Linux targets):

| Architecture | Target Triple | Compiler Package | Command |
| :--- | :--- | :--- | :--- |
| **PowerPC 32-bit** | `powerpc-linux-gnu` | `gcc-powerpc-linux-gnu` | `sudo apt install gcc-powerpc-linux-gnu` |
| **PowerPC 64-bit (Little-Endian)**| `powerpc64le-linux-gnu` | `gcc-powerpc64le-linux-gnu`| `sudo apt install gcc-powerpc64le-linux-gnu`|

Install them using:

```bash
sudo apt install gcc-powerpc-linux-gnu gcc-powerpc64le-linux-gnu
```

-----

## Using the Cross-Compilers

Once installed, the cross-compilers and tools will have a **prefix** corresponding to their target triple. You'll use these specific compiler names instead of the default `gcc` or `g++`.

### Compiling a Simple C File

For a source file named `hello.c`:

| Target Architecture | Compiler Command |
| :--- | :--- |
| **ARM 32-bit** | `arm-linux-gnueabihf-gcc hello.c -o hello_arm32` |
| **ARM 64-bit** | `aarch64-linux-gnu-gcc hello.c -o hello_arm64` |
| **PowerPC 32-bit**| `powerpc-linux-gnu-gcc hello.c -o hello_ppc32` |
| **PowerPC 64-bit**| `powerpc64le-linux-gnu-gcc hello.c -o hello_ppc64` |

You can verify the output file type using the `file` command:

```bash
file hello_arm64
# Expected output: ...ELF 64-bit LSB executable, **ARM aarch64**, version 1 (SYSV)...
```

### Compiling Universal (Cross-Platform Library)

When compiling a project like your **Universal** library, you'll need to pass the appropriate compiler and configuration options to its build system (e.g., Make, CMake, Autotools).

#### **Autotools (`./configure` and `make`)**

Set the cross-compiler for C, C++, and specify the host architecture using the `--host` flag:

```bash
# Example for ARM 64-bit
export CC=aarch64-linux-gnu-gcc
export CXX=aarch64-linux-gnu-g++
./configure --host=aarch64-linux-gnu
make
```

#### **CMake**

The recommended way for CMake is to use a **Toolchain File**. This file, often named `Toolchain-<ARCH>.cmake`, tells CMake about the compilers and system roots for the target.

**Example `Toolchain-arm64.cmake`:**

```cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Specify the cross compilers
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# Specify the sysroot (where target headers/libraries are)
# On Ubuntu, this is often the default path for the installed toolchain
# set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu) 
# set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

Then, configure your build:

```bash
mkdir build_arm64
cd build_arm64
cmake -DCMAKE_TOOLCHAIN_FILE=../Toolchain-arm64.cmake ..
make
```

## The cross-compilation jobs in CI

The RISC-V 64, POWER 64 LE and MinGW-w64 jobs in `.github/workflows/cmake.yml` build with the toolchain files in `cmake/toolchains/` and run the tests under QEMU user mode or Wine. They run inside prebuilt images that already contain the toolchains, so a job makes no `apt` calls. Installing the toolchains from the Ubuntu mirrors on every run used to be the most common reason these jobs failed: a slow mirror ran out the install's time limit before anything was built (#1817).

| Job | Image | Toolchain packages |
|-----|-------|--------------------|
| Linux RISC-V 64 (GCC cross) | `ghcr.io/stillwater-sc/universal-ci-cross:riscv64-<n>` | `g++-riscv64-linux-gnu`, `qemu-user-static` |
| Linux POWER 64 LE (GCC cross) | `ghcr.io/stillwater-sc/universal-ci-cross:ppc64le-<n>` | `g++-powerpc64le-linux-gnu`, `qemu-user-static` |
| Windows x64 (MinGW GCC cross) | `ghcr.io/stillwater-sc/universal-ci-cross:mingw-<n>` | `g++-mingw-w64-x86-64`, `wine`, `wine64` |

Every image is Ubuntu 24.04, the same release as the `ubuntu-latest` runners. Each also carries `git`, `ccache`, `cmake`, `make`, `zstd` and `gh`, which the job's steps need.

**Where they come from.** `docker/ci-cross/Dockerfile` has one stage per target. The `CI images` workflow (`.github/workflows/ci-images.yml`) builds them and pushes them to GHCR:

- every Monday, on main, so security fixes and point releases still arrive;
- on any push that changes the Dockerfile or the workflow;
- by hand, with `workflow_dispatch`.

**Changing an image:**

1. Edit `docker/ci-cross/Dockerfile`.
2. Bump the number `<n>` in that target's `container:` tag in `cmake.yml`.

The image workflow reads its tags from `cmake.yml`, so the two cannot drift apart. A branch never overwrites a tag that main uses. If the Dockerfile changed and the number did not, the image workflow fails and asks for the bump. When you push the branch, the image is published before the pull request's full tier needs it.

A pull request from a fork cannot publish images. If it changes the Dockerfile, a maintainer has to push the branch to this repository first.

**Reproducing a CI failure locally:**

```bash
docker run --rm -it -v "$PWD":/src -w /src ghcr.io/stillwater-sc/universal-ci-cross:ppc64le-1 bash
# inside the container
cmake -B build -DUNIVERSAL_BUILD_CI_LITE=ON -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/ppc64le-linux-gnu.cmake
cmake --build build -j4 && cd build && ctest -j4 --output-on-failure
```

For the MinGW image, run `wineboot --init && wineserver -w && wineserver -p && wineboot` before a parallel `ctest`. Otherwise the first tests race to start Wine and can fail with `chdir to /tmp/wine-...` (#1838).

You can also build the image yourself with `docker build --target ppc64le -t universal-ci-cross:ppc64le docker/ci-cross`.
