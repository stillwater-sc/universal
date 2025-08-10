#pragma once
// posit_fwd.hpp :  forward declarations of the posit/quire environment
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	namespace internal {
		template<unsigned fbits> class bitblock;
		// generalized floating point type
		template<unsigned fbits> class value;
	}

	// posit types
	template<unsigned nbits, unsigned es> class posit;
	template<unsigned nbits, unsigned es> inline int scale(const posit<nbits, es>&);
	template<unsigned nbits, unsigned es, unsigned fbits> inline internal::bitblock<fbits + 1> extract_significant(const posit<nbits, es>&);
	template<unsigned nbits, unsigned es> posit<nbits, es> abs(const posit<nbits, es>&);
	template<unsigned nbits, unsigned es> posit<nbits, es> sqrt(const posit<nbits, es>&);
	template<unsigned nbits, unsigned es, unsigned fbits> posit<nbits, es>& convert(const internal::value<fbits>&, posit<nbits, es>&);

	template<unsigned nbits> int decode_regime(const internal::bitblock<nbits>&);
	template<unsigned nbits, unsigned es> constexpr int calculate_k(int scale);

	// quire types
	template<unsigned nbits, unsigned es, unsigned capacity> class quire;
	template<unsigned nbits, unsigned es, unsigned capacity> internal::value<2 * (nbits - 2 - es)> quire_mul(const posit<nbits, es>&, const posit<nbits, es>&);

}} // namespace sw::universal

