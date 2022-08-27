#pragma once
// sorn_fwd.hpp :  forward declarations for the SORN number system environment
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstddef>  // for size_t
#include <string>

namespace sw { namespace universal {

// core SORN types and functions
	template<typename Real> 
		struct sornInterval;
	template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero>  
		class sorn;

	template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero> 
		sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>
		abs(sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>);
	template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero> 
		sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>
		sqrt(const sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>&);

	// transform sorn to a binary representation
	template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero>
	inline std::string to_binary(const sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>& number, bool nibbleMarker = false);

}} // namespace sw::universal

