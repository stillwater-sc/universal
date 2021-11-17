#pragma once
// math_classify.hpp: classification functions for rationals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

// STD LIB function for IEEE floats: Categorizes floating point value arg into the following categories: zero, subnormal, normal, infinite, NAN, or implementation-defined category.
int fpclassify(const rational& a) {
	return std::fpclassify((long double)(a));
}
	
// STD LIB function for IEEE floats: Determines if the given floating point number arg has finite value i.e. it is normal, subnormal or zero, but not infinite or NaN.
// specialized for rationals
inline bool isfinite(const rational& a) {
	return true;
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is a fixpntive or negative infinity.
// specialized for rationals
inline bool isinf(const rational& a) {
	return false;
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is a not-a-number (NaN) value.
// specialized for rationals
inline bool isnan(const rational& a) {
	return false;
}

// STD LIB function for IEEE floats: Determines if the given floating point number arg is normal, i.e. is neither zero, subnormal, infinite, nor NaN.
// specialized for rationals
inline bool isnormal(const rational& a) {
	return true;
}

}  // namespace sw::universal
