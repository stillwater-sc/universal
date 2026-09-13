#pragma once 
// attributes.hpp: functions to query number system attributes 
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Part of the text layer (#1334): sign(), scale() and significant() are pure
// bit inspection, but bfloat_range() streams bfloat16 values through a
// stringstream, so this header sits above iostream.hpp and is not included by
// core.hpp. Self-contained: <iomanip> for std::setw was used but never named.
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>   // std::setw
#include <universal/number/bfloat16/core.hpp>
#include <universal/number/bfloat16/manipulators.hpp>   // type_tag
#include <universal/number/bfloat16/iostream.hpp>       // operator<< on bfloat16, used by bfloat_range

namespace sw { namespace universal {  

// functions to provide details about properties of a bfloat configuration

	inline bool sign(bfloat16 bf) {
		return bf.sign();
	}

	inline int scale(bfloat16 bf) {
		return bf.scale();
	}
	
	inline uint32_t significant(bfloat16 bf) {
		return ((bf.bits() & 0x007Fu) | 0x0080u);
	}

	// generate the maxneg through maxpos value range of a bfloat16 configuration
	inline std::string bfloat_range() {
		bfloat16 v{ 0 };
		std::stringstream s;
		s << std::setw(80) << type_tag(v) << " : [ "
			<< v.maxneg() << " ... "
			<< v.minneg() << " "
			<< "0 "
			<< v.minpos() << " ... "
			<< v.maxpos() << " ]";
		return s.str();
	}

}}  // namespace sw::universal
