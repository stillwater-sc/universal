// posit_properties.cpp example program comparing epsilon and minpos across posit configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <limits>
#include <cmath>
#include <utility>
#include <array>

// select the number systems we would like to compare
#include <universal/number/integer/integer.hpp>
//#include <universal/fixpnt/fixpnt>   // TODO: this causes this warning for an unknown reason:
// include\universal/posit/posit.hpp(851,1): warning C4305: 'specialization': truncation from 'const size_t' to 'bool'
#include <universal/number/areal/areal.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/valid/valid.hpp>

//constexpr long double pi     = 3.14159265358979323846;
//constexpr long double e      = 2.71828182845904523536;
//constexpr long double log_2e = 1.44269504088896340736;

template<size_t n>
struct Fib {
  static const size_t val = Fib<n-1>::val + Fib<n-2>::val;
};

template<>
struct Fib<0> {
  static const size_t val = 0;
};

template<>
struct Fib<1> {
  static const size_t val = 1;
};

// create a compile time table using a variadic template
template<size_t ... N>
size_t fib_impl(std::index_sequence<N...>, size_t n) 
{
	constexpr std::array<size_t, sizeof...(N)> a 
		= { Fib<N>::val... };
    return a[n];
}

size_t fibonacci(size_t n) {
  return fib_impl(std::make_index_sequence<50>(), n);
}

/*
// but there is a closed solution to the Fibonacci sequence with the Binet formula

size_t fib2(const size_t n) {
   const size_t sqrt_5 = std::sqrt(5);

   if (n == 0) return 0;
   if (n == 1) return 1;

   return static_cast<size_t>((1 + sqrt_5, n) - std::pow(1 - sqrt_5, n)) / (std::pow(2, n) * sqrt_5);
}
*/

#define VARIADIC_EPSILON 0
#if VARIADIC_EPSILON

// operator=() of posit can't be constexpr due to bitset<> so this doesn't work until C++20

// variadic template to generate a range of nbits
template<size_t ... nbits>
long double eps_impl(std::index_sequence<nbits...>, size_t index) 
{
	constexpr std::array<size_t, sizeof...(nbits)> eps = { (long double)std::numeric_limits<sw::universal::posit<nbits+2,2>>::epsilon()... } ;
    return eps[index];
}

template<size_t es = 2>
const long double eps(size_t nbits) {
	return eps_impl(std::make_index_sequence<64>(), nbits);
}
#endif

template<size_t nbits, size_t es>
std::string properties(const std::string& label) {
	using Scalar = sw::universal::posit<nbits, es>;
	Scalar minpos(sw::universal::SpecificValue::minpos), maxpos(sw::universal::SpecificValue::maxpos);
	Scalar eps  = std::numeric_limits<Scalar>::epsilon();
	std::stringstream ostr;
	ostr << nbits
		<< '\t'
		<< label
		<< '\t'
		<< minpos
		<< '\t'
		<< eps
		<< '\t'
		<< maxpos
		<< '\t'
		<< eps / minpos
		<< '\t'
		<< maxpos / eps
		<< '\n';
	return ostr.str();
}

int main()
try {
	using namespace sw::universal;

	std::cout << "minpos/epsilon/maxpos for different number systems\n";

	// report on smallest number, precision and dynamic range of the number system

	std::streamsize precision = std::cout.precision();

//	std::cout << "Fibonacci(45) = " << fibonacci(45) << '\n';
//	std::cout << "posit<16,2> | 16 |  " << eps(16) << '\n';

	// operator=() of posit can't be constexpr due to bitset<>
	// constexpr long double posit_16_2_eps = (long double)std::numeric_limits<sw::universal::posit<6, 2>>::epsilon();
	// std::cout << "constexpr " << posit_16_2_eps;

	std::cout << "nbits\tposit\tminpos\tepsilon\tmaxpos\teps/minpos\tmaxpos/eps\n";
	std::cout << properties<8,0>("posit<8,0>");
	std::cout << properties<16, 1>("posit<16,1>");
	std::cout << properties<32, 2>("posit<32,2>");
	std::cout << properties<64, 3>("posit<64,3>");
	std::cout << properties<128, 4>("posit<128,4>");
	std::cout << properties<256, 5>("posit<256,5>");
	std::cout << "\n";


	std::cout << properties<6, 2>("posit<6,2> ");
	std::cout << properties<8, 2>("posit<8,2> ");
	std::cout << properties<10, 2>("posit<10,2>");
	std::cout << properties<12, 2>("posit<12,2>");
	std::cout << properties<14, 2>("posit<14,2>");
	std::cout << properties<16, 2>("posit<16,2>");
	std::cout << properties<18, 2>("posit<18,2>");
	std::cout << properties<20, 2>("posit<20,2>");
	std::cout << properties<24, 2>("posit<24,2>");
	std::cout << properties<28, 2>("posit<28,2>");
	std::cout << properties<32, 2>("posit<32,2>");
	std::cout << properties<36, 2>("posit<36,2>");
	std::cout << properties<40, 2>("posit<40,2>");
	std::cout << properties<44, 2>("posit<44,2>");
	std::cout << properties<48, 2>("posit<48,2>");
	std::cout << properties<52, 2>("posit<52,2>");
	std::cout << properties<56, 2>("posit<56,2>");
	std::cout << properties<60, 2>("posit<60,2>");
	std::cout << properties<64, 2>("posit<64,2>");
	std::cout << properties<72, 2>("posit<72,2>");
	std::cout << properties<80, 2>("posit<80,2>");
	std::cout << properties<88, 2>("posit<88,2>");
	std::cout << properties<96, 2>("posit<96,2>");
	std::cout << properties<104, 2>("posit<104,2>");
	std::cout << properties<112, 2>("posit<112,2>");
	std::cout << properties<120, 2>("posit<120,2>");
	std::cout << properties<128, 2>("posit<128,2>");
	std::cout << properties<144, 2>("posit<144,2>");
	std::cout << properties<160, 2>("posit<160,2>");
	std::cout << properties<176, 2>("posit<176,2>");
	std::cout << properties<192, 2>("posit<192,2>");
	std::cout << properties<208, 2>("posit<208,2>");
	std::cout << properties<224, 2>("posit<224,2>");
	std::cout << properties<240, 2>("posit<240,2>");
	std::cout << properties<256, 2>("posit<256,2>");

	std::cout << std::setprecision(precision);
	std::cout << std::endl;
	
	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
