#pragma once
// posito_fwd.hpp :  forward declarations of the posit/quire environment
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

	// posito types
	template<unsigned nbits, unsigned es> class posito;
	template<unsigned nbits, unsigned es> inline int scale(const posito<nbits, es>&);
	template<unsigned nbits, unsigned es, unsigned fbits> inline internal::bitblock<fbits + 1> extract_significant(const posito<nbits, es>&);
	template<unsigned nbits, unsigned es> posito<nbits, es> abs(const posito<nbits, es>&);
	template<unsigned nbits, unsigned es> posito<nbits, es> sqrt(const posito<nbits, es>&);
	template<unsigned nbits, unsigned es, unsigned fbits> posito<nbits, es>& convert(const internal::value<fbits>&, posito<nbits, es>&);

	template<unsigned nbits> int decode_regime(const internal::bitblock<nbits>&);
	template<unsigned nbits, unsigned es> constexpr int calculate_k(int scale);

}} // namespace sw::universal

