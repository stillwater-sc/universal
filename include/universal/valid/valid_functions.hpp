#pragma once
// valid_functions.hpp: simple math functions on valid types
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
namespace unum {

	// functions to provide details about
	// the properties of a valid configuration

	// TODO : how do you specialize these functions between posits and valids?
	
	/*
	// calculate exponential scale of useed
	template<size_t nbits, size_t es>
	int useed_scale() {
		return (uint32_t(1) << es);
	}

	// calculate exponential scale of maxpos
	template<size_t nbits, size_t es>
	int maxpos_scale() {
		return (nbits - 2) * (1 << es);
	}

	// calculate exponential scale of minpos
	template<size_t nbits, size_t es>
	int minpos_scale() {
		return static_cast<int>(2 - int(nbits)) * (1 << es);
	}

	// calculate the constrained k value
	template<size_t nbits, size_t es>
	int calculate_k(int scale) {
		// constrain the scale to range [minpos, maxpos]
		if (scale < 0) {
			scale = scale > minpos_scale<nbits, es>() ? scale : minpos_scale<nbits, es>();
		}
		else {
			scale = scale < maxpos_scale<nbits, es>() ? scale : maxpos_scale<nbits, es>();
		}
		// bad int k = scale < 0 ? -(-scale >> es) - 1 : (scale >> es);
		// the scale of a valid is  2 ^ scale = useed ^ k * 2 ^ exp
		// -> (scale >> es) = (k*2^es + exp) >> es
		// -> (scale >> es) = k + (exp >> es) -> k = (scale >> es)
		int k = scale < 0 ? -(-scale >> es) : (scale >> es);
		if (k == 0 && scale < 0) {
			// project back to south-east quadrant
			k = -1;
		}
		return k;
	}

	// calculate the unconstrained k value
	template<size_t nbits, size_t es>
	int calculate_unconstrained_k(int scale) {
		// the scale of a valid is  2 ^ scale = useed ^ k * 2 ^ exp
		// -> (scale >> es) = (k*2^es + exp) >> es
		// -> (scale >> es) = k + (exp >> es) 
		// -> k = (scale >> es)
		int k = scale < 0 ? -(-scale >> es) : (scale >> es);
		if (k == 0 && scale < 0) {
			// project back to south-east quadrant
			k = -1;
		}
		return k;
	}

	// double value representation of the useed value of a valid<nbits, es>
	template<size_t nbits, size_t es>
	double useed() {
		return std::pow(2.0, std::pow(2.0, es));
	}

	// calculate the value of useed
	template<size_t nbits, size_t es>
	double useed_value() {
		return double(uint64_t(1) << useed_scale<nbits, es>());
	}

	// calculate the value of maxpos
	template<size_t nbits, size_t es>
	long double maxpos_value() {
		return std::pow((long double)(useed_value<nbits, es>()), (long double)(nbits - 2));
	}

	// calculate the value of minpos
	template<size_t nbits, size_t es>
	long double minpos_value() {
		return std::pow((long double)(useed_value<nbits, es>()), (long double)(static_cast<int>(2 - int(nbits))));
	}

	// generate the minpos bit pattern for the sign requested (true is negative half, false is validive half)
	template<size_t nbits, size_t es>
	bitblock<nbits> minpos_pattern(bool sign = false) {
		bitblock<nbits> _bits;
		_bits.reset();
		_bits.set(0, true);
		return (sign ? twos_complement(_bits) : _bits);
	}

	// generate the maxpos bit pattern for the sign requested (true is negative half, false is validive half)
	template<size_t nbits, size_t es>
	bitblock<nbits> maxpos_pattern(bool sign = false) {
		bitblock<nbits> _bits;
		_bits.reset();
		_bits.flip();
		_bits.set(nbits - 1, false);
		return (sign ? twos_complement(_bits) : _bits);
	}
	*/

	// forward reference
	template<size_t nbits, size_t es> class valid;

	template<size_t nbits, size_t es>
	inline int sign_value(const valid<nbits, es>& v) {
		int s = 1;
		std::cerr << "sign_value(valid) not implemented yet";
		return s;
	}

	template<size_t nbits, size_t es>
	inline long double regime_value(const valid<nbits, es>& v) {
		std::cerr << "regime_value(valid) not implemented yet";
		return 0.0l;
	}

	template<size_t nbits, size_t es>
	inline long double exponent_value(const valid<nbits, es>& v) {
		std::cerr << "exponent_value(valid) not implemented yet";
		return 0.0l;
	}

	template<size_t nbits, size_t es>
	inline long double fraction_value(const valid<nbits, es>& p) {
		std::cerr << "fraction_value(valid) not implemented yet";
		return 0.0l;
	}

	// get the sign of the valid
	template<size_t nbits, size_t es>
	inline bool sign(const valid<nbits, es>& v) {
		return v.isneg();
	}

	// calculate the scale of a valid
	template<size_t nbits, size_t es>
	inline int scale(const valid<nbits, es>& v) {
		std::cerr << "scale(valid) not implemented yet";
		return 0; // return the scale
	}

	// calculate the significant of a valid
	template<size_t nbits, size_t es, size_t fbits>
	inline bitblock<fbits+1> significant(const valid<nbits, es>& v) {
		std::cerr << "significant(valid) not implemented yet";
		return bitblock<fbits+1>();
	}

	// get the fraction bits of a valid
	template<size_t nbits, size_t es, size_t fbits>
	inline bitblock<fbits> extract_fraction(const valid<nbits, es>& v) {
		//constexpr size_t fbits = nbits - 3 - es;
		std::cerr << "extract_fraction(valid) not implemented yet";
		return bitblock<fbits>();
	}

	// calculate the scale of the regime component of the valid
	template<size_t nbits, size_t es>
	inline int regime_scale(const valid<nbits, es>& v) {
		std::cerr << "regime_scale(valid) not implemented yet";
		return 0;
	}

	// calculate the scale of the exponent component of the valid
	template<size_t nbits, size_t es>
	inline int exponent_scale(const valid<nbits, es>& v) {
		std::cerr << "exponent_scale(valid) not implemented yet";
		return 0;
	}


} // namespace unum
} // namespace sw
