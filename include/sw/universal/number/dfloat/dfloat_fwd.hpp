#pragma once
// dfloat_fwd.hpp :  forward declarations of the decimal floating-point dfloat environment
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>

namespace sw { namespace universal {

	// forward references
	template<unsigned ndigits, unsigned es, typename BlockType> class dfloat;

	template<unsigned ndigits, unsigned es, typename BlockType>
	bool parse(const std::string& number, dfloat<ndigits, es, BlockType>& v);

	template<unsigned ndigits, unsigned es, typename BlockType>
	dfloat<ndigits, es, BlockType>
		abs(const dfloat<ndigits, es, BlockType>&);
		
	template<unsigned ndigits, unsigned es, typename BlockType>
	dfloat<ndigits, es, BlockType>
		sqrt(const dfloat<ndigits, es, BlockType>&);

	template<unsigned ndigits, unsigned es, typename BlockType>
	dfloat<ndigits, es, BlockType>
		fabs(dfloat<ndigits, es, BlockType>);

#ifdef DFLOAT_QUIRE

// quire types

#endif

}} // namespace sw::universal

