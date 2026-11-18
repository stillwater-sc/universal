#pragma once
// numeric_limits.hpp: definition of numeric_limits for integer types
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>
#include <universal/utility/decimal_digits.hpp>   // exact digits10 / max_digits10 (#1597, #1601)
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
// every kind of integer (#2131): an IntegerNumber is a two's complement signed type; WholeNumber
// {0, 1, 2, ...} and NaturalNumber {1, 2, 3, ...} are unsigned kinds whose smallest values are 0
// and 1 (USER decision on #2131: the documented domains, which the operators now check). The non-floating members return
// the type's zero, as the standard asks of an integer type: epsilon, round_error, denorm_min and
// infinity used to return float, and min() returned 1 instead of the most negative value.
template <unsigned nbits, typename bt, sw::universal::IntegerNumberType NumberType>
class numeric_limits< sw::universal::integer<nbits, bt, NumberType> > {
	static constexpr bool isSigned = (NumberType == sw::universal::IntegerNumberType::IntegerNumber);
public:
	using Integer = sw::universal::integer<nbits, bt, NumberType>;
	static constexpr bool is_specialized = true;
	static constexpr Integer min() noexcept {   // the smallest finite value, as for a native integer
		Integer v{};   // zero limbs via the defaulted ctor: Integer(0) would call the domain-checked conversion, which throws for a NaturalNumber
		if constexpr (isSigned) v.setbit(nbits - 1);                                   // 100...0
		else if constexpr (NumberType == sw::universal::IntegerNumberType::NaturalNumber) v.setbit(0);   // 1
		return v;
	}
	static constexpr Integer max() noexcept {
		Integer v{};   // zero limbs via the defaulted ctor: Integer(0) would call the domain-checked conversion, which throws for a NaturalNumber
		v.flip();                                  // all ones
		if constexpr (isSigned) v.setbit(nbits - 1, false);   // 011...1
		return v;
	}
	static constexpr Integer lowest() noexcept { return min(); }
	static constexpr Integer epsilon()       noexcept { return Integer{}; }
	static constexpr Integer round_error()   noexcept { return Integer{}; }
	static constexpr Integer denorm_min()    noexcept { return Integer{}; }
	static constexpr Integer infinity()      noexcept { return Integer{}; }   // has_infinity is false
	static constexpr Integer quiet_NaN()     noexcept { return Integer{}; }
	static constexpr Integer signaling_NaN() noexcept { return Integer{}; }

	static constexpr int digits       = isSigned ? static_cast<int>(nbits) - 1 : static_cast<int>(nbits);
	static constexpr int digits10     = sw::universal::decimal_digits10_integer(digits);
	static constexpr int max_digits10 = 0;   // integer types have no decimal round trip to size
	static constexpr bool is_signed   = isSigned;
	static constexpr bool is_integer  = true;
	static constexpr bool is_exact    = true;
	static constexpr int radix        = 2;

	static constexpr int min_exponent   = 0;
	static constexpr int min_exponent10 = 0;
	static constexpr int max_exponent   = digits;
	static constexpr int max_exponent10 = sw::universal::decimal_max_exponent10(max_exponent);
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
