#pragma once
// numeric_limits.hpp: definition of numeric_limits for integer types
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>
// TODO: is this the proper way to go about this type? 
// For big integers, the return types will not yield standard types
namespace std {

	using namespace sw::universal;
/*
	Trait class that identifies whether T is a literal type.

A literal type is a type that can qualify as constexpr. This is true for scalar types, references, 
certain classes, and arrays of any such types.

A class that is a literal type is a class (defined with class, struct or union) that :
1- has a trivial destructor,
2- every constructor call and any non - static data member that has brace - or equal - initializers is a constant expression,
3- is an aggregate type, or has at least one constexpr constructor or constructor template that is not a copy or move constructor, and
4- has all non - static data members and base classes of literal types

	TODO: how to make the integer class a literal type so that we can use it as a return type for min/max/lowest etc.
*/
template <unsigned nbits, typename bt> 
class numeric_limits< sw::universal::integer<nbits, bt> > {
public:
	using Integer = sw::universal::integer<nbits, bt>;
	static constexpr bool is_specialized = true;
	static constexpr Integer min() { return 1; } // return minimum value
	static constexpr Integer max() {             // return maximum value
		Integer imax(0); // sw::universal::integers are 2's complement encoded numbers
		imax.setbit(nbits - 1);
		imax.flip();
		return imax;
	} 
	static constexpr Integer lowest() { // return most negative value
		Integer ilowest(0); // 2's complement maxneg is 1000...0000
		ilowest.setbit(nbits - 1);
		return ilowest;
	}
	static constexpr float epsilon() { // return smallest effective increment from 1.0
		return 1.0f;
	}
	static constexpr float round_error() { // return largest rounding error
		return 0.5f;
	}
	static constexpr float denorm_min() {  // return minimum denormalized value
		return 1.0f; 
	}
	static constexpr Integer infinity() { // return positive infinity
		return max(); 
	}
	static constexpr Integer quiet_NaN() { // return non-signaling NaN
		return 0; 
	}
	static constexpr Integer signaling_NaN() { // return signaling NaN
		return 0;
	}

	static constexpr int digits       = nbits - 1;
	static constexpr int digits10     = static_cast<int>(digits / 3.3f);
	static constexpr int max_digits10 = digits10;
	static constexpr bool is_signed   = true;
	static constexpr bool is_integer  = true;
	static constexpr bool is_exact    = true;
	static constexpr int radix        = 2;

	static constexpr int min_exponent   = 0;
	static constexpr int min_exponent10 = 0;
	static constexpr int max_exponent   = nbits - 1;
	static constexpr int max_exponent10 = static_cast<int>(max_exponent / 3.3f);
	static constexpr bool has_infinity  = false;
	static constexpr bool has_quiet_NaN = false;
	static constexpr bool has_signaling_NaN = false;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss = false;

	static constexpr bool is_iec559 = false;
	static constexpr bool is_bounded = true;
	static constexpr bool is_modulo = true;
	static constexpr bool traps = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_toward_zero;
};

}
