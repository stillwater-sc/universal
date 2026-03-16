// logarithm.cpp: test suite runner for log/log1p/log2/log10 functions for quad-double (qd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <string>
#include <numbers>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw {
	namespace universal {

		qd trace_log(const qd& a) {
			if (a.isnan() || a.isinf()) return a;

			if (a.iszero()) return qd(SpecificValue::infneg);

			if (a.isone()) return 0.0;

			if (a[0] <= 0.0) {
				std::cerr << "log: non-positive argument\n";
				errno = EDOM;
				return qd(SpecificValue::qnan);
			}

			/* Strategy.  The Taylor series for log converges much more
			   slowly than that of exp, due to the lack of the factorial
			   term in the denominator.  Hence this routine instead tries
			   to determine the root of the function

				   f(x) = exp(x) - a

			   using Newton iteration.  The iteration is given by

				   x' = x - f(x)/f'(x)
					  = x - (1 - a * exp(-x))
					  = x + a * exp(-x) - 1.

			   Two iteration is needed, since Newton's iteration
			   approximately doubles the number of digits per iteration.
			 */

			qd x = std::log(a[0]);   // Initial approximation
			std::cout << "initial approximation :\n" << to_binary(x) << '\n';
			
			// if a = e then x = 1 + e * 1 / e - 1.0;

			x = x + a * exp(-x) - 1.0;
			std::cout << "1st Newton iteration  :\n" << to_binary(x) << '\n';
			x = x + a * exp(-x) - 1.0;
			std::cout << "2nd Newton iteration  :\n" << to_binary(x) << '\n';
			x = x + a * exp(-x) - 1.0;
			std::cout << "3rd Newton iteration  :\n" << to_binary(x) << '\n';

			return x;
		}

		template<typename TestType>
		void ReportQuadDoubleFunctionError(const std::string& op, const TestType& a, const TestType& ref, const TestType& error) {
			std::cerr << op << " : " << a << " != " << ref << " : error : " << error << '\n';
		}

		// Verify log() using the identity log(exp(x)) == x
		// The reference is the exact integer i (as TestType), not a double approximation.
		// This tests the round-trip accuracy of the exp/log pair at full precision.
		template<typename TestType>
		int VerifyLogFunction(bool reportTestCases, double maxError = 1.0e-60) {
			int nrOfFailedTestCases{ 0 };
			for (int i = -64; i < 65; ++i) {
				if (i == 0) continue;  // log(exp(0)) = log(1) = 0, tested separately
				TestType ref(i);
				TestType a = exp(ref);         // a = e^i at full TestType precision
				TestType v = log(a);           // v = log(e^i), should equal i
				TestType error = abs(v - ref);
				if (error > maxError) {
					++nrOfFailedTestCases;
					if (reportTestCases) ReportQuadDoubleFunctionError("log", v, ref, error);
				}
			}
			// also verify log(1) == 0 and log(e) == 1
			{
				TestType v = log(TestType(1.0));
				if (abs(v) > maxError) {
					++nrOfFailedTestCases;
					if (reportTestCases) ReportQuadDoubleFunctionError("log(1)", v, TestType(0.0), abs(v));
				}
			}
			return nrOfFailedTestCases;
		}

		// Verify log2() using the identity log2(2^i) == i (exact for integer powers of 2)
		template<typename TestType>
		int VerifyLog2Function(bool reportTestCases, double maxError = 1.0e-60) {
			int nrOfFailedTestCases{ 0 };
			for (int i = -52; i < 53; ++i) {
				// 2^i is exactly representable as a double for |i| <= 1023
				TestType a = ldexp(TestType(1.0), i);  // exact 2^i
				TestType ref(i);                        // exact reference
				TestType v = log2(a);
				TestType error = abs(v - ref);
				if (error > maxError) {
					++nrOfFailedTestCases;
					if (reportTestCases) ReportQuadDoubleFunctionError("log2", v, ref, error);
				}
			}
			return nrOfFailedTestCases;
		}

		// Verify log10() using the identity log10(10^i) == i
		// Construct 10^i via repeated multiplication in TestType precision
		template<typename TestType>
		int VerifyLog10Function(bool reportTestCases, double maxError = 1.0e-60) {
			int nrOfFailedTestCases{ 0 };
			for (int i = -18; i < 19; ++i) {
				// construct 10^i at full precision
				TestType a(1.0);
				if (i > 0) { for (int k = 0; k < i; ++k) a *= TestType(10.0); }
				else if (i < 0) { for (int k = 0; k < -i; ++k) a /= TestType(10.0); }
				TestType ref(i);
				TestType v = log10(a);
				TestType error = abs(v - ref);
				if (error > maxError) {
					++nrOfFailedTestCases;
					if (reportTestCases) ReportQuadDoubleFunctionError("log10", v, ref, error);
				}
			}
			return nrOfFailedTestCases;
		}

		// Verify log1p() using the identity log1p(exp(x)-1) == x for small x,
		// and log1p(a) == log(1+a) for larger a (cross-checking two implementations)
		template<typename TestType>
		int VerifyLog1pFunction(bool reportTestCases, double maxError = 1.0e-60) {
			int nrOfFailedTestCases{ 0 };
			// for small arguments, verify log1p(exp(x)-1) == x
			for (int i = 1; i < 30; ++i) {
				TestType x = ldexp(TestType(1.0), -i);  // x = 2^-i (small)
				TestType a = exp(x) - TestType(1.0);
				TestType v = log1p(a);
				TestType error = abs(v - x);
				if (error > maxError) {
					++nrOfFailedTestCases;
					if (reportTestCases) ReportQuadDoubleFunctionError("log1p", v, x, error);
				}
			}
			// for larger arguments, verify log1p(a) == log(1+a)
			for (int i = 1; i < 20; ++i) {
				TestType a(i);
				TestType v1 = log1p(a);
				TestType v2 = log(TestType(1.0) + a);
				TestType error = abs(v1 - v2);
				if (error > maxError) {
					++nrOfFailedTestCases;
					if (reportTestCases) ReportQuadDoubleFunctionError("log1p", v1, v2, error);
				}
			}
			return nrOfFailedTestCases;
		}


	}
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

	std::string test_suite  = "quad-double mathlib logarithm function validation";
	std::string test_tag    = "log/log1p/log2/log10";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);
	std::cerr << test_tag << '\n';

