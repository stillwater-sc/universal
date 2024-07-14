#pragma once
// fixpnt_fwd.hpp: forward declarations for fixpnt type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// forward references
	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt> class fixpnt;
	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt> fixpnt<nbits, rbits, arithmetic, bt> abs(const fixpnt<nbits, rbits, arithmetic, bt>&);

	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt> struct fixpntdiv_t;
	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt> fixpntdiv_t<nbits, rbits, arithmetic, bt> fixpntdiv(const fixpnt<nbits, rbits, arithmetic, bt>&, const fixpnt<nbits, rbits, arithmetic, bt>&);

	// fixpntdiv_t for fixpnt<nbits,rbits> to capture quotient and remainder during long division
	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt> struct fixpntdiv_t;

	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
	bool parse(const std::string& number, fixpnt<nbits, rbits, arithmetic, bt>& v);

	// free function generator to create a 1's complement copy of a fixpnt
	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
	inline fixpnt<nbits, rbits, arithmetic, bt> onesComplement(const fixpnt<nbits, rbits, arithmetic, bt>& value);
	// free function generator to create the 2's complement of a fixpnt
	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
	inline fixpnt<nbits, rbits, arithmetic, bt> twosComplement(const fixpnt<nbits, rbits, arithmetic, bt>& value);

	// The free function scale calculates the power of 2 exponent that would capture an approximation of a normalized real value
	template<unsigned nbits, unsigned rbits, bool arithmetic, typename bt>
	inline int scale(const fixpnt<nbits, rbits, arithmetic, bt>& i);

}} // namespace sw::universal
