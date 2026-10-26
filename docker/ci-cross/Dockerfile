# Universal Numbers Library -- toolchain images for the cross-compilation CI jobs (#1817).
#
# The RISC-V, POWER and MinGW jobs in .github/workflows/cmake.yml used to install their
# toolchains from the Ubuntu mirrors on every run, and a slow mirror failed the job before
# anything was built. These images carry the toolchains preinstalled, so the jobs make no
# apt calls at all. .github/workflows/ci-images.yml rebuilds them weekly and on a change to
# this file, and pushes them to ghcr.io/stillwater-sc/universal-ci-cross:<target>-<version>.
#
# One stage per target:
#   docker build --target ppc64le -t universal-ci-cross:ppc64le docker/ci-cross
#
# The toolchain packages are the ones the jobs installed before, with apt's default
# recommends, so the compilers, sysroots and emulators are unchanged.

# -- base: what every CI job step needs inside the container -----------------
# git: actions/checkout makes a real clone (CMake embeds the commit hash)
# ccache: found on PATH, so ccache-action does not apt-install it
# zstd: the cache actions compress with it; without it they fall back to gzip and miss
# gh: tools/scripts/ci-drop-ccache-generation.sh calls the GitHub API
FROM ubuntu:24.04 AS base
LABEL org.opencontainers.image.source="https://github.com/stillwater-sc/universal"
LABEL org.opencontainers.image.description="Universal cross-compilation CI toolchains"
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get -o Acquire::Retries=5 update \
 && apt-get -o Acquire::Retries=5 install -y --no-install-recommends \
        ca-certificates ccache cmake gh git make zstd \
 && rm -rf /var/lib/apt/lists/*

# -- RISC-V 64: GCC cross + QEMU user mode -----------------------------------
FROM base AS riscv64
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get -o Acquire::Retries=5 update \
 && apt-get -o Acquire::Retries=5 install -y g++-riscv64-linux-gnu qemu-user-static \
 && rm -rf /var/lib/apt/lists/*
# the sysroot for the dynamic linker and libraries, for QEMU invoked without -L
ENV QEMU_LD_PREFIX=/usr/riscv64-linux-gnu

# -- POWER 64 LE: GCC cross + QEMU user mode ---------------------------------
FROM base AS ppc64le
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get -o Acquire::Retries=5 update \
 && apt-get -o Acquire::Retries=5 install -y g++-powerpc64le-linux-gnu qemu-user-static \
 && rm -rf /var/lib/apt/lists/*
ENV QEMU_LD_PREFIX=/usr/powerpc64le-linux-gnu

# -- Windows x64: MinGW-w64 GCC cross + Wine ---------------------------------
FROM base AS mingw
ARG DEBIAN_FRONTEND=noninteractive
RUN dpkg --add-architecture i386 \
 && apt-get -o Acquire::Retries=5 update \
 && apt-get -o Acquire::Retries=5 install -y g++-mingw-w64-x86-64 wine wine64 \
 && rm -rf /var/lib/apt/lists/*
# No Wine prefix is made here: wineboot copies its DLLs into the prefix (1.3 GB, more than
# a quarter of the image), and making it once per job at the first test costs less than
# pulling it. Wine makes the prefix under HOME, as it did on the runner.
