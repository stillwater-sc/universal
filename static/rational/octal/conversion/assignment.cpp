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
	// every one of these is a base8 rational, so their digit counts are ndigits, not nbits:
	// rb8 and rb16 are the library's base2 aliases and do not belong in this file (#1526)
	using ro10 = rational<10, base8, std::uint16_t>;
	using ro12 = rational<12, base8, std::uint16_t>;
	using ro14 = rational<14, base8, std::uint16_t>;
	using ro20 = rational<20, base8, std::uint32_t>;
	using ro24 = rational<24, base8, std::uint32_t>;

	ro8  r8{ v };
	ro10 r10{ v };
	ro12 r12{ v };
	ro14 r14{ v };
	ro16 r16{ v };
	ro20 r20{ v };
	ro24 r24{ v };

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

	std::string test_suite  = "octal rational float assignment validation";
	std::string test_tag    = "octal rational assignment";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ro16 a,b;
	a.set(0x02, 0x0A);
	std::cout << to_binary(a) << '\n';
	double da = double(a);
	b = da;
	std::cout << to_binary(da) << " : " << da << '\n';
	std::cout << to_binary(a) << " : " << a << '\n';
	std::cout << to_binary(b) << " : " << b << '\n';

	Ranges(1.0f);

	// manual exhaustive test
	
	// ValidateAssignment enumerates the raw component encodings of whatever base it is given, and a
	// base8 component holds 8^ndigits of them, so the digit count is what keeps the sweep feasible:
	// 3 digits is 512 encodings per component, 4 digits is 4096 and about 10 minutes (#1526).
	using Sweep = rational<3, base8, std::uint8_t>;
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment<Sweep>(reportTestCases), type_tag(Sweep()), test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< rational<1, base8, std::uint8_t> >(reportTestCases), type_tag(rational<1, base8, std::uint8_t>()), test_tag);

	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< rational<2, base8, std::uint8_t> >(reportTestCases), type_tag(rational<2, base8, std::uint8_t>()), test_tag);
#endif

#if REGRESSION_LEVEL_2
	// 512 encodings per component
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< rational<3, base8, std::uint8_t> >(reportTestCases), type_tag(rational<3, base8, std::uint8_t>()), test_tag);
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
	// 4096 encodings per component: about 16.7M pairs
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< rational<4, base8, std::uint8_t> >(reportTestCases), type_tag(rational<4, base8, std::uint8_t>()), test_tag);
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
