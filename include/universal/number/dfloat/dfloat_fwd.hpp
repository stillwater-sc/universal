#pragma once
// dfloat_fwd.hpp :  forward declarations of the decimal floating-point dfloat environment
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// forward references
	template<size_t ndigits, size_t es, typename BlockType> class dfloat;

	template<size_t ndigits, size_t es, typename BlockType, typename NativeFloat> 
	dfloat<ndigits, es, BlockType>& 
		convert(NativeFloat v, dfloat<ndigits, es, BlockType>& result);

	template<size_t ndigits, size_t es, typename BlockType>
	dfloat<ndigits, es, BlockType>&
		convert_unsigned(std::uint64_t v, dfloat<ndigits, es, BlockType>& result);

	template<size_t ndigits, size_t es, typename BlockType>
	bool parse(const std::string& number, dfloat<ndigits, es, BlockType>& v);

	template<size_t ndigits, size_t es, typename BlockType>
	dfloat<ndigits, es, BlockType>
		abs(const dfloat<ndigits, es, BlockType>&);
		
	template<size_t ndigits, size_t es, typename BlockType>
	dfloat<ndigits, es, BlockType>
		sqrt(const dfloat<ndigits, es, BlockType>&);

	template<size_t ndigits, size_t es, typename BlockType>
	dfloat<ndigits, es, BlockType>
		fabs(dfloat<ndigits, es, BlockType>);

#ifdef DFLOAT_QUIRE

// quire types

#endif

}} // namespace sw::universal

