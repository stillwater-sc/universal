#pragma once
// primes.hpp: algorithms to create, categorize, classify, and identify prime factors
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <universal/number/integer/exceptions.hpp>

namespace sw { namespace universal {

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
template<typename IntegerType>
IntegerType gcd(const IntegerType& a, const IntegerType& b) {
	return b == 0 ? a : gcd(b, a % b);
}

// calculate the greatest common divisor of N numbers
template<typename IntegerType>
IntegerType gcd(const std::vector< IntegerType >& v) {
	if (v.size() == 0) return 0;
	if (v.size() == 1) return v[0];
	IntegerType gcd_n = v[0];
	for (size_t i = 1; i < v.size(); ++i) {
		gcd_n = gcd(gcd_n, v[i]);
	}
	return gcd_n;
}

// calculate the least common multiple of two numbers
template<typename IntegerType>
IntegerType lcm(const IntegerType& a, const IntegerType& b) {
	return (a * b) / gcd(a, b);
}

// calculate the least common multiple of N numbers
template<typename IntegerType>
IntegerType lcm(const std::vector< IntegerType >& v) {
	if (v.size() == 0) return 0;
	if (v.size() == 1) return v[0];
	IntegerType lcm = v[0];
	for (size_t i = 0; i < v.size(); ++i) {
		lcm = (v[i] * lcm) / gcd(lcm, v[i]);
	}
	return lcm;
}

// check if a number is prime
template<typename IntegerType>
bool isPrime_(const IntegerType& a) {
	if (a <= 1) return false; // smallest prime number is 2
	for (IntegerType i = 2; i <= a / 2; ++i) if ((a % i) == 0) return false;
	return true;
}

template<typename IntegerType>
bool isPrime(const IntegerType& a) {
	if (a <= 1) return false; // smallest prime number is 2
	if (a <= 3) return true; // 2 and 3 are primes
	if (a % 2 == 0 || a % 3 == 0) return false; // this allows us to skip middle 
	for (IntegerType i = 5; i*i <= a; i += 6) if ((a % i) == 0 || a % (i + 2) == 0) return false;
	return true;
}

template<typename IntegerType>
bool isPrimeTracer(const IntegerType& a) {
	if (a <= 1) return false; // smallest prime number is 2
	if (a <= 3) return true; // 2 and 3 are primes
	if (a % 2 == 0 || a % 3 == 0) return false; // this allows us to skip middle 
	for (IntegerType i = 5; i * i <= a; i += 6) {
		if ((a % i) == 0 || a % (i + 2) == 0) return false;
		std::cout << i << '\n';
	}
	return true;
}

// generate prime numbers in a range
template<typename IntegerType>
bool primeNumbersInRange(const IntegerType low, const IntegerType high, std::vector< IntegerType >& primes) {
	bool bFound = false;
	for (IntegerType i = low; i < high; ++i) {
		if (isPrime(i)) {
			primes.push_back(i);
			bFound = true;
		}
	}
	return bFound;
}

// print the prime numbers in a range
template<typename IntegerType>
void printPrimes(const std::vector< IntegerType >& v) {
	constexpr size_t PAGE_WIDTH = 65;
	size_t nrPrimes = v.size();
	if (nrPrimes == 0) return;
	// determine the size of the largest prime
	size_t COL_WIDTH = 1;
	auto number = v[nrPrimes - 1];
	while (number >= 1) {
		++COL_WIDTH;
		number /= 10;
	}
	std::cout << "largest prime: " << v[nrPrimes - 1] << " is " << COL_WIDTH - 1 << " decades\n";
	int column = 1;
	for (auto p : v) {
		std::cout << std::setw(COL_WIDTH) << p;
		if (column * COL_WIDTH < PAGE_WIDTH) {
			++column;
		}
		else {
			column = 1;
			std::cout << '\n';
		}
	}
	std::cout << '\n';
}

// prime factors of an arbitrary integer
template<typename IntegerType>
class primefactors : public std::vector< std::pair< IntegerType, IntegerType > > { };



// generate prime factors of an arbitrary integer
template<typename IntegerType>
void primeFactorization(const IntegerType& a, primefactors<IntegerType>& factors) {
	IntegerType i(a);
	IntegerType factor = 2;
	IntegerType power = 0;
	// powers of 2
	while (i.iseven()) { ++power; i >>= 1; }
	if (power > 0) factors.push_back(std::pair<IntegerType, IntegerType>(factor, power));
	// powers of odd numbers > 2
	for (factor = 3; factor <= sqrt(i); factor += 2) {
		if (isPrime(factor)) {
			power = 0;
			while ((i % factor) == 0) { ++power; i /= factor; }
			if (power > 0) factors.push_back(std::pair<IntegerType, IntegerType>(factor, power));
		}
	}
	if (i > 2) factors.push_back(std::pair < IntegerType, IntegerType>(i, 1));
}

// Factorization using Fermat's method: precondition number must be odd
// trying various values of a with the goal to find a^2 - number = b^2, a square
template<typename IntegerType>
IntegerType fermatFactorization(const IntegerType& number) {
	if (number.iseven()) return 0; // number must be odd
	IntegerType a = ceil_sqrt(number);
	IntegerType bsquare = a * a - number;
	while (!perfect_square(bsquare)) {
		++a;
		bsquare = a * a - number;
	}
	return a - sqrt(bsquare);
}

}} // namespace sw::universal
