#pragma once
// dfixpnt_fwd.hpp: forward declarations for the decimal fixed-point dfixpnt type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <universal/number/shared/decimal_encoding.hpp>

namespace sw { namespace universal {

	// forward references
	template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt> class dfixpnt;

	template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
	dfixpnt<ndigits, radix, encoding, arithmetic, bt>
		abs(const dfixpnt<ndigits, radix, encoding, arithmetic, bt>&);

	template<unsigned ndigits, unsigned radix, DecimalEncoding encoding, bool arithmetic, typename bt>
	bool parse(const std::string& number, dfixpnt<ndigits, radix, encoding, arithmetic, bt>& v);

}} // namespace sw::universal
