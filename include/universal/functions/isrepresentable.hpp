#pragma once
// isrepresentable.hpp: test to see if a ratio of Integer type can be represented by a binary real
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
namespace unum {

// isRepresentable tests if the ratio a/b can be represented exactly by a binary Real
template<typename IntegerType>
bool isRepresentable(IntegerType a, IntegerType b) {
	if (b == IntegerType(0)) return false;
	while (b % 2 == IntegerType(0)) { b /= IntegerType(2); }
	while (b % 5 == IntegerType(0)) { b /= IntegerType(5); }
	return a % b == IntegerType(0);
}

} // namespace unum
} // namespace sw
