#!/bin/sh

export CC=/usr/bin/clang
export CXX=/usr/bin/clang++
rm -rf build_clang
mkdir build_clang
cd ./build_clang
#cmake -DCMAKE_USER_MAKE_RULES_OVERRIDE=../ClangOverrides.txt -DBUILD_CI_CHECK=ON -DUSE_AVX2=ON ..
cmake -DBUILD_CI_CHECK=ON -DUSE_AVX2=ON ..
cd ..
