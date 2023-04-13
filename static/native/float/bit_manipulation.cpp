//  bit_manipulation.cpp: experiments with the C++20 <bit> library
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp> // set up the compiler environment to support long doubles
#include <universal/utility/bit_cast.hpp>    // set up the compiler environment to support C++20 <bit> library
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <universal/native/ieee754.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	int VerifyFloatFieldExtraction(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTests = 0;

		bool sign{ false };
		std::uint64_t rawExponent{ 0 };
		int exponent{ 0 };
		std::uint64_t rawFraction{ 0 };
		int fbits{ 64 };

		Real a;
		a = 1.0;

		if constexpr (sizeof(Real) == 4) {
			fbits = 23;
		}
		else if constexpr (sizeof(Real) == 8) {
			fbits = 52;
		}
		else if constexpr (sizeof(Real) == 16) {
			fbits = 64;
		}
		extractFields(a, sign, rawExponent, rawFraction);
		exponent = static_cast<int>(rawExponent) - ieee754_parameter<Real>::bias;
		if (sign != false || exponent != 0 || rawFraction != 0) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "fp components: " << (sign ? '1' : '0') << " exp: " << exponent << " frac: " << to_binary(rawFraction, fbits, true) << '\n';
		}

		return nrOfFailedTests;
	}

} } // namespace sw::universal

union float_decoder {
	float_decoder() : f{ 0.0f } {}
	float_decoder(float _f) : f{ _f } {}
	float f;
	struct {
		uint32_t fraction : 23;
		uint32_t exponent : 8;
		uint32_t sign : 1;
	} parts;
};

template<typename Integer,
	typename = typename std::enable_if< std::is_integral<Integer>::value, Integer >::type
>
inline std::string to_binary(const Integer& number, int nbits = 0, bool bNibbleMarker = true) {
	std::stringstream s;
	if (nbits == 0) nbits = 8 * sizeof(number);
	s << 'b';
	uint64_t mask = (uint64_t(1) << (nbits - 1));
	for (int i = int(nbits) - 1; i >= 0; --i) {
		s << ((number & mask) ? '1' : '0');
		if (bNibbleMarker && i > 0 && i % 4 == 0) s << '\'';
		mask >>= 1;
	}
	return s.str();
}

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
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// create a float with the following layout
	// b1.00001111.00011001011010001001001"
	float_decoder decoder;
	decoder.parts.fraction = 0b00011001011010001001001;
	decoder.parts.exponent = 0b0000'0001;
	decoder.parts.sign = 0b1;

	std::cout << decoder.f << '\n';
#ifdef BIT_CAST_SUPPORT
	uint32_t bc = std::bit_cast<uint32_t, float>(decoder.f);
	std::cout << to_binary(bc, 32) << '\n';
#endif
	
	nrOfFailedTestCases += ReportTestResult(VerifyFloatFieldExtraction<float>(reportTestCases), "float", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyFloatFieldExtraction<float>(reportTestCases), "float", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFloatFieldExtraction<double>(reportTestCases), "double", test_tag);
#if LONG_DOUBLE_SUPPORT
	nrOfFailedTestCases += ReportTestResult(VerifyFloatFieldExtraction<long double>(reportTestCases), "long double", test_tag);
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