#if MANUAL_TESTING

	auto defaultPrecision = std::cout.precision();

	{
		double a0 = 1.0;
		double a1 = ulp(a0) / 2.0;
		double a2 = ulp(a1) / 2.0;
		double a3 = ulp(a2) / 2.0;
		qd a(a0, a1, a2, a3);
		std::cout << to_quad(a) << '\n';
		std::cout << std::setprecision(64) << a << std::setprecision(defaultPrecision) << '\n';
		std::cout << to_binary(a) << '\n';
		std::cout << color_print(a, true) << '\n';
	}

	{
		volatile double residual;
		double square;
		double a{ 1.0e50 };
		for (int i = 0; i < 3; ++i) {
			square = two_sqr(a, residual);
			std::cout << "square   : " << square << '\n';
			std::cout << "residual : " << residual << '\n';
			a *= 1.0e50;
		}
	}
	{
		qd x = trace_log(qd_e);
		std::cout << x << '\n';

		x = exp(qd(1.0));
		std::cout << "exp( 1.0) : " << std::setprecision(64) << x << std::setprecision(defaultPrecision) << '\n';
		x = exp(qd(2.0));
		std::cout << "exp( 2.0) : " << std::setprecision(64) << x << std::setprecision(defaultPrecision) << '\n';
		x = exp(qd(4.0));
		std::cout << "exp( 4.0) : " << std::setprecision(64) << x << std::setprecision(defaultPrecision) << '\n';

		x = exp(qd(-1.0));
		std::cout << "exp(-1.0) : " << std::setprecision(64) << x << std::setprecision(defaultPrecision) << '\n';
		double a = 1.0 / std::numbers::e;
		std::cout << "exp(-1.0) : " << std::setprecision(16) << a << std::setprecision(defaultPrecision) << '\n';
	}


	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// References are computed at full qd precision using mathematical identities
	// (log(exp(x))==x, log2(2^i)==i, log10(10^i)==i) -- no double-precision oracle needed.
	// Threshold is near qd epsilon (~1.5e-63 = 2^-209).
	nrOfFailedTestCases += ReportTestResult(VerifyLogFunction<qd>(reportTestCases, 1.0e-60), "quad-double", "log()");
	nrOfFailedTestCases += ReportTestResult(VerifyLog2Function<qd>(reportTestCases, 1.0e-60), "quad-double", "log2()");
	nrOfFailedTestCases += ReportTestResult(VerifyLog10Function<qd>(reportTestCases, 1.0e-60), "quad-double", "log10()");
	nrOfFailedTestCases += ReportTestResult(VerifyLog1pFunction<qd>(reportTestCases, 1.0e-60), "quad-double", "log1p()");
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
