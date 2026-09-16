// assignment.cpp: test suite runner for assignment conversion of floats to fixed-sized, arbitrary configuration rationals
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/rational/rational.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/rational_test_suite.hpp>

template<typename TargetFloat>
void GenerateBitWeightTable() {
	using namespace sw::universal;
//	constexpr size_t minNormalExponent = static_cast<size_t>(-ieee754_parameter<TargetFloat > ::minNormalExp);
	constexpr size_t minSubnormalExponent = static_cast<size_t>(-ieee754_parameter<TargetFloat>::minSubnormalExp);

	TargetFloat multiplier = ieee754_parameter<TargetFloat>::minSubnormal;
	for (size_t i = 0; i < minSubnormalExponent; ++i) {
		std::cout << i << ' ' << to_binary(multiplier) << ' ' << multiplier << '\n';
		multiplier *= 2.0f; // these are error free multiplies
	}
}

template<typename Real>
void Ranges(Real v) {
	using namespace sw::universal;
	using rb10 = rational<10, base2, std::uint16_t>;
	using rb12 = rational<12, base2, std::uint16_t>;
	using rb14 = rational<14, base2, std::uint16_t>;
	using rb20 = rational<20, base2, std::uint32_t>;
	using rb24 = rational<24, base2, std::uint32_t>;

	rb8 r8{ v };
	rb10 r10{ v };
	rb12 r12{ v };
	rb14 r14{ v };
	rb16 r16{ v };
	rb20 r20{ v };
	rb24 r24{ v };

	std::cout << symmetry_range(r8)  << '\n' << to_binary(r8)  << " : " << r8  << '\n';
	std::cout << symmetry_range(r10) << '\n' << to_binary(r10) << " : " << r10 << '\n';
	std::cout << symmetry_range(r12) << '\n' << to_binary(r12) << " : " << r12 << '\n';
	std::cout << symmetry_range(r14) << '\n' << to_binary(r14) << " : " << r14 << '\n';
	std::cout << symmetry_range(r16) << '\n' << to_binary(r16) << " : " << r16 << '\n';
	std::cout << symmetry_range(r20) << '\n' << to_binary(r20) << " : " << r20 << '\n';
	std::cout << symmetry_range(r24) << '\n' << to_binary(r24) << " : " << r24 << '\n';
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

	std::string test_suite  = "binary rational float assignment validation";
	std::string test_tag    = "binary rational assignment";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	rb16 a,b;
	a.set(0x02, 0x0A);
	std::cout << to_binary(a) << '\n';
	double da = double(a);
	b = da;
	std::cout << to_binary(da) << " : " << da << '\n';
	std::cout << to_binary(a) << " : " << a << '\n';
	std::cout << to_binary(b) << " : " << b << '\n';

	Ranges(1.0f);

	// manual exhaustive test
	
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment<rb8>(reportTestCases), type_tag(rb8()), test_tag);
	// rb16 is an exhaustive 2^32 pair sweep: minutes now that the conversions come back exact and
	// the loop no longer returns after its first 10 failures (#1523). rb12 is 2^24 pairs.
	using rb12 = rational<12, base2, std::uint16_t>;
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment<rb12>(reportTestCases), type_tag(rb12()), test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< rational<4, base2, std::uint8_t> >(reportTestCases), type_tag(rational<4, base2, std::uint8_t>()), test_tag);

	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< rational<8, base2, std::uint8_t> >(reportTestCases), type_tag(rational<8, base2, std::uint8_t>()), test_tag);

	// set(n, d) must hold the value it was given, for every encoding with a denominator, the signed
	// minimum -2^(nbits-1) included (#1525)
	nrOfFailedTestCases += ReportTestResult(ValidateNormalization< rational<4, base2, std::uint8_t> >(reportTestCases), type_tag(rational<4, base2, std::uint8_t>()), "normalization");

	nrOfFailedTestCases += ReportTestResult(ValidateNormalization< rational<8, base2, std::uint8_t> >(reportTestCases), type_tag(rational<8, base2, std::uint8_t>()), "normalization");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(ValidateNormalization< rational<10, base2, std::uint16_t> >(reportTestCases), type_tag(rational<10, base2, std::uint16_t>()), "normalization");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< rational<12, base2, std::uint16_t> >(reportTestCases), type_tag(rational<12, base2, std::uint16_t>()), test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(ValidateNormalization< rational<12, base2, std::uint16_t> >(reportTestCases), type_tag(rational<12, base2, std::uint16_t>()), "normalization");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
