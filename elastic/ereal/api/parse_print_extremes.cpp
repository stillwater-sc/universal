// parse_print_extremes.cpp: ereal parse at the exponent limits and fixed-format rounding below 10^-p (#2005)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// parse accumulated the decimal exponent in an int, so ereal("1e4294967297") wrapped to 1e1
// (UBSan: signed overflow). The exponent saturates now, and the value goes to inf or 0.
// Fixed-format to_string printed every value below 10^-p as zero: 0.006 at two digits was
// "0.00", where printf gives "0.01".
#include <universal/utility/directives.hpp>
#include <cstdio>
#include <iostream>
#include <string>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	template<typename Real>
	int VerifyParseExponentLimits(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		struct Case { const char* text; int kind; bool negative; };   // kind: 1 inf, 0 zero
		const Case cases[] = {
			{ "1e4294967297", 1, false },
			{ "-1e4294967297", 1, true },
			{ "1e-4294967297", 0, false },
			{ "-1e-4294967297", 0, true },
			{ "1e99999999999999999999", 1, false },
			{ "1e-99999999999999999999", 0, false },
			{ "0.5e2147483648", 1, false },
			{ "123456e-2147483649", 0, false },
		};
		for (const auto& c : cases) {
			Real v;
			const bool ok = v.parse(c.text);
			const bool right = ok && ((c.kind == 1) ? v.isinf() : v.iszero()) && v.signbit() == c.negative;
			if (!right) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: parse(\"" << c.text << "\") = " << double(v) << (ok ? "" : " (rejected)") << '\n';
			}
		}
		// a zero with any exponent is zero, and a huge exponent does not disturb the digits
		Real z;
		if (!z.parse("0e4294967297") || !z.iszero()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: parse(\"0e4294967297\") is not 0\n";
		}
		Real one;
		if (!one.parse("1000e-3") || one != Real(1.0)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: parse(\"1000e-3\") is not 1\n";
		}
		// a byte above 127 is rejected, without std::isdigit on a negative char
		const std::string highByte = std::string("1") + static_cast<char>(0xB5) + "2";
		Real h(7.0);
		if (h.parse(highByte)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: a byte above 127 parsed\n";
		}
		return nrOfFailedTestCases;
	}

	// fixed format against printf, for values around and below 10^-p
	template<typename Real>
	int VerifyFixedRounding(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const double values[] = { 0.006, 0.0096, 0.004, 0.0051, 0.0049, -0.006, -0.004, 0.0996, 0.00049, 0.0006, 0.00051, 1.006, 12.3456 };
		for (double v : values) {
			for (int p : { 1, 2, 3, 4 }) {
				char buf[64];
				std::snprintf(buf, sizeof(buf), "%.*f", p, v);
				const std::string got = Real(v).to_string(p, 0, true, false);
				if (got != buf) {
					++nrOfFailedTestCases;
					if (reportTestCases) std::cerr << "FAIL: fixed " << v << " at " << p << " digits = '" << got << "', printf '" << buf << "'\n";
				}
			}
		}
		return nrOfFailedTestCases;
	}

}  // namespace

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

	std::string test_suite  = "ereal parse and print at the extremes";
	std::string test_tag    = "parse/print extremes";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ereal<> v;
	v.parse("1e4294967297");
	std::cout << "ereal(\"1e4294967297\") = " << double(v) << '\n';
	std::cout << "0.006 at 2 digits: " << ereal<>(0.006).to_string(2, 0, true, false) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyParseExponentLimits<ereal<>>(reportTestCases), "ereal<8>", "parse exponent limits");
	nrOfFailedTestCases += ReportTestResult(VerifyParseExponentLimits<ereal<4, float>>(reportTestCases), "ereal<4, float>", "parse exponent limits");
	nrOfFailedTestCases += ReportTestResult(VerifyFixedRounding<ereal<>>(reportTestCases), "ereal<8>", "fixed rounding");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyFixedRounding<ereal<2>>(reportTestCases), "ereal<2>", "fixed rounding");
#if LONG_DOUBLE_SUPPORT
	nrOfFailedTestCases += ReportTestResult(VerifyParseExponentLimits<ereal<4, long double>>(reportTestCases), "ereal<4, long double>", "parse exponent limits");
#endif
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
	std::cerr << "Uncaught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught unexpected universal internal exception: " << err.what() << std::endl;
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
