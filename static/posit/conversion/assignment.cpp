// assignment.cpp : test suite runner for native type literal assignments for posits
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
//#define ALGORITHM_VERBOSE_OUTPUT
//#define ALGORITHM_TRACE_CONVERSION
//#define VALUE_TRACE_CONVERSION
#include <universal/number/posit/posit.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_suite_mathlib.hpp>

namespace sw { namespace universal {

#define FULL_ENUMERATION 1

	template<size_t nbits, size_t es, typename Ty>
	int VerifyAssignment(bool reportTestCases) {
		const size_t NR_POSITS = (size_t(1) << nbits);
		int nrOfFailedTestCases = 0;

		// use only valid posit values
		// posit_raw -> to value in Ty -> assign to posit -> compare posits
		sw::universal::posit<nbits, es> p, assigned;
		for (size_t i = 0; i < NR_POSITS; i++) {
			p.setbits(i); // std::cout << p.get() << endl;
			if (p.isnar() && std::numeric_limits<Ty>::is_exact) continue; // can't assign NaR for integer types
			Ty value = (Ty)(p);
			assigned = value;
#if FULL_ENUMERATION
			if (p != assigned) {
                std::cout << "FAIL : " << value << '\n';
			    std::cout << "  : " << to_binary(p) << " : " << p << " -> " << assigned << '\n';
			    std::cout << "  : " << to_binary(assigned) << " : " << assigned << '\n';; 
            }
            else {
                std::cout << "PASS : " << value << '\n';
			    std::cout << "  : " << to_binary(p) << " : " << p << " -> " << assigned << '\n';
			    std::cout << "  : " << to_binary(assigned) << " : " << assigned << '\n';; 
            }
#else
			if (p != assigned) {
				nrOfFailedTestCases++;
				if (reportTestCases) ReportAssignmentError("FAIL", "=", p, assigned, value);
			}
			else {
				//if (reportTestCases) ReportAssignmentSuccess("PASS", "=", p, assigned, value);
			}
#endif
		}
		return nrOfFailedTestCases;
	}

} } // namespace sw::universal

template<size_t nbits, size_t es, typename Ty>
Ty GenerateValue(const sw::universal::posit<nbits, es>& p) {
	Ty value = 0;
	if (std::numeric_limits<Ty>::is_exact) {
		if (std::numeric_limits<Ty>::is_signed) {
			value = (long long)(p);
		}
		else {
			value = (unsigned long long)(p);
		}
	}
	else {
		value = (long double)(p);
	}
	return value;
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

	std::string test_suite  = "posit assignment validation";
	std::string test_tag    = "assignment";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

    float f = 0.125f;
    posit<5,1> p{};
    p = f;
    std::cout << to_binary(p) << " : " << p << " -> " << f << '\n';

    value<23> v;
    v = f;
    std::cout << to_triple(v) << " : " << v << '\n';

	//nrOfFailedTestCases = ReportTestResult(VerifyAssignment<5, 1, float>(reportTestCases), test_tag, "posit<5,1>");
	//nrOfFailedTestCases = ReportTestResult(VerifyAssignment<6, 2, float>(reportTestCases), test_tag, "posit<6,2>");

    ++nrOfFailedTestCases;
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<3, 0, float>(reportTestCases), test_tag, "posit<3,0>");

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<4, 0, float>(reportTestCases), test_tag, "posit<4,0>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<4, 1, float>(reportTestCases), test_tag, "posit<4,1>");

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<5, 0, float>(reportTestCases), test_tag, "posit<5,0>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<5, 1, float>(reportTestCases), test_tag, "posit<5,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<5, 2, float>(reportTestCases), test_tag, "posit<5,2>");

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<6, 0, float>(reportTestCases), test_tag, "posit<6,0>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<6, 1, float>(reportTestCases), test_tag, "posit<6,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<6, 2, float>(reportTestCases), test_tag, "posit<6,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<6, 3, float>(reportTestCases), test_tag, "posit<6,3>");

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<7, 0, float>(reportTestCases), test_tag, "posit<7,0>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<7, 1, float>(reportTestCases), test_tag, "posit<7,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<7, 2, float>(reportTestCases), test_tag, "posit<7,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<7, 3, float>(reportTestCases), test_tag, "posit<7,3>");

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<8, 0, float>(reportTestCases), test_tag, "posit<8,0>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<8, 1, float>(reportTestCases), test_tag, "posit<8,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<8, 2, float>(reportTestCases), test_tag, "posit<8,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<8, 3, float>(reportTestCases), test_tag, "posit<8,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<8, 4, float>(reportTestCases), test_tag, "posit<8,4>");

	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<9, 0, float>(reportTestCases), test_tag, "posit<9,0>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<9, 1, float>(reportTestCases), test_tag, "posit<9,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<9, 2, float>(reportTestCases), test_tag, "posit<9,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<9, 3, float>(reportTestCases), test_tag, "posit<9,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyAssignment<9, 4, float>(reportTestCases), test_tag, "posit<9,4>");
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



