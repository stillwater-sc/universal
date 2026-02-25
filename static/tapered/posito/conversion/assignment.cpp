// assignment.cpp : test suite runner for native type literal assignments for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
//#define POSIT_FAST_SPECIALIZATION
#define POSIT_FAST_POSIT_8_2 1
#define POSIT_FAST_POSIT_16_2 1
#include <universal/number/posit1/posit1.hpp>
#include <universal/number/posito/posito.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw {
	namespace universal {

		template<typename PositType, typename Ty>
		int VerifyAssignment(bool reportTestCases) {
			constexpr unsigned nbits = PositType::nbits;
			const size_t NR_POSITS = (size_t(1) << nbits);
			int nrOfFailedTestCases = 0;

			// use only valid posit values
			// posit_raw -> to value in Ty -> assign to posit -> compare posits
			PositType p, assigned;
			for (size_t i = 0; i < NR_POSITS; i++) {
				p.setbits(i); // std::cout << p.get() << endl;
				if (p.isnar() && std::numeric_limits<Ty>::is_exact) continue; // can't assign NaR for integer types
				Ty value = (Ty)(p);
				assigned = value;
				if (p != assigned) {
					nrOfFailedTestCases++;
					if (reportTestCases) ReportAssignmentError("FAIL", "=", p, assigned, value);
				}
				else {
					//if (reportTestCases) ReportAssignmentSuccess("PASS", "=", p, assigned, value);
				}
			}
			return nrOfFailedTestCases;
		}

	}
} // namespace sw::universal

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
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

	std::string test_suite  = "posit assignment verification";
	std::string test_tag    = "assignment";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using Posit8_2 = posit<8, 2>;
	using Posit16_2 = posit<16, 2>;
	using Posito8_2 = posito<8, 2>;
	using Posito16_2 = posito<16, 2>;
	nrOfFailedTestCases += ReportTestResult(VerifyAssignment<Posit8_2, float>(reportTestCases), test_tag, "posit<8,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyAssignment<Posito8_2, float>(reportTestCases), test_tag, "posito<8,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyAssignment<Posit16_2, float>(reportTestCases), test_tag, "posit<16,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyAssignment<Posito16_2, float>(reportTestCases), test_tag, "posito<16,2>");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
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
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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



