// assignment.cpp: test suite runner for assignment conversion of floats to fixed-sized, arbitrary precision logarithmic number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/verification/test_suite.hpp>


namespace sw { namespace universal {

	// TODO: needs a type trait to only match on lns<> type
	template<typename LnsType, std::enable_if_t<is_lns<LnsType>, bool> = true>
	int ValidateAssignment(bool reportTestCases) {
		constexpr size_t nbits = LnsType::nbits;
		constexpr size_t NR_ENCODINGS = (1ull << nbits);
		int nrOfFailedTestCases = 0;

		LnsType a, b;
		for (size_t i = 0; i < NR_ENCODINGS; ++i) {
			a.setbits(i);
			double da = double(a);
			b = da;
	//		std::cout << to_binary(a) << " : " << da << " vs " << b << '\n';
			if (a != b) {
				if (isnan(a) && b.isnan()) continue;
				++nrOfFailedTestCases;
				if (reportTestCases) ReportAssignmentError("FAIL", "=", da, b, a);
			}
			else {
				// if (reportTestCases) ReportAssignmentSuccess("PASS", "=", da, b, a);
			}
		}

		// test clipping or saturation

		return nrOfFailedTestCases;
	}

} }

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
void SampleTest(Real v) {
	using namespace sw::universal;
	std::cout << symmetry_range(lns<8, 0>()) << '\n' << to_binary(lns<8, 0>(v)) << " : " << lns<8, 0>(v) << '\n';
	std::cout << symmetry_range(lns<8, 1>()) << '\n' << to_binary(lns<8, 1>(v)) << " : " << lns<8, 1>(v) << '\n';
	std::cout << symmetry_range(lns<8, 2>()) << '\n' << to_binary(lns<8, 2>(v)) << " : " << lns<8, 2>(v) << '\n';
	std::cout << symmetry_range(lns<8, 3>()) << '\n' << to_binary(lns<8, 3>(v)) << " : " << lns<8, 3>(v) << '\n';
	std::cout << symmetry_range(lns<8, 4>()) << '\n' << to_binary(lns<8, 4>(v)) << " : " << lns<8, 4>(v) << '\n';
	std::cout << symmetry_range(lns<8, 5>()) << '\n' << to_binary(lns<8, 5>(v)) << " : " << lns<8, 5>(v) << '\n';
	std::cout << symmetry_range(lns<8, 6>()) << '\n' << to_binary(lns<8, 6>(v)) << " : " << lns<8, 6>(v) << '\n';
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

	std::string test_suite  = "lns assignment validation";
	std::string test_tag    = "assignment";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

//	using LNS16_5 = lns<16, 5, std::uint16_t>;
//	using LNS11_5 = lns<11, 5, std::uint8_t>;
//	using LNS8_2 = lns<8, 2, std::uint8_t>;
	using LNS5_2 = lns<5, 2, std::uint8_t>;
//	using LNS4_1 = lns<4, 1, std::uint8_t>;

	// GenerateBitWeightTable<double>();
	SampleTest(1024.0f);

	// manual exhaustive test
//	nrOfFailedTestCases += ReportTestResult(ValidateAssignment<LNS4_1>(reportTestCases), type_tag(LNS4_1()), test_tag);
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment<LNS5_2>(reportTestCases), type_tag(LNS5_2()), test_tag);
//	nrOfFailedTestCases += ReportTestResult(ValidateAssignment<LNS8_2>(reportTestCases), type_tag(LNS8_2()), test_tag);
//	nrOfFailedTestCases += ReportTestResult(ValidateAssignment<LNS11_5>(reportTestCases), type_tag(LNS11_5()), test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< lns<4, 0, std::uint8_t> >(reportTestCases), type_tag(lns<4, 0, std::uint8_t>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< lns<4, 1, std::uint8_t> >(reportTestCases), type_tag(lns<4, 1, std::uint8_t>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< lns<4, 2, std::uint8_t> >(reportTestCases), type_tag(lns<4, 2, std::uint8_t>()), test_tag);

	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< lns<8, 0, std::uint8_t> >(reportTestCases), type_tag(lns<8, 0, std::uint8_t>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< lns<8, 2, std::uint8_t> >(reportTestCases), type_tag(lns<8, 2, std::uint8_t>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< lns<8, 4, std::uint8_t> >(reportTestCases), type_tag(lns<8, 4, std::uint8_t>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< lns<8, 6, std::uint8_t> >(reportTestCases), type_tag(lns<8, 6, std::uint8_t>()), test_tag);
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
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
