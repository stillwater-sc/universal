#pragma once
// ieee754.hpp: GNU gcc/g++ specific manipulation functions for IEEE-754 native types
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#if (defined(__GNUC__) || defined(__GNUG__)) && !defined(__clang__)
/* GNU x86 GCC/G++. --------------------------------------------- */

#include <limits>

namespace sw { namespace universal {

// specializations of IEEE-754 parameters for GCC

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
	static constexpr uint64_t snanmask = 0x7FA0'0000ull;
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
	static constexpr uint64_t snanmask = 0x7FF4'0000'0000'0000ull;
	static constexpr double   minNormal       = 2.2250738585072013830902327173324e-308; // == 2^-1022
	static constexpr double   minSubnormal    = 4.9406564584124654417656879286220e-324; // == 2^-1074
	static constexpr int      minNormalExp    = -1022;
	static constexpr int      minSubnormalExp = -1074;
};

// GNU long double = 80 bit extended precision
//
// IEEE-754 parameter constexpressions for long double. This is also the shape of the fields
// extractFields() hands out for a long double that is not x87 -- IEEE binary128 on aarch64,
// IBM double-double on POWER -- computed from the value rather than read from its bits (#1515).
// qnanmask and snanmask are x87's canonical quiet and signalling NaN in the 63-bit fraction
// field: the quiet bit is bit 62. They used to be double's patterns, whose bits 51-62 made
// every x87 NaN with a payload classify as quiet, a signalling one included.
template<>
class ieee754_parameter<long double> {
public:
	static constexpr int      nbits    = 80;
	static constexpr uint64_t smask    = 0x0000'0000'0000'8000ull; // mask for the top half
	static constexpr int      ebits    = 15;
	static constexpr int      bias     = 16383;
	static constexpr uint64_t emask    = 0x0000'0000'0000'7FFFull; // mask for the top half
	static constexpr uint64_t eallset  = 0x7FFF;
	static constexpr int      fbits    = 63;
	// the explicit integer bit, in the bottom half. It was 0x8000'0000'0000'0001: a stray 2^-63 in
	// every significand built as f | hmask, so rational<32> = 100.0l gave 100.000006 (#1515)
	static constexpr uint64_t hmask    = 0x8000'0000'0000'0000ull;
	static constexpr uint64_t fmask    = 0x7FFF'FFFF'FFFF'FFFFull; // mask for the bottom half
	static constexpr uint64_t hfmask   = 0xFFFF'FFFF'FFFF'FFFFull; // mast for the bottom half
	static constexpr uint64_t fmsb     = 0x8000'0000'0000'0000ull;
	static constexpr uint64_t qnanmask = 0x4000'0000'0000'0000ull; // the quiet bit, in the fraction field
	static constexpr uint64_t snanmask = 0x2000'0000'0000'0000ull; // canonical signalling payload, quiet bit clear
	// the format's own range, which differs from x87's: binary128 reaches 2^-16494, and IBM
	// double-double's normal range ends at 2^-969, where x87's 2^-16382 is 0 (#1515)
	static constexpr long double minNormal       = std::numeric_limits<long double>::min();
	static constexpr long double minSubnormal    = std::numeric_limits<long double>::denorm_min();
	static constexpr int         minNormalExp    = std::numeric_limits<long double>::min_exponent - 1;
	static constexpr int         minSubnormalExp = std::numeric_limits<long double>::min_exponent - std::numeric_limits<long double>::digits;
};

}} // namespace sw::universal

#endif // GNU GCC/G++
