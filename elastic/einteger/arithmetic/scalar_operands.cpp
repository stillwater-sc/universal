// scalar_operands.cpp: test suite runner for einteger arithmetic with native scalar operands
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <string>
#include <sstream>
#include <universal/number/einteger/einteger.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	template<typename BlockType>
	bool equals(const einteger<BlockType>& got, long long expected) {
		std::ostringstream os, es;
		os << got;
		es << expected;
		return os.str() == es.str();
	}

	// + - * / % with a native operand of type Scalar on either side, against native long long
	// arithmetic. a / 2 did not compile before #1987: the long long and unsigned long long
	// overloads of / tied for an int operand.
	template<typename BlockType, typename Scalar>
	int VerifyScalarOperands(const char* scalarName, bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		auto check = [&](const einteger<BlockType>& got, long long expected, long long a, const char* op, long long b) {
			if (!equals(got, expected)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << a << ' ' << op << ' ' << b << " (" << scalarName << ") = " << got << ", expected " << expected << '\n';
			}
		};
		const long long values[] = { -1000003, -77, -12, -1, 0, 1, 7, 12, 99, 1000003 };
		const long long scalars[] = { 1, 2, 3, 7, 10, 100 };
		for (long long a : values) {
			const einteger<BlockType> ea(a);
			for (long long sv : scalars) {
				const Scalar s = static_cast<Scalar>(sv);
				check(ea + s, a + sv, a, "+", sv);
				check(ea - s, a - sv, a, "-", sv);
				check(ea * s, a * sv, a, "*", sv);
				check(ea / s, a / sv, a, "/", sv);
				check(ea % s, a % sv, a, "%", sv);
				check(s + ea, sv + a, sv, "+", a);
				check(s - ea, sv - a, sv, "-", a);
				check(s * ea, sv * a, sv, "*", a);
				if (a != 0) {
					check(s / ea, sv / a, sv, "/", a);
					check(s % ea, sv % a, sv, "%", a);
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// a floating-point operand converts as einteger(double) does, truncating toward zero
	template<typename BlockType>
	int VerifyFloatingOperands(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const einteger<BlockType> a(100);
		const einteger<BlockType> results[] = { a / 2.5, a * 2.0f, a + 1.9, 3.5 - a, 250.0 / a };
		const long long refs[] = { 50, 200, 101, -97, 2 };
		for (size_t k = 0; k < sizeof(refs) / sizeof(refs[0]); ++k) {
			if (!equals(results[k], refs[k])) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: floating case " << k << " = " << results[k] << ", expected " << refs[k] << '\n';
			}
		}
		return nrOfFailedTestCases;
	}

	// A NaN or infinite operand converts to 0, as einteger(double) converts it and as edecimal does
	// (#1992): einteger has no NaN or inf to carry. So + and - leave the einteger alone, * gives 0,
	// and a division or remainder by the operand is a division by zero, reported as einteger reports
	// it (#2010). The compound forms used to convert the operand to long long, undefined for NaN and
	// inf, and a += NaN came out a - 2^63.
	template<typename BlockType, typename Real>
	int VerifyNonFiniteOperands(const char* realName, bool reportTestCases) {
		using EInteger = einteger<BlockType>;
		int nrOfFailedTestCases = 0;
		auto check = [&](const EInteger& got, long long expected, long long a, const char* op, const char* x) {
			if (!equals(got, expected)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << a << ' ' << op << ' ' << x << " (" << realName << ") = " << got << ", expected " << expected << '\n';
			}
		};
		// a / x, a % x, a /= x and a %= x divide by zero: they throw, or give 0 when the exception is opted out
		auto checkDivideByZero = [&](auto op, long long a, const char* label, const char* x) {
#if EINTEGER_THROW_DIVIDE_BY_ZERO
			try {
				const EInteger r = op();
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << a << ' ' << label << ' ' << x << " (" << realName << ") = " << r << ", expected einteger_divide_by_zero\n";
			}
			catch (const einteger_divide_by_zero&) {}
#else
			check(op(), 0, a, label, x);
#endif
		};
		struct NonFinite { Real value; const char* name; };
		const NonFinite operands[] = {
			{ std::numeric_limits<Real>::quiet_NaN(), "nan" },
			{ -std::numeric_limits<Real>::quiet_NaN(), "-nan" },
			{ std::numeric_limits<Real>::infinity(), "inf" },
			{ -std::numeric_limits<Real>::infinity(), "-inf" },
		};
		const long long values[] = { -1000003, -12, -1, 0, 1, 7, 1000003 };
		for (long long a : values) {
			const EInteger ea(a);
			for (const NonFinite& op : operands) {
				const Real x = op.value;
				check(ea + x, a, a, "+", op.name);
				check(ea - x, a, a, "-", op.name);
				check(ea * x, 0, a, "*", op.name);
				check(x + ea, a, a, "+ (reversed)", op.name);
				check(x - ea, -a, a, "- (reversed)", op.name);
				check(x * ea, 0, a, "* (reversed)", op.name);
				if (a != 0) {
					check(x / ea, 0, a, "/ (reversed)", op.name);
					check(x % ea, 0, a, "% (reversed)", op.name);
				}
				checkDivideByZero([&] { return EInteger(ea / x); }, a, "/", op.name);
				checkDivideByZero([&] { return EInteger(ea % x); }, a, "%", op.name);
				EInteger r(ea);
				r += x;
				check(r, a, a, "+=", op.name);
				r -= x;
				check(r, a, a, "-=", op.name);
				r *= x;
				check(r, 0, a, "*=", op.name);
				checkDivideByZero([&] { EInteger q(ea); q /= x; return q; }, a, "/=", op.name);
				checkDivideByZero([&] { EInteger q(ea); q %= x; return q; }, a, "%=", op.name);
			}
		}
		// a finite compound operand past the long long range converts as einteger(double) does
		{
			EInteger r(100);
			r -= Real(1e18) * Real(1e18);
			EInteger ref(Real(1e18) * Real(1e18));
			ref = EInteger(100) - ref;
			if (r != ref) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: 100 -= 1e36 (" << realName << ") = " << r << ", expected " << ref << '\n';
			}
			EInteger q(7);
			q *= Real(2.75);
			q += Real(-0.5);
			q /= Real(3.9);
			if (!equals(q, 4)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: ((7 *= 2.75) += -0.5) /= 3.9 (" << realName << ") = " << q << ", expected 4\n";
			}
		}
		return nrOfFailedTestCases;
	}

}}  // namespace sw::universal

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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "einteger arithmetic with native scalar operands";
	std::string test_tag    = "scalar operands";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	einteger<std::uint32_t> a(10);
	std::cout << a << " / 2 = " << (a / 2) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	using U32 = std::uint32_t;
	nrOfFailedTestCases += ReportTestResult(VerifyScalarOperands<U32, int>("int", reportTestCases), "einteger<uint32_t>", "int");
	nrOfFailedTestCases += ReportTestResult(VerifyScalarOperands<U32, unsigned>("unsigned", reportTestCases), "einteger<uint32_t>", "unsigned");
	nrOfFailedTestCases += ReportTestResult(VerifyScalarOperands<U32, short>("short", reportTestCases), "einteger<uint32_t>", "short");
	nrOfFailedTestCases += ReportTestResult(VerifyScalarOperands<U32, signed char>("signed char", reportTestCases), "einteger<uint32_t>", "signed char");
	nrOfFailedTestCases += ReportTestResult(VerifyScalarOperands<U32, unsigned char>("unsigned char", reportTestCases), "einteger<uint32_t>", "unsigned char");
	nrOfFailedTestCases += ReportTestResult(VerifyScalarOperands<U32, long>("long", reportTestCases), "einteger<uint32_t>", "long");
	nrOfFailedTestCases += ReportTestResult(VerifyScalarOperands<U32, long long>("long long", reportTestCases), "einteger<uint32_t>", "long long");
	nrOfFailedTestCases += ReportTestResult(VerifyScalarOperands<U32, unsigned long long>("unsigned long long", reportTestCases), "einteger<uint32_t>", "unsigned long long");
	nrOfFailedTestCases += ReportTestResult(VerifyScalarOperands<std::uint8_t, int>("int", reportTestCases), "einteger<uint8_t>", "int");
	nrOfFailedTestCases += ReportTestResult(VerifyFloatingOperands<U32>(reportTestCases), "einteger<uint32_t>", "float and double");
	nrOfFailedTestCases += ReportTestResult(VerifyNonFiniteOperands<U32, float>("float", reportTestCases), "einteger<uint32_t>", "float nan/inf");
	nrOfFailedTestCases += ReportTestResult(VerifyNonFiniteOperands<U32, double>("double", reportTestCases), "einteger<uint32_t>", "double nan/inf");
	nrOfFailedTestCases += ReportTestResult(VerifyNonFiniteOperands<std::uint8_t, double>("double", reportTestCases), "einteger<uint8_t>", "double nan/inf");
#if LONG_DOUBLE_SUPPORT
	nrOfFailedTestCases += ReportTestResult(VerifyNonFiniteOperands<U32, long double>("long double", reportTestCases), "einteger<uint32_t>", "long double nan/inf");
#endif
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
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
