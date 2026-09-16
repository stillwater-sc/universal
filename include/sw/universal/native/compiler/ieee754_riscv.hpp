#pragma once
// ieee754.hpp: RISC-V specific manipulation functions for IEEE-754 native types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#if defined(__riscv)
/* RISC-V G++ tool chain */

// The IEEE-754 parameters for float, double and long double come from the GNU block, which RISC-V
// gcc also selects: it defines __GNUC__ as well. This file used to specialize them here, and its
// long double carried DOUBLE's field layout -- 11 exponent bits, bias 1023, 52 fraction bits --
// against fields extractFields() hands out in x87 shape, so every long double conversion
// overflowed: cfloat<32,8> = 0.75l gave inf (#1399). What remains is the <cmath> gap below.

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

