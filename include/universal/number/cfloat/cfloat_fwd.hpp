#pragma once
// cfloat_fwd.hpp :  forward declarations of the classic floating-point cfloat environment
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>  // for size_t

namespace sw::universal {

	namespace internal {
		// generalized floating point type
		template<size_t fbits> class value;
	}

// core cfloat types
template<size_t nbits, size_t es, typename BlockType> class cfloat;
template<size_t nbits, size_t es, typename BlockType> cfloat<nbits, es> abs(const posit<nbits, es>& p);
template<size_t nbits, size_t es, typename BlockType> cfloat<nbits, es> sqrt(const posit<nbits, es>& p);

#ifdef LATER
// core constexpr values of the number system
template<size_t nbits, size_t es> constexpr cfloat<nbits, es>& minpos(posit<nbits, es>& p);
template<size_t nbits, size_t es> constexpr cfloat<nbits, es>& maxpos(posit<nbits, es>& p);
template<size_t nbits, size_t es> constexpr cfloat<nbits, es>  minpos();
template<size_t nbits, size_t es> constexpr cfloat<nbits, es>  maxpos();

// quire types
template<size_t nbits, size_t es, size_t capacity> class quire;
template<size_t nbits, size_t es, size_t capacity> cfloat<nbits, es, bt>& quire_mul(const cfloat<nbits, es>&, const cfloat<nbits, es>&);
#endif

} // namespace sw::universal

