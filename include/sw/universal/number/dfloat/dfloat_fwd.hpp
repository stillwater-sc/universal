#pragma once
// dfloat_fwd.hpp :  forward declarations of the decimal floating-point dfloat environment
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>

namespace sw { namespace universal {

	// IEEE 754-2008 decimal floating-point encoding formats
	enum class DecimalEncoding {
		BID,   // Binary Integer Decimal: significand stored as binary integer
		DPD    // Densely Packed Decimal: significand stored as 10-bit declets
	};

	// forward references
	template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType> class dfloat;

	template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
	bool parse(const std::string& number, dfloat<ndigits, es, Encoding, BlockType>& v);

	template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
	dfloat<ndigits, es, Encoding, BlockType>
		abs(const dfloat<ndigits, es, Encoding, BlockType>&);

	template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
	dfloat<ndigits, es, Encoding, BlockType>
		sqrt(const dfloat<ndigits, es, Encoding, BlockType>&);

	template<unsigned ndigits, unsigned es, DecimalEncoding Encoding, typename BlockType>
	dfloat<ndigits, es, Encoding, BlockType>
		fabs(dfloat<ndigits, es, Encoding, BlockType>);

}} // namespace sw::universal
