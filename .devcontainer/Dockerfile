FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
        clang-format \
        clang-tidy \
 && apt-get clean && rm -rf /var/lib/apt/lists/*
