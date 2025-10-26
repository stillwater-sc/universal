#pragma once
// classify.hpp: classification functions for integer
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// STD LIB function for IEEE floats: Categorizes the integer value:
	// integers are always finite and normal (except zero)

	// isnan: integers can never be NaN
	template<unsigned nbits, typename bt, IntegerNumberType nt>
	inline bool isnan(const integer<nbits, bt, nt>&) {
		return false;
	}

	// isinf: integers can never be infinite
	template<unsigned nbits, typename bt, IntegerNumberType nt>
	inline bool isinf(const integer<nbits, bt, nt>&) {
		return false;
	}

	// check if the integer is zero
	template<unsigned nbits, typename bt, IntegerNumberType nt>
	inline bool iszero(const integer<nbits, bt, nt>& i) {
		return (i == 0);
	}

	// isnormal: integers are normal unless they are zero
	template<unsigned nbits, typename bt, IntegerNumberType nt>
	inline bool isnormal(const integer<nbits, bt, nt>& v) {
		return v != integer<nbits, bt, nt>(0);
	}

	// isfinite: integers are always finite
	template<unsigned nbits, typename bt, IntegerNumberType nt>
	inline bool isfinite(const integer<nbits, bt, nt>&) {
		return true;
	}

}} // namespace sw::universal