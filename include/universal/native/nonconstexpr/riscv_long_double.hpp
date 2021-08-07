#pragma once
// riscv_long_double.hpp: nonconstexpr implementation of IEEE-754 long double manipulators
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#if defined(__riscv)
/* RISC-V G++ tool chain */

namespace sw::universal {

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// compiler specific long double IEEE floating point

inline void extract_fp_components(long double fp, bool& _sign, int& _exponent, long double& _fr, uint64_t& _fraction) {
	// RISC-V ABI defines long double as a 128-bit quadprecision floating point
	static_assert(sizeof(double) == 16, "This function only works when long double is 128 bits.");
	_sign = fp < 0.0 ? true : false;
	_fr = frexpl(fp, &_exponent);
	_fraction = uint64_t(0x7FFFFFFFFFFFFFFFull) & reinterpret_cast<uint64_t&>(_fr);
}


} // namespace sw::universal

#endif // RISC-V G++ tool chain

