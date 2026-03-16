// exponent.cpp: test suite runner for exponentiation function for quad-double (qd) floats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {
	namespace detail {

		template<typename TestType>
		void ReportExpFunctionError(const std::string& op, const TestType& v, const TestType& ref, const TestType& error) {
			std::cerr << op << " : " << v << " != " << ref << " : error : " << error << '\n';
		}

		// Verify exp() using the identity exp(ln(a)) == a
		// Construct a = 2^i (exact), compute exp(log(a)), compare against a.
		// Also verify exp(0) == 1 and exp(1) == qd_e.
		template<typename TestType>
		int VerifyExpFunction(bool reportTestCases, double maxError = 1.0e-60) {
			int nrOfFailedTestCases{ 0 };
			// exp(0) == 1
			{
				TestType v = exp(TestType(0.0));
				TestType error = abs(v - TestType(1.0));
				if (error > maxError) {
					++nrOfFailedTestCases;
					if (reportTestCases) ReportExpFunctionError("exp(0)", v, TestType(1.0), error);
				}
			}
			// exp(1) == e (use qd_e constant as reference)
			{
				TestType v = exp(TestType(1.0));
				TestType error = abs(v - qd_e);
				if (error > maxError) {
					++nrOfFailedTestCases;
					if (reportTestCases) ReportExpFunctionError("exp(1)", v, qd_e, error);
				}
			}
			// round-trip: exp(log(2^i)) == 2^i for integer powers of 2
			for (int i = -30; i < 31; ++i) {
				TestType a = ldexp(TestType(1.0), i);  // exact 2^i
				TestType v = exp(log(a));
				TestType error = abs(v - a);
				if (error > maxError * abs(a)) {
					++nrOfFailedTestCases;
					if (reportTestCases) ReportExpFunctionError("exp(log(2^" + std::to_string(i) + "))", v, a, error);
				}
			}
			return nrOfFailedTestCases;
		}

		// Verify exp2() using the identity exp2(i) == 2^i (exact for integers)
		template<typename TestType>
		int VerifyExp2Function(bool reportTestCases, double maxError = 1.0e-60) {
			int nrOfFailedTestCases{ 0 };
			for (int i = -30; i < 31; ++i) {
				TestType ref = ldexp(TestType(1.0), i);  // exact 2^i
				TestType v = exp2(TestType(i));
				TestType error = abs(v - ref);
				if (error > maxError * abs(ref)) {
					++nrOfFailedTestCases;
					if (reportTestCases) ReportExpFunctionError("exp2(" + std::to_string(i) + ")", v, ref, error);
				}
			}
			return nrOfFailedTestCases;
		}

		// Verify exp10() using the identity exp10(i) == 10^i
		// Construct 10^i via repeated multiplication at full TestType precision
		template<typename TestType>
		int VerifyExp10Function(bool reportTestCases, double maxError = 1.0e-60) {
			int nrOfFailedTestCases{ 0 };
			for (int i = -15; i < 16; ++i) {
				TestType ref(1.0);
				if (i > 0) { for (int k = 0; k < i; ++k) ref *= TestType(10.0); }
				else if (i < 0) { for (int k = 0; k < -i; ++k) ref /= TestType(10.0); }
				TestType v = exp10(TestType(i));
				TestType error = abs(v - ref);
				if (error > maxError * abs(ref)) {
					++nrOfFailedTestCases;
					if (reportTestCases) ReportExpFunctionError("exp10(" + std::to_string(i) + ")", v, ref, error);
				}
			}
			return nrOfFailedTestCases;
		}

		// Verify expm1() using the identity expm1(x) + 1 == exp(x)
		template<typename TestType>
		int VerifyExpm1Function(bool reportTestCases, double maxError = 1.0e-60) {
			int nrOfFailedTestCases{ 0 };
			// expm1(0) == 0
			{
				TestType v = expm1(TestType(0.0));
				if (abs(v) > maxError) {
					++nrOfFailedTestCases;
					if (reportTestCases) ReportExpFunctionError("expm1(0)", v, TestType(0.0), abs(v));
				}
			}
			// cross-check: expm1(x) + 1 == exp(x) for various x
			for (int i = -20; i < 21; ++i) {
				TestType x = TestType(i) * TestType(0.1);
				TestType v1 = expm1(x) + TestType(1.0);
				TestType v2 = exp(x);
				TestType error = abs(v1 - v2);
				if (error > maxError * abs(v2)) {
					++nrOfFailedTestCases;
					if (reportTestCases) ReportExpFunctionError("expm1+1 vs exp", v1, v2, error);
				}
			}
			return nrOfFailedTestCases;
		}

	}
}}

