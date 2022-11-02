#pragma once
// dfloat_fwd.hpp :  forward declarations of the decimal floating-point dfloat environment
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

// core dfloat types
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating> class dfloat;
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
		dfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>
		abs(const dfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>&);
template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
		dfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>
		sqrt(const dfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>&);

template<unsigned nbits, unsigned es, typename bt, bool hasSubnormals, bool hasSupernormals, bool isSaturating>
		dfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>
		fabs(dfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>);

#ifdef DFLOAT_QUIRE

// quire types

#endif

}} // namespace sw::universal

