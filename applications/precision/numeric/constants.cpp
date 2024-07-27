// numbers.cpp: example program to use C++20 <numbers> high precision constants
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#if (__cplusplus == 202003L) || (_MSVC_LANG == 202003L)
#include <numbers>    // high-precision numbers
#endif

// select the number systems we would like to compare
#define FIXPNT_NATIVE_SQRT 1
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>

/*
	Generate irrational constants with high precision using native SQRT algorithms
	in different number systems.

	The irrational constants of interest are the square roots:
	sqrt(2)
	sqrt(3)
	sqrt(5)
	sqrt(7)
 */

#ifdef PERFORMANCE_TESTING

#include <universal/verification/performance_runner.hpp>

template<typename Scalar>
void SqrtWorkload(size_t NR_OPS) {
	Scalar a{ 0 }, c{ 0 };
	size_t maxval = 1024*1024*1024;
	for (size_t i = 0; i < NR_OPS; ++i) {
		a = Scalar(i % maxval);
		a = (a < 0 ? -a : a);
		// std::cout << "a = " << a << '\n';
		a += Scalar(1.0f);
		if (a < 0) continue;
		c = sw::universal::sqrt(a);
	}
	if (a == c) std::cout << "amazing\n";
}

void PerformanceTest() {
	using namespace sw::universal;
	using Fixed = fixpnt<80, 75>;
	//	using Posit = posit<64, 2>;
	using HP = cfloat< 16, 5, uint32_t, true>;
	using SP = cfloat< 32, 8, uint32_t, true>;
	using DP = cfloat< 64, 11, uint32_t, true>;
	//	using EP = cfloat< 80, 11, uint32_t, true>;
	//	using QP = cfloat<128, 15, uint32_t, true>;

	constexpr size_t NR_OPS = 1024;
	PerformanceRunner(type_tag(Fixed()) + "::sqrt ", SqrtWorkload< Fixed >, NR_OPS);
	PerformanceRunner(type_tag(HP()) + "::sqrt ", SqrtWorkload< HP >, NR_OPS);
	PerformanceRunner(type_tag(SP()) + "::sqrt ", SqrtWorkload< SP >, NR_OPS);
	PerformanceRunner(type_tag(DP()) + "::sqrt ", SqrtWorkload< DP >, NR_OPS);
	//	PerformanceRunner(type_tag(EP()) + "::sqrt ", SqrtWorkload< EP >, NR_OPS);
	//	PerformanceRunner(type_tag(QP()) + "::sqrt ", SqrtWorkload< QP >, NR_OPS);
}
#endif

template<typename Scalar>
void Sqrt(double v) {
	Scalar s{ v };
	s = sqrt(s);  // sqrt in template type
	std::cout << std::setw(20) << type_tag(s) << " : " << s << '\n';
}

template<typename Scalar>
void CompareBabylonianMethods(double v) {
	std::cout << "sqrt(" << v << ")\n";
	Sqrt<Scalar>(v);
	Scalar a(v);
	auto b = BabylonianMethod(a);
	std::cout << b * b << '\n';
	b = BabylonianMethod2(a);
	std::cout << b * b << '\n';
	b = BabylonianMethod3(a);
	std::cout << b * b << '\n';
}

int main()
try {
	using namespace sw::universal;

	std::cout << "high-precision constants\n";

#if LONG_DOUBLE_SUPPORT
	using Native = long double;
	std::string native = "long double";
#else
	using Native = double;
	std::string native = "double";

#endif
	using Fixed = fixpnt<80,75>;
	using Posit = posit<64,2>;
	using HP = cfloat< 16,  5, uint32_t, true>;
	using SP = cfloat< 32,  8, uint32_t, true>;
	using DP = cfloat< 64, 11, uint32_t, true>;
	using EP = cfloat< 80, 11, uint32_t, true>;
	using QP = cfloat<128, 15, uint32_t, true>;
	//using Areal = areal<128, 15,uint32_t>;
	//using Lns   = lns<128, uint32_t>;

	std::streamsize precision = std::cout.precision();
	std::cout << std::setprecision(std::numeric_limits<Native>::max_digits10);
	std::cout << native << " digits of precision : " << std::numeric_limits<Native>::max_digits10 << '\n';

//	CompareBabylonianMethods<Fixed>(2.0);

	// MSVC doesn't support proper long double: this is guarded with a compile guard: LONG_DOUBLE_SUPPORT
	{
		std::cout << "sqrt(2)\n";
		float f = 2.0f;
		std::cout << sqrt(Native(f)) << " : " << type_tag(Native()) << '\n';
		std::cout << sqrt(Fixed(f)) << " : " << type_tag(Fixed()) << '\n';

		std::cout << sqrt(Posit(f)) << " : " << type_tag(Posit()) << '\n';
		std::cout << sqrt(HP(f)) << " : " << type_tag(HP()) << '\n';
		std::cout << sqrt(SP(f)) << " : " << type_tag(SP()) << '\n';
		std::cout << sqrt(DP(f)) << " : " << type_tag(DP()) << '\n';
		std::cout << sqrt(EP(f)) << " : " << type_tag(EP()) << '\n';
		std::cout << sqrt(QP(f)) << " : " << type_tag(QP()) << '\n';
	}

	{
		std::cout << "sqrt(3)\n";
		float f = 3.0f;
		std::cout << sqrt(Native(f)) << " : " << type_tag(Native()) << '\n';
		std::cout << sqrt(Fixed(f)) << " : " << type_tag(Fixed()) << '\n';
		std::cout << sqrt(Posit(f)) << " : " << type_tag(Posit()) << '\n';
	}

	{
		std::cout << "sqrt(5)\n";
		float f = 5.0f;
		std::cout << sqrt(Native(f)) << " : " << type_tag(Native()) << '\n';
		std::cout << sqrt(Fixed(f)) << " : " << type_tag(Fixed()) << '\n';
		std::cout << sqrt(Posit(f)) << " : " << type_tag(Posit()) << '\n';
	}

	{
		std::cout << "sqrt(7)\n";
		float f = 7.0f;
		std::cout << sqrt(Native(f)) << " : " << type_tag(Native()) << '\n';
		std::cout << sqrt(Fixed(f)) << " : " << type_tag(Fixed()) << '\n';
		std::cout << sqrt(Posit(f)) << " : " << type_tag(Posit()) << '\n';
	}

	compareNumberTraits<Posit, Fixed>(std::cout);

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
