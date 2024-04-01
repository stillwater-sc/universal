#pragma once
// posit_fwd.hpp :  forward declarations of the posit/quire environment
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>  // for unsigned
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>

namespace sw { namespace universal {

	namespace internal {
		// generalized floating point type
		template<unsigned fbits> class value;
	}

// posit types
template<unsigned nbits, unsigned es, typename bt> class posit;

// posit-specialized math functions
template<unsigned nbits, unsigned es, typename bt> posit<nbits, es, bt> abs(const posit<nbits, es, bt>&);
template<unsigned nbits, unsigned es, typename bt> posit<nbits, es, bt> sqrt(const posit<nbits, es, bt>&);

// helpers
template<unsigned nbits, unsigned es, typename bt, unsigned fbits, BlockTripleOperator op> posit<nbits, es, bt>& convert(const blocktriple<fbits, op, bt>&, posit<nbits, es, bt>&);

template<unsigned nbits, typename bt> int decode_regime(const blockbinary<nbits, bt>&);

// quire types
//template<unsigned nbits, unsigned es, unsigned capacity> class quire;
//template<unsigned nbits, unsigned es, unsigned capacity> internal::value<2 * (nbits - 2 - es)> quire_mul(const posit<nbits, es>&, const posit<nbits, es>&);

}} // namespace sw::universal

