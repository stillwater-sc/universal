#pragma once
// attributes.hpp: functions to query number system attributes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <string>
#include <sstream>

namespace sw { namespace universal {

// functions to provide details about properties of a microfloat configuration

	template<unsigned nbits, unsigned es, bool hasInf, bool hasNaN, bool isSaturating>
	inline bool sign(microfloat<nbits, es, hasInf, hasNaN, isSaturating> mf) {
		return mf.sign();
	}

	template<unsigned nbits, unsigned es, bool hasInf, bool hasNaN, bool isSaturating>
	inline int scale(microfloat<nbits, es, hasInf, hasNaN, isSaturating> mf) {
		return mf.scale();
	}

	// generate the maxneg through maxpos value range of a microfloat configuration
	template<unsigned nbits, unsigned es, bool hasInf, bool hasNaN, bool isSaturating>
	inline std::string microfloat_range() {
		using MF = microfloat<nbits, es, hasInf, hasNaN, isSaturating>;
		MF v{};
		std::stringstream s;
		s << std::setw(40) << type_tag(v) << " : [ "
			<< v.maxneg() << " ... "
			<< v.minneg() << " "
			<< "0 "
			<< v.minpos() << " ... "
			<< v.maxpos() << " ]";
		return s.str();
	}

}}  // namespace sw::universal
