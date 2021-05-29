#pragma once
// ieee754.hpp: RISC-V specific manipulation functions for IEEE-754 native types
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#if defined(__riscv)
/* RISC-V G++ tool chain */

namespace sw::universal {

////////////////////////////////////////////////////////////////////////
// numerical helpers


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// compiler specific long double IEEE floating point

// floating point component extractions
inline void extract_fp_components(float fp, bool& _sign, int& _exponent, float& _fr, uint32_t& _fraction) {
	static_assert(sizeof(float) == 4, "This function only works when float is 32 bits.");
	_sign = fp < 0.0 ? true : false;
	_fr = frexpf(fp, &_exponent);
	_fraction = uint32_t(0x007FFFFFul) & reinterpret_cast<uint32_t&>(_fr);
}
inline void extract_fp_components(double fp, bool& _sign, int& _exponent, double& _fr, uint64_t& _fraction) {
	static_assert(sizeof(double) == 8, "This function only works when double is 64 bits.");
	_sign = fp < 0.0 ? true : false;
	_fr = frexp(fp, &_exponent);
	_fraction = uint64_t(0x000FFFFFFFFFFFFFull) & reinterpret_cast<uint64_t&>(_fr);
}
inline void extract_fp_components(long double fp, bool& _sign, int& _exponent, long double& _fr, uint64_t& _fraction) {
	// RISC-V ABI defines long double as a 128-bit quadprecision floating point
	static_assert(sizeof(double) == 16, "This function only works when long double is 128 bits.");
	_sign = fp < 0.0 ? true : false;
	_fr = frexpl(fp, &_exponent);
	_fraction = uint64_t(0x7FFFFFFFFFFFFFFFull) & reinterpret_cast<uint64_t&>(_fr);
}


} // namespace sw::universal

#endif // RISC-V G++ tool chain

