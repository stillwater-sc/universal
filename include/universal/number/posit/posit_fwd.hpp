#pragma once
// posit_fwd.hpp :  forward declarations of the posit/quire environment
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>  // for size_t

namespace sw { namespace universal {

// generalized floating point type
template<size_t fbits> class value;

// posit types
template<size_t nbits, size_t es> class posit;
template<size_t nbits, size_t es> posit<nbits, es> abs(const posit<nbits, es>& p);
template<size_t nbits, size_t es> posit<nbits, es> sqrt(const posit<nbits, es>& p);
template<size_t nbits, size_t es> constexpr posit<nbits, es>& minpos(posit<nbits, es>& p);
template<size_t nbits, size_t es> constexpr posit<nbits, es>& maxpos(posit<nbits, es>& p);
template<size_t nbits, size_t es> constexpr posit<nbits, es>  minpos();
template<size_t nbits, size_t es> constexpr posit<nbits, es>  maxpos();
template<size_t nbits, size_t es, size_t fbits> posit<nbits, es>& convert(const value<fbits>&, posit<nbits, es>&);

// quire types
template<size_t nbits, size_t es, size_t capacity> class quire;
template<size_t nbits, size_t es, size_t capacity> value<2 * (nbits - 2 - es)> quire_mul(const posit<nbits, es>&, const posit<nbits, es>&);

}} // namespace sw::universal

