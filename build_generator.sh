#!/bin/sh

####
# Set build options
# MSVC generates SSE/SSE2 code by default. No support for SSE3 as of 7/15/2018

# HOW TO USE
# after cloning the repo 
# $ git clone https://github.com/stillwater-sc/universal
# $ cd universal
# edit the program components below to enable specific components by changing OFF to ON
# create a build directory
# $ mkdir build
# go into the build directory
# $ cd build
# and execute this script
# $ sh ../build_generator.sh
# build the software
# $ make -j 16
# test the software built
# $ ctest -j 16

# BUILD_CI_CHECK is special: when set to ON, it will enable all the components in the Universal library
cmake \
-DUSE_SSE3=OFF \
-DUSE_AVX=OFF \
-DUSE_AVX2=OFF \
\
-DBUILD_EDUCATION_EXAMPLES=OFF \
-DBUILD_APPLICATION_EXAMPLES=OFF \
-DBUILD_CMD_LINE_TOOLS=OFF \
-DBUILD_PLAYGROUND=OFF \
 \
-DBUILD_BITBLOCK=OFF \
-DBUILD_REAL_NUMBER=OFF \
-DBUILD_INTEGER_NUMBER=OFF \
-DBUILD_UNUM_TYPE_1=OFF \
-DBUILD_UNUM_TYPE_2=OFF \
-DBUILD_UNUM_TYPE_3_POSIT=OFF \
-DBUILD_UNUM_TYPE_3_VALID=OFF \
-DBUILD_APF=OFF \
 \
-DBUILD_PERF_TESTS=OFF \
-DBUILD_C_API_LIB=OFF \
 \
-DBUILD_IEEE_FLOAT_QUIRES=OFF \
-DBUILD_CI_CHECK=OFF \
-DBUILD_DOCS=OFF \
..
