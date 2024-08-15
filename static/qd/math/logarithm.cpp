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

		// generate specific test case 
		template<typename Ty>
		void GenerateLogTestCase(Ty fa) {
			unsigned precision = 25;
			unsigned width = 30;
			Ty fref;
			qd a, ref, v;
			a = fa;
			fref = std::log(fa);
			ref = fref;
			v = sw::universal::log(a);
			qd error = (v - ref);
			auto oldPrec = std::cout.precision();
			std::cout << std::setprecision(precision);
			std::cout << " -> log(" << fa << ") = " << std::setw(width) << fref << '\n';
			std::cout << " -> log( " << a << ") = " << v << '\n' << to_binary(v) << '\n';
			std::cout << to_binary(ref) << "\n -> reference\n";
			std::cout << "    error  : " << error << '\n';
			std::cout << (ref == v ? "PASS" : "FAIL") << '\n';
			std::cout << '\n';
			std::cout << std::setprecision(oldPrec);
		}

		template<typename Ty>
		void GenerateLog2TestCase(Ty fa) {
			unsigned precision = 25;
			unsigned width = 30;
			Ty fref;
			qd a, ref, v;
			a = fa;
			fref = std::log2(fa);
			ref = fref;
			v = sw::universal::log2(a);
			qd error = (v - ref);
			auto oldPrec = std::cout.precision();
			std::cout << std::setprecision(precision);
			std::cout << " -> log2(" << fa << ") = " << std::setw(width) << fref << '\n';
			std::cout << " -> log2( " << a << ") = " << v << '\n' << to_binary(v) << '\n';
			std::cout << to_binary(ref) << "\n -> reference\n";
			std::cout << "    error  : " << error << '\n';
			std::cout << (ref == v ? "PASS" : "FAIL") << '\n';
			std::cout << '\n';
			std::cout << std::setprecision(oldPrec);
		}

		template<typename Ty>
		void GenerateLog10TestCase(Ty fa) {
			unsigned precision = 25;
			unsigned width = 30;
			Ty fref;
			qd a, ref, v;
			a = fa;
			fref = std::log10(fa);
			ref = fref;
			v = sw::universal::log10(a);
			auto oldPrec = std::cout.precision();
			qd error = (v - ref);
			std::cout << std::setprecision(precision);
			std::cout << " -> log10(" << fa << ") = " << std::setw(width) << fref << '\n';
			std::cout << " -> log10( " << a << ") = " << v << '\n' << to_binary(v) << '\n';
			std::cout << to_binary(ref) << "\n -> reference\n";
			std::cout << "    error  : " << error << '\n';
			std::cout << (ref == v ? "PASS" : "FAIL") << '\n';
			std::cout << '\n';
			std::cout << std::setprecision(oldPrec);
		}

		template<typename Ty>
		void GenerateLog1pTestCase(Ty fa) {
			unsigned precision = 25;
			unsigned width = 30;
			Ty fref;
			qd a, ref, v;
			a = fa;
			fref = std::log1p(fa);
			ref = fref;
			v = sw::universal::log1p(a);
			auto oldPrec = std::cout.precision();
			qd error = (v - ref);
			std::cout << std::setprecision(precision);
			std::cout << " -> log1p(" << fa << ") = " << std::setw(width) << fref << '\n';
			std::cout << " -> log1p( " << a << ") = " << v << '\n' << to_binary(v) << '\n';
			std::cout << to_binary(ref) << "\n -> reference\n";
			std::cout << "    error  : " << error << '\n';
			std::cout << (ref == v ? "PASS" : "FAIL") << '\n';
			std::cout << '\n';
			std::cout << std::setprecision(oldPrec);
		}

		template<typename TestType>
		void ReportQuadDoubleFunctionError(const std::string& op, const TestType& a, const TestType& ref, const TestType& error) {
			std::cerr << op << " : " << a << " != " << ref << " : error : " << error << '\n';
		}

		template<typename TestType>
		int VerifyLogFunction(bool reportTestCases, double maxError = 1.0e-15) {
			using std::log;
			int nrOfFailedTestCases{ 0 };
			constexpr double eulersNr = std::numbers::e;
			for (int i = -64; i < 65; ++i) {
				double da = std::pow(eulersNr, double(i));
				TestType a = da;
				double dref = log(da);
				TestType ref = dref;
				TestType v = log(a);
				TestType error = abs(v - ref);
				if (error > maxError) {
					++nrOfFailedTestCases;
					if (reportTestCases) ReportQuadDoubleFunctionError("log", v, ref, error);
				}
			}

			return nrOfFailedTestCases;
		}

		template<typename TestType>
		int VerifyLog2Function(bool reportTestCases, double maxError = 1.0e-15) {
			using std::log2;
			int nrOfFailedTestCases{ 0 };
			for (int i = -64; i < 65; ++i) {
				double da = std::pow(2.0, double(i));
				TestType a = da;
				double dref = log2(da);
				TestType ref = dref;
				TestType v = log2(a);
				TestType error = abs(v - ref);
				if (error > maxError) {
					++nrOfFailedTestCases;
					if (reportTestCases) ReportQuadDoubleFunctionError("log2", v, ref, error);
				}
			}

			return nrOfFailedTestCases;
		}

		template<typename TestType>
		int VerifyLog10Function(bool reportTestCases, double maxError = 1.0e-15) {
			using std::log10;
			int nrOfFailedTestCases{ 0 };
			for (int i = -64; i < 65; ++i) {
				double da = std::pow(2.0, double(i));
				TestType a = da;
				double dref = log10(da);
				TestType ref = dref;
				TestType v = log10(a);
				TestType error = abs(v - ref);
				if (error > maxError) {
					++nrOfFailedTestCases;
					if (reportTestCases) ReportQuadDoubleFunctionError("log10", v, ref, error);
				}
			}

			return nrOfFailedTestCases;
		}

		template<typename TestType>
		int VerifyLog1pFunction(bool reportTestCases, double maxError = 1.0e-15) {
			using std::log1p;
			int nrOfFailedTestCases{ 0 };
			for (int i = -64; i < 65; ++i) {
				double da = std::pow(2.0, double(i));
				TestType a = da;
				double dref = log1p(da);
				TestType ref = dref;
				TestType v = log1p(a);
				TestType error = abs(v - ref);
				if (error > maxError) {
					++nrOfFailedTestCases;
					if (reportTestCases) ReportQuadDoubleFunctionError("log1p", v, ref, error);
				}
			}

			return nrOfFailedTestCases;
		}


	}
}


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

	return 0;

	GenerateLogTestCase(1.0);
	GenerateLogTestCase(std::numbers::e);
	GenerateLogTestCase(pow(std::numbers::e, 2.0));


	GenerateLog2TestCase(1.0);
	GenerateLog2TestCase(2.0);
	GenerateLog2TestCase(4.0);

	{
		std::stringstream s;
		double maxError = 1.0e-14;
		s << maxError;
		std::string test_id = "log(error < " + s.str() + ")";
		nrOfFailedTestCases += ReportTestResult(VerifyLogFunction<qd>(reportTestCases, maxError), "quad-double", test_id);
	}

	{
		std::stringstream s;
		double maxError = 1.0e-29;
		s << maxError;
		std::string test_id = "log2(error < " + s.str() + ")";
		nrOfFailedTestCases += ReportTestResult(VerifyLog2Function<qd>(reportTestCases, maxError), "quad-double", test_id);
	}

	{
		std::stringstream s;
		double maxError = 1.0e-15;
		s << maxError;
		std::string test_id = "log10(error < " + s.str() + ")";
		nrOfFailedTestCases += ReportTestResult(VerifyLog10Function<qd>(reportTestCases, maxError), "quad-double", test_id);
	}

	{
		std::stringstream s;
		double maxError = 1.0e-14;
		s << maxError;
		std::string test_id = "log1p(error < " + s.str() + ")";
		nrOfFailedTestCases += ReportTestResult(VerifyLog1pFunction<qd>(reportTestCases, maxError), "quad-double", test_id);
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	std::cout << "NOTE: quad-double log functions are LESS accurate than stdlib double: \ncurrently log() is accurate to just 14 digits, quad-double should have 32 digits of accuracy\n";
	nrOfFailedTestCases += ReportTestResult(VerifyLogFunction<qd>(reportTestCases, 1.0e-14), "quad-double", "log()");
	nrOfFailedTestCases += ReportTestResult(VerifyLog2Function<qd>(reportTestCases, 1.0e-14), "quad-double", "log2()");
	nrOfFailedTestCases += ReportTestResult(VerifyLog10Function<qd>(reportTestCases, 1.0e-14), "quad-double", "log10()");
	nrOfFailedTestCases += ReportTestResult(VerifyLog1pFunction<qd>(reportTestCases, 1.0e-14), "quad-double", "log1p()");
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
