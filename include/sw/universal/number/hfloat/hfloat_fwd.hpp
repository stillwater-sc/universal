#pragma once
// hfloat_fwd.hpp : forward declarations of the IBM System/360 hexadecimal floating-point environment
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>

namespace sw { namespace universal {

	// forward references
	template<unsigned ndigits, unsigned es, typename BlockType> class hfloat;

	template<unsigned ndigits, unsigned es, typename BlockType>
	bool parse(const std::string& number, hfloat<ndigits, es, BlockType>& v);

	template<unsigned ndigits, unsigned es, typename BlockType>
	hfloat<ndigits, es, BlockType>
		abs(const hfloat<ndigits, es, BlockType>&);

	template<unsigned ndigits, unsigned es, typename BlockType>
	hfloat<ndigits, es, BlockType>
		sqrt(const hfloat<ndigits, es, BlockType>&);

	template<unsigned ndigits, unsigned es, typename BlockType>
	hfloat<ndigits, es, BlockType>
		fabs(hfloat<ndigits, es, BlockType>);

}} // namespace sw::universal
