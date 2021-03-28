#pragma once
// math_classify.hpp: templated classification function stubs for native floating-point
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>

namespace sw::universal {

#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */


#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */


#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */

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

#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/aC++. ---------------------------------- */

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */


#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */

#endif

	// STD LIB function for IEEE floats: Categorizes floating point value arg into the following categories: zero, subnormal, normal, infinite, NAN, or implementation-defined category.
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
	int fpclassify(const Scalar& v) {
		return std::fpclassify(v);
	}

	// Universal function supported by all number systems
	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		inline bool isdenorm(const Scalar& v) {
		return !std::isnormal(v) && !std::isnan(v) && !std::isinf(v);
	}

	// define an alias for isdenorm
	template<typename Scalar>
	inline bool issubnorm(const Scalar& v) { return isdenorm(v); }

}  // namespace sw::universal
