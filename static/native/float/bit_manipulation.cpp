//  bit_manipulation.cpp: experiments with the C++20 <bit> library
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp> // set up the compiler environment to support long doubles
#include <universal/utility/bit_cast.hpp>    // set up the compiler environment to support C++20 <bit> library
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <universal/native/ieee754.hpp>
#include <universal/native/ieee754_float.hpp>
#include <universal/native/ieee754_double.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	int VerifyRealFieldExtraction(bool reportTestCases) {
		using namespace sw::universal;

		constexpr unsigned fbits = ieee754_parameter<Real>::fbits;

		int nrOfFailedTests = 0;

#if defined(UNIVERSAL_ARCH_X86_64)
std::cout << "Architecture is x86_64\n";
#elif defined(UNIVERSAL_ARCH_ARM)
std::cout << "Architecture is ARM\n";
#else
std::cout << "Architecture is unknown\n";
#endif
		bool sign{ false };
		uint64_t rawExponent{ 0 };
		int exponent{ 0 };
		uint64_t rawFraction{ 0 };
		uint64_t bits{ 0 };
		
		Real a;
		a = 1.0;
		std::cout << to_binary(a) << " : " << a << '\n';
		ReportValue(a);

		extractFields(a, sign, rawExponent, rawFraction, bits);
		exponent = static_cast<int>(rawExponent) - ieee754_parameter<Real>::bias;
		std::cout << "sign              : " << (sign ? "1\n" : "0\n");
		std::cout << "rawExponent       : " << rawExponent << '\n';
		std::cout << "exponent bias     : " << ieee754_parameter<Real>::bias << '\n';
		std::cout << "unbiased exponent : " << exponent << '\n';
		if (sign != false || exponent != 0 || rawFraction != 0) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "fp components: " << (sign ? '1' : '0') << " exp: " << exponent << " frac: " << to_binary(rawFraction, fbits, true) << '\n';
		}

		return nrOfFailedTests;
	}

} } // namespace sw::universal

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "native IEEE-754 bit manipulation verification";
	std::string test_tag    = "floating-point field extraction";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// create a float with the following layout
	// b1.00001111.00011001011010001001001"
	float_decoder decoder;
	uint64_t fraction = 0b00011001011010001001001;
	uint64_t exponent = 0b0000'0001;
	bool     sign     = 0b1;
	decoder.parts.fraction = fraction;
	decoder.parts.exponent = exponent;
	decoder.parts.sign     = sign;
	float f = decoder.f;
	std::cout << to_binary(f) << " : " << f << '\n';

	// using the Universal non-const functions
	double value{ 0.0 };
	setFields(value, sign, exponent, fraction);
	std::cout << to_binary(value) << " : " << value << '\n';

	// do the reverse
	uint32_t bc = sw::bit_cast<uint32_t, float>(f);
	std::cout << to_binary(bc, 32) << '\n';

	f = 1.0f;	
	std::cout << "size of float       : " << sizeof(f) << '\n';
		ReportValue(f);
	double d{1.0};	
	std::cout << "size of double      : " << sizeof(d) << '\n';
		ReportValue(d);
	long double ld{1.0l};	
	std::cout << "size of long double : " << sizeof(ld) << '\n';
		ReportValue(ld);

	{
	/*
        struct blob {
		    std::uint64_t hi;
		    std::uint64_t fraction;
	    } raw;
	    raw = std::bit_cast<blob, long double>(value);
		std::cout << "sign mask     : " << to_binary(ieee754_parameter<long double>::smask) << '\n';
		std::cout << "exponent mask : " << to_binary(ieee754_parameter<long double>::emask) << '\n';
        std::cout << "fraction mask : " << to_binary(ieee754_parameter<long double>::fmask) << '\n';
		std::cout << "hi            : " << to_binary(raw.hi) << '\n';
		std::cout << "fraction bits : " << to_binary(raw.fraction) << '\n';
		bool s = (ieee754_parameter<long double>::smask & raw.hi);
		uint64_t eBits = (ieee754_parameter<long double>::emask & raw.hi);
		uint64_t fBits = (ieee754_parameter<long double>::fmask & raw.fraction);
		std::cout << "sign          : " << (s ? "1\n" : "0\n");
		std::cout << "eBits         : " << to_binary(eBits) << '\n';
		std::cout << "fBits         : " << to_binary(fBits) << '\n';
	*/
		long_double_decoder decoder;
		decoder.ld = 1.0l;
		std::cout << "sign          : " << (decoder.parts.sign ? "1\n" : "0\n");
		std::cout << "eBits         : " << to_binary(decoder.parts.exponent) << '\n';
		std::cout << "fBits         : " << to_binary(decoder.parts.fraction) << '\n';
	}

	nrOfFailedTestCases += ReportTestResult(VerifyRealFieldExtraction<float>(reportTestCases), "float", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRealFieldExtraction<double>(reportTestCases), "double", test_tag);
#if LONG_DOUBLE_SUPPORT
	nrOfFailedTestCases += ReportTestResult(VerifyRealFieldExtraction<long double>(reportTestCases), "long double", test_tag);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyRealFieldExtraction<float>(reportTestCases), "float", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRealFieldExtraction<double>(reportTestCases), "double", test_tag);
#if LONG_DOUBLE_SUPPORT
	nrOfFailedTestCases += ReportTestResult(VerifyRealFieldExtraction<long double>(reportTestCases), "long double", test_tag);
#endif

#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if	REGRESSION_LEVEL_4

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
