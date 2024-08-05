//  api.cpp : test suite runner for the class interface of a simplified floating-point type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/internal/gfp/gfp.hpp>
#include <universal/verification/test_suite.hpp>

// https://en.cppreference.com/w/cpp/utility/feature_test
#include <version>

#if __cpp_lib_to_string >= 202110L
constexpr auto revision() { return " (post C++23)"; }
#define HAS_STD_FLOATS 1
#include <stdfloat>
#else
constexpr auto revision() { return " (pre C++23)"; }
#define HAS_STD_FLOATS 0
#endif

namespace sw {
	namespace universal {

		std::string to_string(uint64_t bits) {
			std::cout << "incoming: " << bits << "   log10() : " << std::log10(bits) << '\n';
			std::cout << "incoming: " << bits << "   log2()  : " << std::log2(bits) << '\n';
			std::cout << "incoming: " << bits << "   scale() : " << scale(float(bits)) << '\n';
			unsigned nrDigits = std::log10(bits) + 1;
			std::string str(nrDigits, '\0');
			char* p = &str.back();
			do {
				*p = bits % 10 + '0'; // extract least significant digit
				bits /= 10;
				std::cout << "digits  : " << str << '\n';
				--p;
			} while (bits);
			return str;
		}
	}
}
int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "gfp decimal string conversion validation";
	std::string test_tag    = "API";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////         construction
	{
		gfp<uint32_t> a, b, c;
		a = 1.0e0f;
		std::cout << to_binary(a) << '\n';
		b = 1.0e0f;
		c = a + b;
		std::cout << a << " * " << b << " = " << c << '\n';
	}

	{
		gfp<uint64_t> a, b, c;
		a = 1.0e0;
		std::cout << to_binary(a) << '\n';
		b = 1.0e0;
		c = a + b;
		std::cout << a << " * " << b << " = " << c << '\n';
	}

	{
		gfp<uint64_t> a, b, c;
		a.set(false, 0, 0xf'ffff'ffffull, 52); // emulate a double
		b.set(false, 0, 0x1'ffff'ffffull, 52); // emulate a double);
		std::cout << to_hex(b.significant()) << '\n';
		c = a * b;
		std::cout << a << " * " << b << " = " << c << '\n';
		int alpha = 0;
		std::cout << "alpha : " << alpha << "  k : " << c.calculate_k(alpha) << '\n';
	}

	{
		int alpha = 0;
		for (int binaryScale = -10; binaryScale <= 64; ++binaryScale) {
			std::cout << "binaryScale : " << binaryScale << " vs decimalScale : " << decimalScale(binaryScale, 64, alpha) << '\n';
		}
	}

	{
		using Scalar = std::uint32_t;
		gfp<Scalar> a;
		a = 1.0f;
		std::cout << to_binary(a) << " : " << float(a) << '\n';

		a = 0.03125f;
		std::cout << to_binary(a) << " : " << float(a) << '\n';
	}

	{
		using Scalar = std::uint64_t;
		gfp<Scalar> a;
		a = 1.0;
		std::cout << to_binary(a) << " : " << double(a) << '\n';

		a = 0.03125;
		std::cout << to_binary(a) << " : " << double(a) << '\n';
	}

	{
		half f{ 0.03125 };
		std::cout << "floating-point value : " << to_binary(f) << " : " << f << " : " << to_triple(f) << '\n';

		duble d{ 0.0312 };
		std::cout << "floating-point value : " << to_binary(d) << " : " << d << " : " << to_triple(d) << '\n';
	}

	{
		// basic to_string algorithm
		std::string digits = sw::universal::to_string(1024 * 1024 * 1024);
		std::cout << "1024 * 1024 * 1024 : " << digits << '\n';
	}

#if HAS_STD_FLOATS
	{
		// for C++23
		// defined in <stdfloat>

#if __STDCPP_FLOAT64_T__ != 1
#error "64-bit float type required"
#endif
		// testing new float types
		std::float32_t f{ 1.0f };
		std::float64_t d{ 1.0 };
		std::float128_t l{ 1.0l };
	}
#endif

	{
		std::cout << grisu<std::uint64_t>(1.0) << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
