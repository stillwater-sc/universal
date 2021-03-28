#pragma once
// math_classify.hpp: templated classification function stubs for native floating-point
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw::universal {

	// STD LIB function for IEEE floats: Categorizes floating point value arg into the following categories: zero, subnormal, normal, infinite, NAN, or implementation-defined category.
	template<typename Scalar,
			 typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
	int fpclassify(const Scalar& v) {
		return std::fpclassify(v);
	}

	/*
	// STD LIB function for IEEE floats: Determines if the given floating point number arg has finite value i.e. it is normal, subnormal or zero, but not infinite or NaN.
	template<typename Scalar,
			 typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
	inline bool isfinite(const Scalar& v) {
		return !std::isinf(v) && !std::isnan(v);
	}

	// STD LIB function for IEEE floats: Determines if the given floating point number arg is a positive or negative infinity.
	template<typename Scalar,
			 typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
	inline bool isinf(const Scalar& v) {
		return std::isinf(v);
	}

	// STD LIB function for IEEE floats: Determines if the given floating point number arg is a not-a-number (NaN) value.
	template<typename Scalar,
			 typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
	inline bool isnan(const Scalar& v) {
		return std::isnan(v);
	}

	// STD LIB function for IEEE floats: Determines if the given floating point number arg is normal, i.e. is neither zero, subnormal, infinite, nor NaN.
	template<typename Scalar,
			 typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
	inline bool isnormal(const Scalar& v) {
		return std::isnormal(v);
	}
	*/

	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		inline bool isdenorm(const Scalar& v) {
		return !std::isnormal(v) && !std::isnan(v) && !std::isinf(v);
	}

}  // namespace sw::universal
