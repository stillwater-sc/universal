#pragma once
// cfloat_fwd.hpp :  forward declarations of the classic floating-point cfloat environment
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>  // for size_t

namespace sw { namespace universal {

// core cfloat types
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating> class cfloat;
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
		cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>
		abs(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>&);
template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
		cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>
		sqrt(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>&);

template<size_t nbits, size_t es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
		cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>
		fabs(cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>);

#ifdef CFLOAT_QUIRE

// quire types

#endif

}} // namespace sw::universal

