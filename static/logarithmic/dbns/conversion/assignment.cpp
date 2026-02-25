// assignment.cpp: test suite runner for assignment conversion of floats to fixed-sized, arbitrary precision double-base logarithmic number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/native/ieee754.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/dbns/table.hpp>
#include <universal/verification/test_suite.hpp>


namespace sw { namespace universal {

	// TODO: needs a type trait to only match on dbns<> type
	template<typename DbnsType>
	int ValidateAssignment(bool reportTestCases) {
		constexpr size_t nbits = DbnsType::nbits;
		constexpr size_t NR_ENCODINGS = (1ull << nbits);
		int nrOfFailedTestCases = 0;

		DbnsType a, b;
		for (size_t i = 0; i < NR_ENCODINGS; ++i) {
			a.setbits(i);
			double da = double(a);
			b = da;
//			std::cout << to_binary(a) << " : " << da << " vs " << b << '\n';
			if (a != b) {
				if (a.isnan() && b.isnan()) continue;
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

	std::string test_suite = "dbns assignment validation";
	std::string test_tag = "assignment";
	bool reportTestCases = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	GenerateDbnsTable<5, 2>(std::cout);

	{
		// check the assignment when we are out of range
		dbns<5, 2> a;
		std::cout << range(a) << '\n';
		a = 50.0;
		ReportValue(a, "a");
		a = -50.0;
		ReportValue(a, "a");
		a = 0.01;
		ReportValue(a, "a");
		a = -0.01;
		ReportValue(a, "a");
	}

	//	using DBNS16_8 = dbns<16, 8, std::uint16_t>;
	//	using DBNS12_5 = dbns<12, 5, std::uint8_t>;
	using DBNS8_3 = dbns<8, 3, std::uint8_t>;
	using DBNS6_3 = dbns<6, 3, std::uint8_t>;
	//	using DBNS4_1 = dbns<4, 1, std::uint8_t>;

	// manual exhaustive test
//	nrOfFailedTestCases += ReportTestResult(ValidateAssignment<DBNS4_1>(reportTestCases), type_tag(DBNS4_1()), test_tag);
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment<DBNS6_3>(reportTestCases), type_tag<DBNS6_3>(), test_tag);
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment<DBNS8_3>(reportTestCases), type_tag(DBNS8_3()), test_tag);
//	nrOfFailedTestCases += ReportTestResult(ValidateAssignment<DBNS12_5>(reportTestCases), type_tag(DBNS12_5()), test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< dbns<4, 1, std::uint8_t> >(reportTestCases), type_tag(dbns<4, 1, std::uint8_t>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< dbns<4, 2, std::uint8_t> >(reportTestCases), type_tag(dbns<4, 2, std::uint8_t>()), test_tag);

	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< dbns<5, 2, std::uint8_t> >(reportTestCases), type_tag(dbns<5, 2, std::uint8_t>()), test_tag);

	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< dbns<6, 2, std::uint8_t> >(reportTestCases), type_tag(dbns<6, 2, std::uint8_t>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< dbns<6, 3, std::uint8_t> >(reportTestCases), type_tag(dbns<6, 3, std::uint8_t>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< dbns<6, 4, std::uint8_t> >(reportTestCases), type_tag(dbns<6, 4, std::uint8_t>()), test_tag);

	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< dbns<7, 3, std::uint8_t> >(reportTestCases), type_tag(dbns<7, 3, std::uint8_t>()), test_tag);

	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< dbns<8, 2, std::uint8_t> >(reportTestCases), type_tag(dbns<8, 2, std::uint8_t>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< dbns<8, 3, std::uint8_t> >(reportTestCases), type_tag(dbns<8, 3, std::uint8_t>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< dbns<8, 4, std::uint8_t> >(reportTestCases), type_tag(dbns<8, 4, std::uint8_t>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< dbns<8, 5, std::uint8_t> >(reportTestCases), type_tag(dbns<8, 5, std::uint8_t>()), test_tag);
	nrOfFailedTestCases += ReportTestResult(ValidateAssignment< dbns<8, 6, std::uint8_t> >(reportTestCases), type_tag(dbns<8, 6, std::uint8_t>()), test_tag);
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
