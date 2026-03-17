// logarithm.cpp: test suite for logarithm functions for dd_cascade floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal { namespace detail {

	template<typename TestType>
	void ReportLogError(const std::string& op, const TestType& v, const TestType& ref, const TestType& error) {
		std::cerr << op << " : " << v << " != " << ref << " : error : " << error << '\n';
	}

	// Verify log() using independent constant checks first, then round-trip.
	template<typename TestType>
	int VerifyLogFunction(bool reportTestCases, double maxError = 1.0e-28) {
		int nrOfFailedTestCases{ 0 };
		// independent: log(1) == 0
		{
			TestType v = log(TestType(1.0));
			if (abs(v) > maxError) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportLogError("log(1)", v, TestType(0.0), abs(v));
			}
		}
		// independent: log(e) == 1
		{
			TestType v = log(ddc_e);
			TestType error = abs(v - TestType(1.0));
			if (error > maxError) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportLogError("log(e)", v, TestType(1.0), error);
			}
		}
		// independent: log(2) == ddc_ln2
		{
			TestType v = log(TestType(2.0));
			TestType error = abs(v - ddc_ln2);
			if (error > maxError) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportLogError("log(2)", v, ddc_ln2, error);
			}
		}
		// independent: log(2^i) == i * ddc_ln2 for exact powers of 2
		for (int i = -52; i < 53; ++i) {
			if (i == 0) continue;
			TestType a = ldexp(TestType(1.0), i);
			TestType ref = ddc_ln2 * TestType(i);
			TestType v = log(a);
			TestType error = abs(v - ref);
			if (error > maxError) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportLogError("log(2^" + std::to_string(i) + ")", v, ref, error);
			}
		}
		// secondary: round-trip log(exp(i)) == i (couples exp and log)
		for (int i = -64; i < 65; ++i) {
			if (i == 0) continue;
			TestType ref(i);
			TestType a = exp(ref);
			TestType v = log(a);
			TestType error = abs(v - ref);
			if (error > maxError) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportLogError("log(exp(i))", v, ref, error);
			}
		}
		return nrOfFailedTestCases;
	}

	// Verify log2() using log2(2^i) == i
	template<typename TestType>
	int VerifyLog2Function(bool reportTestCases, double maxError = 1.0e-28) {
		int nrOfFailedTestCases{ 0 };
		for (int i = -52; i < 53; ++i) {
			TestType a = ldexp(TestType(1.0), i);
			TestType ref(i);
			TestType v = log2(a);
			TestType error = abs(v - ref);
			if (error > maxError) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportLogError("log2", v, ref, error);
			}
		}
		return nrOfFailedTestCases;
	}

	// Verify log10() using log10(10^i) == i
	template<typename TestType>
	int VerifyLog10Function(bool reportTestCases, double maxError = 1.0e-28) {
		int nrOfFailedTestCases{ 0 };
		for (int i = -15; i < 16; ++i) {
			TestType a(1.0);
			if (i > 0) { for (int k = 0; k < i; ++k) a *= TestType(10.0); }
			else if (i < 0) { for (int k = 0; k < -i; ++k) a /= TestType(10.0); }
			TestType ref(i);
			TestType v = log10(a);
			TestType error = abs(v - ref);
			if (error > maxError) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportLogError("log10", v, ref, error);
			}
		}
		return nrOfFailedTestCases;
	}

	// Verify log1p() using log1p(a) == log(1+a)
	template<typename TestType>
	int VerifyLog1pFunction(bool reportTestCases, double maxError = 1.0e-28) {
		int nrOfFailedTestCases{ 0 };
		// small values near zero: log1p(x) must maintain precision
		for (int i = 1; i < 30; ++i) {
			TestType x = ldexp(TestType(1.0), -i);  // x = 2^-i (small)
			TestType v1 = log1p(x);
			TestType v2 = log(TestType(1.0) + x);
			TestType error = abs(v1 - v2);
			if (error > maxError) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportLogError("log1p(small)", v1, v2, error);
			}
		}
		// larger values: cross-check log1p(a) == log(1+a)
		for (int i = 1; i < 20; ++i) {
			TestType a(i);
			TestType v1 = log1p(a);
			TestType v2 = log(TestType(1.0) + a);
			TestType error = abs(v1 - v2);
			if (error > maxError) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportLogError("log1p", v1, v2, error);
			}
		}
		return nrOfFailedTestCases;
	}

}}}

#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#define REGRESSION_LEVEL_1 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "dd_cascade mathlib logarithm function validation";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	using namespace detail;
	// dd_cascade has ~104 bits precision -> threshold ~1e-28
	nrOfFailedTestCases += ReportTestResult(VerifyLogFunction<dd_cascade>(reportTestCases, 1.0e-28), "dd_cascade", "log()");
	nrOfFailedTestCases += ReportTestResult(VerifyLog2Function<dd_cascade>(reportTestCases, 1.0e-28), "dd_cascade", "log2()");
	nrOfFailedTestCases += ReportTestResult(VerifyLog10Function<dd_cascade>(reportTestCases, 1.0e-28), "dd_cascade", "log10()");
	nrOfFailedTestCases += ReportTestResult(VerifyLog1pFunction<dd_cascade>(reportTestCases, 1.0e-28), "dd_cascade", "log1p()");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
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
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
