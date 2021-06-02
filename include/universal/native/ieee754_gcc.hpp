#pragma once
// ieee754.hpp: GNU gcc/g++ specific manipulation functions for IEEE-754 native types
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#if (defined(__GNUC__) || defined(__GNUG__)) && !defined(__clang__)
/* GNU GCC/G++. --------------------------------------------- */

namespace sw::universal {

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
};
// GNU long double = 80 bit extended precision
// 
// IEEE-754 parameter constexpressions for long double
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
	static constexpr uint64_t hmask    = 0x8000'0000'0000'0001ull; // mask for the bottom half
	static constexpr uint64_t fmask    = 0x7FFF'FFFF'FFFF'FFFFull; // mask for the bottom half
	static constexpr uint64_t hfmask   = 0xFFFF'FFFF'FFFF'FFFFull; // mast for the bottom half
	static constexpr uint64_t fmsb     = 0x8000'0000'0000'0000ull;
	static constexpr uint64_t qnanmask = 0x7FF8'0000'0000'0000ull;
	static constexpr uint64_t snanmask = 0x7FF4'0000'0000'0000ull;
};

} // namespace sw::universal

#endif // GNU GCC/G++