// generate specific test case
template<typename Ty>
void GenerateTestCase(Ty fa) {
	unsigned precision = 25;
	unsigned width = 30;
	Ty fref;
	sw::universal::qd a, ref, v;
	a = fa;
	fref = std::exp(fa);
	ref = fref;
	v = sw::universal::exp(a);
	auto oldPrec = std::cout.precision();
	std::cout << std::setprecision(precision);
	std::cout << " -> exp(" << fa << ") = " << std::setw(width) << fref << std::endl;
	std::cout << " -> exp( " << a << ")  = " << v << '\n' << to_binary(v) << '\n';
	std::cout << to_binary(ref) << "\n -> reference\n";
	std::cout << (ref == v ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(oldPrec);
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

	std::string test_suite  = "quad-double mathlib exponentiation function validation";
	std::string test_tag    = "exp/exp2/exp10/expm1";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateTestCase(4.0);

	auto oldPrec = std::cout.precision();
	for (int i = 0; i < 30; ++i) {
		std::string tag = "exp(" + std::to_string(i) + ")";
		double exponentRef = std::exp(double(i));
		qd exponent = exp(qd(i));
		qd error = exponentRef - exponent;
		std::cout << std::setw(20) << tag << " : " << std::setprecision(32) << exponentRef << " : " << exponent << " : " << std::setw(25) << error << '\n';
	}

	for (int i = 0; i < 30; ++i) {
		std::string tag = "exp2(" + std::to_string(i) + ")";
		double exponentRef = std::exp2(double(i));
		qd exponent = exp2(qd(i));
		qd error = exponentRef - exponent;
		std::cout << std::setw(20) << tag << " : " << std::setprecision(32) << exponentRef << " : " << exponent << " : " << std::setw(25) << error << '\n';
	}

	for (int i = 0; i < 30; ++i) {
		std::string tag = "exp10(" + std::to_string(i) + ")";
		double exponentRef = std::pow(10.0, double(i));
		qd exponent = exp10(qd(i));
		qd error = exponentRef - exponent;
		std::cout << std::setw(20) << tag << " : " << std::setprecision(32) << exponentRef << " : " << exponent << " : " << std::setw(25) << error << '\n';
	}

	for (int i = 0; i < 30; ++i) {
		std::string tag = "expm1(" + std::to_string(i) + ")";
		double exponentRef = std::expm1(double(i));
		qd exponent = expm1(qd(i));
		qd error = exponentRef - exponent;
		std::cout << std::setw(20) << tag << " : " << std::setprecision(32) << exponentRef << " : " << exponent << " : " << std::setw(25) << error << '\n';
	}

	std::cout << std::setprecision(oldPrec);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

	using namespace detail;

	// References are computed at full qd precision using mathematical identities
	// (exp(log(2^i))==2^i, exp2(i)==2^i, exp10(i)==10^i, expm1(x)+1==exp(x))
	// -- no double-precision oracle needed.
	// Threshold is near qd epsilon (~1.5e-63 = 2^-209).
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExpFunction<qd>(reportTestCases, 1.0e-60), "quad-double", "exp()");
	nrOfFailedTestCases += ReportTestResult(VerifyExp2Function<qd>(reportTestCases, 1.0e-60), "quad-double", "exp2()");
	nrOfFailedTestCases += ReportTestResult(VerifyExp10Function<qd>(reportTestCases, 1.0e-60), "quad-double", "exp10()");
	nrOfFailedTestCases += ReportTestResult(VerifyExpm1Function<qd>(reportTestCases, 1.0e-60), "quad-double", "expm1()");
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
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
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
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
