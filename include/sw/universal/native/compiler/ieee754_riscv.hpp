#pragma once
// ieee754.hpp: RISC-V specific manipulation functions for IEEE-754 native types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#if defined(__riscv)
/* RISC-V G++ tool chain */

namespace sw { namespace universal {

// specializations for IEEE-754 parameters RISC-V GCC/G++
template<>
class ieee754_parameter<float> {
public:
	static constexpr int      nbits    = 32;
	static constexpr uint64_t smask    = 0x8000'0000ull;
	static constexpr int      ebits    = 8;
	static constexpr int      bias     = 127;
	static constexpr uint64_t emask    = 0x7F80'0000ull;
	static constexpr uint64_t eallset  = 0xFFull;
	static constexpr int      fbits    = 23;
	static constexpr uint64_t hmask    = 0x0080'0000ull;
	static constexpr uint64_t fmask    = 0x007F'FFFFull;
	static constexpr uint64_t hfmask   = 0x00FF'FFFFull;
	static constexpr uint64_t fmsb     = 0x0040'0000ull;
	static constexpr uint64_t qnanmask = 0x7FC0'0000ull;
	static constexpr uint64_t snanmask = 0x7FC0'0001ull;
	static constexpr float    minNormal       = 1.1754943508222875079687365372222e-38f; // == 2^-126
 	static constexpr float    minSubnormal    = 1.4012984643248170709237295832899e-45f; // == 2^-149
	static constexpr int      minNormalExp    = -126;
	static constexpr int      minSubnormalExp = -149;
};
template<>
class ieee754_parameter<double> {
public:
	static constexpr int      nbits    = 64;
	static constexpr uint64_t smask    = 0x8000'0000'0000'0000ull;
	static constexpr int      ebits    = 11;
	static constexpr int      bias     = 1023;
	static constexpr uint64_t emask    = 0x7FF0'0000'0000'0000ull;
	static constexpr uint64_t eallset  = 0x7FF;
	static constexpr int      fbits    = 52;
	static constexpr uint64_t hmask    = 0x0010'0000'0000'0000ull;
	static constexpr uint64_t fmask    = 0x000F'FFFF'FFFF'FFFFull;
	static constexpr uint64_t hfmask   = 0x001F'FFFF'FFFF'FFFFull;
	static constexpr uint64_t fmsb     = 0x0008'0000'0000'0000ull;
	static constexpr uint64_t qnanmask = 0x7FF8'0000'0000'0000ull;
	static constexpr uint64_t snanmask = 0x7FF0'0000'0000'0001ull;
	static constexpr double   minNormal    = 2.2250738585072013830902327173324e-308; // == 2^-1022
	static constexpr double   minSubnormal = 4.9406564584124654417656879286220e-324; // == 2^-1074
	static constexpr int      minNormalExp = -1022;
	static constexpr int      minSubnormalExp = -1074;
};

// IEEE-754 parameter constexpressions for long double
template<>
class ieee754_parameter<long double> {
public:
	static constexpr int      nbits    = 64;
	static constexpr uint64_t smask    = 0x8000'0000'0000'0000ull;
	static constexpr int      ebits    = 11;
	static constexpr int      bias     = 1023;
	static constexpr uint64_t emask    = 0x7FF0'0000'0000'0000ull;
	static constexpr uint64_t eallset  = 0x7FF;
	static constexpr int      fbits    = 52;
	static constexpr uint64_t hmask    = 0x0010'0000'0000'0000ull;
	static constexpr uint64_t fmask    = 0x000F'FFFF'FFFF'FFFFull;
	static constexpr uint64_t hfmask   = 0x001F'FFFF'FFFF'FFFFull;
	static constexpr uint64_t fmsb     = 0x0008'0000'0000'0000ull;
	static constexpr uint64_t qnanmask = 0x7FF8'0000'0000'0000ull;
	static constexpr uint64_t snanmask = 0x7FF0'0000'0000'0001ull;
	static constexpr long double minNormal       = 3.3621031431120935062626778173218e-4932l; // == 2^-16382
	static constexpr long double minSubnormal    = 3.6451995318824746025284059336194e-4951l; // == 2^-16445
	static constexpr int         minNormalExp    = -16382;
	static constexpr int         minSubnormalExp = -16445;
};

}} // namespace sw::universal

// RISC-V has a greatly reduced <cmath> so we are going to stub
// the missing functions out so software compiles, but will
// provide user feedback of the missing implementation
namespace std {

	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar nextafter(Scalar x, Scalar target) {
		return nextafter(double(x), double(target));
	}

	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar trunc(Scalar x) {
		return trunc(double(x));  // call the math.h function
	}

	template<typename Scalar,
		typename = typename std::enable_if<std::is_floating_point<Scalar>::value>::type>
		Scalar round(Scalar x) {
		return round(double(x));   // call the math.h function
	}

}

#endif // RISC-V G++ tool chain

