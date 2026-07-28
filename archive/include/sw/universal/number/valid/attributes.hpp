#pragma once
// attributes.hpp: functions to query attributes of valid types
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw { namespace universal {

	// functions to provide details about
	// the properties of a valid configuration

	// TODO : how do you specialize these functions between posits and valids?
	
	// forward reference
	template<unsigned nbits, unsigned es> class valid;

	template<unsigned nbits, unsigned es>
	inline int sign_value(const valid<nbits, es>& v) {
		int s = 1;
		std::cerr << "sign_value(valid) not implemented yet";
		return s;
	}

	template<unsigned nbits, unsigned es>
	inline long double regime_value(const valid<nbits, es>& v) {
		std::cerr << "regime_value(valid) not implemented yet";
		return 0.0l;
	}

	template<unsigned nbits, unsigned es>
	inline long double exponent_value(const valid<nbits, es>& v) {
		std::cerr << "exponent_value(valid) not implemented yet";
		return 0.0l;
	}

	template<unsigned nbits, unsigned es>
	inline long double fraction_value(const valid<nbits, es>& p) {
		std::cerr << "fraction_value(valid) not implemented yet";
		return 0.0l;
	}

	// get the sign of the valid
	template<unsigned nbits, unsigned es>
	inline bool sign(const valid<nbits, es>& v) {
		return v.isneg();
	}

	// calculate the scale of a valid
	template<unsigned nbits, unsigned es>
	inline int scale(const valid<nbits, es>& v) {
		std::cerr << "scale(valid) not implemented yet";
		return 0; // return the scale
	}

	// calculate the significant of a valid
	template<unsigned nbits, unsigned es, unsigned fbits>
	inline bitblock<fbits+1> significant(const valid<nbits, es>& v) {
		std::cerr << "significant(valid) not implemented yet";
		return bitblock<fbits+1>();
	}

	// get the fraction bits of a valid
	template<unsigned nbits, unsigned es, unsigned fbits>
	inline bitblock<fbits> extract_fraction(const valid<nbits, es>& v) {
		//constexpr unsigned fbits = nbits - 3 - es;
		std::cerr << "extract_fraction(valid) not implemented yet";
		return bitblock<fbits>();
	}

	// calculate the scale of the regime component of the valid
	template<unsigned nbits, unsigned es>
	inline int regime_scale(const valid<nbits, es>& v) {
		std::cerr << "regime_scale(valid) not implemented yet";
		return 0;
	}

	// calculate the scale of the exponent component of the valid
	template<unsigned nbits, unsigned es>
	inline int exponent_scale(const valid<nbits, es>& v) {
		std::cerr << "exponent_scale(valid) not implemented yet";
		return 0;
	}


}} // namespace sw::universal
