#pragma once
// type_tag.hpp: definitions of type_tag functions for SORN numbers 
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <sstream>

#include <universal/native/ieee754_type_tag.hpp>

namespace sw { namespace universal {

	// Generate a type tag for this sorn
	template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero>
	inline std::string type_tag(const sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& = {}) {
		std::stringstream s;
		float f(0.0f);
		s << "sorn<"
		  << std::setw(10) << type_tag<float>(f) // << ", "
		  << '>';
		return s.str();
	}

}} // namespace sw::universal
