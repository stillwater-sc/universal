#pragma once
// cfloat_fwd.hpp :  forward declarations of the classic floating-point cfloat environment
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// core cfloat types
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating> class cfloat;
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
		cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>
		abs(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>&);
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
		cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>
		sqrt(const cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>&);

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
		cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>
		fabs(cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>);

#ifdef CFLOAT_QUIRE

// quire types

#endif

}} // namespace sw::universal

