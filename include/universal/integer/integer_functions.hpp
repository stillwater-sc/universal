#pragma once
// integer_functions.hpp: definition of helper functions for integer type
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include "./integer_exceptions.hpp"

#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */


#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */


#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */


#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/aC++. ---------------------------------- */

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */


#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */

#endif

namespace sw {
namespace unum {

	/* from numerics
		// FUNCTION TEMPLATE gcd
template<class _Mt,	class _Nt>
	_NODISCARD constexpr common_type_t<_Mt, _Nt> gcd(const _Mt _Mx, const _Nt _Nx) noexcept // strengthened
	{	// calculate greatest common divisor
	static_assert(_Is_nonbool_integral<_Mt>::value && _Is_nonbool_integral<_Nt>::value,
		"GCD requires nonbool integral types");

	using _Common = common_type_t<_Mt, _Nt>;
	using _Common_unsigned = make_unsigned_t<_Common>;
	_Common_unsigned _Mx_magnitude = _Abs_u(_Mx);
	_Common_unsigned _Nx_magnitude = _Abs_u(_Nx);
	if (_Mx_magnitude == 0U)
		{
		return (static_cast<_Common>(_Nx_magnitude));
		}

	if (_Nx_magnitude == 0U)
		{
		return (static_cast<_Common>(_Mx_magnitude));
		}

	const auto _Mx_trailing_zeroes = _Stl_bitscan_forward(_Mx_magnitude);
	const auto _Common_factors_of_2 = _Min_value(_Mx_trailing_zeroes,
		_Stl_bitscan_forward(_Nx_magnitude));
	_Nx_magnitude >>= _Common_factors_of_2;
	_Mx_magnitude >>= _Mx_trailing_zeroes;
	do
		{
		_Nx_magnitude >>= _Stl_bitscan_forward(_Nx_magnitude);
		if (_Mx_magnitude > _Nx_magnitude)
			{
			_Common_unsigned _Temp = _Mx_magnitude;
			_Mx_magnitude = _Nx_magnitude;
			_Nx_magnitude = _Temp;
			}

		_Nx_magnitude -= _Mx_magnitude;
		}
	while (_Nx_magnitude != 0U);

	return (static_cast<_Common>(_Mx_magnitude << _Common_factors_of_2));
	}

	// FUNCTION TEMPLATE lcm
template<class _Mt,
	class _Nt>
	_NODISCARD constexpr common_type_t<_Mt, _Nt> lcm(const _Mt _Mx, const _Nt _Nx) noexcept // strengthened
	{	// calculate least common multiple
	static_assert(_Is_nonbool_integral<_Mt>::value && _Is_nonbool_integral<_Nt>::value,
		"LCM requires nonbool integral types");
	using _Common = common_type_t<_Mt, _Nt>;
	using _Common_unsigned = make_unsigned_t<_Common>;
	const _Common_unsigned _Mx_magnitude = _Abs_u(_Mx);
	const _Common_unsigned _Nx_magnitude = _Abs_u(_Nx);
	if (_Mx_magnitude == 0 || _Nx_magnitude == 0)
		{
		return (static_cast<_Common>(0));
		}

	return (static_cast<_Common>((_Mx_magnitude / _STD gcd(_Mx_magnitude, _Nx_magnitude))
		* _Nx_magnitude));
	}
	*/

/*
 given two positive integers a = Product of primes p^a_p, and b = PROD p^b_p,
   where a_p or b_p is the exponent of the prime p that are contained by a or b
 greated common divisor gcd(a, b) = PROD p^min(a_p, b_p)
 least common multiple  lcm(a, b) = PROD p^max(a_p, b_p)
 */

// calculate the greatest common divisor of two numbers
template<size_t nbits, typename BlockType>
integer<nbits, BlockType> gcd(const integer<nbits, BlockType>& a, const integer<nbits, BlockType>& b) {
	return b.iszero() ? a : gcd(b, a % b);
}

// calculate the greatest common divisor of N numbers
template<size_t nbits, typename BlockType>
integer<nbits, BlockType> gcd(const std::vector< integer<nbits, BlockType> >& v) {
	if (v.size() == 0) return 0;
	if (v.size() == 1) return v[0];
	integer<nbits, BlockType> gcd_n = v[0];
	for (size_t i = 1; i < v.size(); ++i) {
		gcd_n = gcd(gcd_n, v[i]);
	}
	return gcd_n;
}

// calculate the least common multiple of two numbers
template<size_t nbits, typename BlockType>
integer<nbits, BlockType> lcm(const integer<nbits, BlockType>& a, const integer<nbits, BlockType>& b) {
	return (a * b) / gcd(a, b);
}

// calculate the least common multiple of N numbers
template<size_t nbits, typename BlockType>
integer<nbits, BlockType> lcm(const std::vector< integer<nbits, BlockType> >& v) {
	if (v.size() == 0) return 0;
	if (v.size() == 1) return v[0];
	integer<nbits, BlockType> lcm = v[0];
	for (size_t i = 0; i < v.size(); ++i) {
		lcm = (v[i] * lcm) / gcd(lcm, v[i]);
	}
	return lcm;
}

// check if a number is prime
template<size_t nbits, typename BlockType>
bool isPrime(const integer<nbits, BlockType>& a) {
	if (a.iszero() || a == 1) return false; // smallest prime number is 2
	for (integer<nbits, BlockType> i = 2; i < a / 2; ++i) if ((a % i) == 0) return false;
	return true;
}

template<size_t nbits, typename BlockType>
bool primeNumbersInRange(const integer<nbits, BlockType>& low, const integer<nbits, BlockType>& high, std::vector< integer<nbits, BlockType> >& primes) {
	bool bFound = false;
	for (integer<nbits, BlockType> i = low; i < high; ++i) {
		if (isPrime(i)) {
			primes.push_back(i);
			bFound = true;
		}
	}
	return bFound;
}

// calculate the integer power a ^ b
// exponentiation by squaring is the standard method for modular exponentiation of large numbers in asymmetric cryptography
template<size_t nbits, typename BlockType>
integer<nbits, BlockType> ipow(const integer<nbits, BlockType>& a, const integer<nbits, BlockType>& b) {
	integer<nbits, BlockType> result(1), base(a), exp(b);
	for (;;) {
		if (exp.isodd()) result *= base;
		exp >>= 1;
		if (exp == 0) break;
		base *= base;
	}
	return result;
}

} // namespace unum
} // namespace sw
