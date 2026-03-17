// logarithm.cpp: test suite for logarithm functions for qd_cascade floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal { namespace detail {

	template<typename TestType>
	void ReportLogError(const std::string& op, const TestType& v, const TestType& ref, const TestType& error) {
		std::cerr << op << " : " << v << " != " << ref << " : error : " << error << '\n';
	}

	// Verify log() against known constants and exact relationships
	// - log(1) == 0
	// - log(e) == 1 (using qdc_e constant)
	// - log(2) == qdc_ln2 (known constant)
	// - log(10) == qdc_ln10 (known constant)
	// - log(2^i) == i * qdc_ln2 (exact powers of 2)
	template<typename TestType>
	int VerifyLogFunction(bool reportTestCases, double maxError = 1.0e-62) {
		int nrOfFailedTestCases{ 0 };
		// log(1) == 0
		{
			TestType v = log(TestType(1.0));
			if (abs(v) > maxError) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportLogError("log(1)", v, TestType(0.0), abs(v));
			}
		}
		// log(e) == 1
		{
			TestType v = log(qdc_e);
			TestType error = abs(v - TestType(1.0));
			if (error > maxError) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportLogError("log(e)", v, TestType(1.0), error);
			}
		}
		// log(2) == qdc_ln2
		{
			TestType v = log(TestType(2.0));
			TestType error = abs(v - qdc_ln2);
			if (error > maxError) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportLogError("log(2)", v, qdc_ln2, error);
			}
		}
		// log(10) == qdc_ln10
		{
			TestType v = log(TestType(10.0));
			TestType error = abs(v - qdc_ln10);
			if (error > maxError) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportLogError("log(10)", v, qdc_ln10, error);
			}
		}
		// log(2^i) == i * qdc_ln2 for exact powers of 2
		for (int i = -52; i < 53; ++i) {
			if (i == 0) continue;
			TestType a = ldexp(TestType(1.0), i);
			TestType ref = qdc_ln2 * TestType(i);
			TestType v = log(a);
			TestType error = abs(v - ref);
			if (error > maxError) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportLogError("log(2^" + std::to_string(i) + ")", v, ref, error);
			}
		}
		return nrOfFailedTestCases;
	}

	// Verify log2() using log2(2^i) == i (exact)
	template<typename TestType>
	int VerifyLog2Function(bool reportTestCases, double maxError = 1.0e-62) {
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

	// Verify log10() using cross-check log10(a) == log(a) / qdc_ln10
	// and log10(10) == 1 (exact), log10(100) == 2, etc. for small powers
	template<typename TestType>
	int VerifyLog10Function(bool reportTestCases, double maxError = 1.0e-62) {
		int nrOfFailedTestCases{ 0 };
		// cross-check: log10(a) == log(a) / qdc_ln10 for powers of 2
		for (int i = -30; i < 31; ++i) {
			if (i == 0) continue;
			TestType a = ldexp(TestType(1.0), i);
			TestType v = log10(a);
			TestType ref = log(a) / qdc_ln10;
			TestType error = abs(v - ref);
			if (error > maxError) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportLogError("log10", v, ref, error);
			}
		}
		// log10(10) == 1 (exact via known constant)
		{
			TestType v = log10(TestType(10.0));
			TestType error = abs(v - TestType(1.0));
			if (error > maxError) {
				++nrOfFailedTestCases;
				if (reportTestCases) ReportLogError("log10(10)", v, TestType(1.0), error);
			}
		}
		return nrOfFailedTestCases;
	}

	// Verify log1p() using log1p(a) == log(1+a) cross-check
	template<typename TestType>
	int VerifyLog1pFunction(bool reportTestCases, double maxError = 1.0e-62) {
		int nrOfFailedTestCases{ 0 };
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

	std::string test_suite  = "qd_cascade mathlib logarithm function validation";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	using namespace detail;
	// qd_cascade log() is verified against known constants (qdc_ln2, qdc_ln10, qdc_e)
	// which are accurate to full 4-component precision. Threshold 1e-62 is near
	// the qd_cascade epsilon (~2^-208 ~= 1.5e-63).
	nrOfFailedTestCases += ReportTestResult(VerifyLogFunction<qd_cascade>(reportTestCases, 1.0e-62), "qd_cascade", "log()");
	nrOfFailedTestCases += ReportTestResult(VerifyLog2Function<qd_cascade>(reportTestCases, 1.0e-62), "qd_cascade", "log2()");
	nrOfFailedTestCases += ReportTestResult(VerifyLog10Function<qd_cascade>(reportTestCases, 1.0e-62), "qd_cascade", "log10()");
	nrOfFailedTestCases += ReportTestResult(VerifyLog1pFunction<qd_cascade>(reportTestCases, 1.0e-62), "qd_cascade", "log1p()");
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
