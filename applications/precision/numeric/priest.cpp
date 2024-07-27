// priest.cpp: experiments with Douglas Priest's arbitrary precision floating-point arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <utility>
#if (__cplusplus == 202003L) || (_MSVC_LANG == 202003L)
#include <numbers>    // high-precision numbers
#endif
#include <universal/benchmark/performance_runner.hpp>

// select the number systems we would like to compare
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>

/*
Definition of FAITHFUL arithmetic
   For a t-digit number a and b, and op element {+,-,*,/}, let c = a op b exactly.
   Suppose x and y are consecutive t-digit floating-point numbers with the same 
   sign as c such at |x| <= |c| < |y|. Then the floating-point arithmetic is
   called faithful if fl(a op b) = x whenever c = x and fl(a op b) is either x or y
   whenever c != x.

 */

template<typename Real>
std::pair<Real, Real> SumErr(Real a, Real b) {
	using std::abs;
	Real c, d, e, f, g, h;

	if (abs(a) < abs(b)) std::swap(a, b);
	c = a + b;
	e = c - a;
	g = c - e;
	h = g - a;
	f = b - h;
	d = f - e;
	if (d + e != f) {
		c = a, d = b;
	}
	return std::pair(c, d);
}

template<typename Real>
std::pair<Real, Real> SumErrCorollary2(Real a, Real b) {
	using std::abs;
	Real c, d, e;

	if (abs(a) < abs(b)) std::swap(a, b);
	c = a + b;
	e = c - a;
	d = b - e;

	return std::pair(c, d);
}

template<typename Real>
void TestSumErr() {
	using namespace sw::universal;
	using Float = sw::universal::cfloat<32, 8, uint32_t>;
	Float fa, fb;
	Real a, b, c, d;

	fa = 1.0f;
	++fa;
	fb = 1.0f;
	--fb;
	a = float(fa);
	b = float(fb);
	auto result = SumErr(a, b);
	c = result.first;
	d = result.second;
	std::cout << c << " + " << d << " = " << a << " + " << b << '\n';
	std::cout << to_binary(c) << " : " << to_binary(a + b) << '\n';
	std::cout << to_binary(d) << '\n';
	std::cout << "a : " << to_binary(a) << '\n';
	std::cout << "b : " << to_binary(b) << '\n';
	std::cout << "c : " << to_binary(c) << '\n';
	std::cout << "d : " << to_binary(d) << '\n';

	std::cout << "Corollary2: simplified\n";
	result = SumErrCorollary2(a, b);
	c = result.first;
	d = result.second;
	std::cout << c << " + " << d << " = " << a << " + " << b << '\n';
	std::cout << to_binary(c) << " : " << to_binary(a + b) << '\n';
	std::cout << to_binary(d) << '\n';
	std::cout << "a : " << to_binary(a) << '\n';
	std::cout << "b : " << to_binary(b) << '\n';
	std::cout << "c : " << to_binary(c) << '\n';
	std::cout << "d : " << to_binary(d) << '\n';
}

template<typename Real>
std::pair<Real, Real> split(Real x, unsigned k) {
	using namespace sw::universal;
	unsigned t = ieee754_parameter<Real>::fbits;
	Real a_k = static_cast<Real>((1ull << (t - k)) + 1);
	Real y = a_k * x;
	Real z = y - x;
	Real xp = y - z;
	Real xpp = x - xp;
	return std::pair(xp, xpp);
}

int main()
try {
	using namespace sw::universal;

	std::cout << "Douglas Priest arbitrary precision arithmetic experiments\n";

	//using Sngle = float;
	using Longd = long double;
	//using Fixed = fixpnt<32,16>;
	//using Posit = posit<32,2>;
	using Float = cfloat<32, 8, uint32_t>;
	//using Areal = areal<32, 8, uint32_t>;
	//using Lns   = lns<32, uint32_t>;


	std::streamsize precision = std::cout.precision();
	std::cout << std::setprecision(std::numeric_limits<Longd>::max_digits10);
	std::cout << "float       digits of precision : " << std::numeric_limits<float>::max_digits10 << '\n';
	std::cout << "double      digits of precision : " << std::numeric_limits<double>::max_digits10 << '\n';
	std::cout << "long double digits of precision : " << std::numeric_limits<Longd>::max_digits10 << '\n';

	TestSumErr<float>();
	TestSumErr<double>();

	std::cout << "\nSplitting of a floating-point value\n";
	Float fa{ 1.875f + 0.0625f + 0.03125f };
	++fa;
	float x = float(fa);
	for (unsigned k = 1; k < 10; ++k) {
		auto xs = split(x, k);
		float xp = xs.first;
		float xpp = xs.second;
		std::cout << "x   : " << to_binary(x) << " : " << x << '\n';
		std::cout << "x'  : " << to_binary(xp) << " : " << xp << '\n';
		std::cout << "x'' : " << to_binary(xpp) << " : " << xpp << '\n';
	}


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
