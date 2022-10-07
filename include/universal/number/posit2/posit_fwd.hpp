#pragma once
// posit_fwd.hpp :  forward declarations of the posit/quire environment
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>  // for size_t

namespace sw { namespace universal {

	namespace internal {
		// generalized floating point type
		template<size_t fbits> class value;
	}

// posit types
template<size_t nbits, size_t es, typename bt> class posit;
template<size_t nbits, size_t es, typename bt> posit<nbits, es, bt> abs(const posit<nbits, es, bt>& p);
template<size_t nbits, size_t es, typename bt> posit<nbits, es, bt> sqrt(const posit<nbits, es, bt>& p);
template<size_t nbits, size_t es, typename bt, size_t fbits> posit<nbits, es, bt>& convert(const internal::value<fbits>&, posit<nbits, es, bt>&);

// quire types
//template<size_t nbits, size_t es, size_t capacity> class quire;
//template<size_t nbits, size_t es, size_t capacity> internal::value<2 * (nbits - 2 - es)> quire_mul(const posit<nbits, es>&, const posit<nbits, es>&);

}} // namespace sw::universal

