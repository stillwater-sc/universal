#!/usr/bin/env bash

# script to be executed inside -build- directory
# By default, the following environments are automatically set
# BUILD_EXAMPLES_APPLICATIONS
# BUILD_EXAMPLES_EDUCATION
# BUILD_CMD_LINE_TOOLS
# BUILD_PLAYGROUND

# so to build just the applications, we turn of the other defaults
cmake -DBUILD_EXAMPLES_EDUCATION=OFF -DBUILD_CMD_LINE_TOOLS=OFF -DBUILD_PLAYGROUND=OFF ..
