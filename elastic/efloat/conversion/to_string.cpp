// to_string.cpp: regression tests for efloat decimal output (operator<< / to_string).
//
// Before #1150, operator<< emitted the literal "TBD" for every finite value.
// It now produces a correctly-rounded decimal string at arbitrary precision.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

template<unsigned nlimbs>
std::string sci(const sw::universal::efloat<nlimbs>& v, int prec) {
	std::ostringstream os;
	os << std::scientific << std::setprecision(prec) << v;
	return os.str();
}

int VerifyEfloatToString(bool reportTestCases) {
	using namespace sw::universal;
	using E      = efloat<16>;  // 512 bits
	int failures = 0;

	// ---------------------------------------------------------------------
	// 1. no more "TBD"; basic scientific values match std for double-range
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying basic decimal output...\n";
	{
		if (sci(E(1.5), 6).find("TBD") != std::string::npos) {
			if (reportTestCases)
				std::cout << "    FAIL: still prints TBD\n";
			++failures;
		}
		for (double d : {1.5, -3.25, 1234.5678, 2.0 / 3.0, 0.1, 1e-9}) {
			std::ostringstream es;
			es << std::scientific << std::setprecision(14) << E(d);
			std::ostringstream ds;
			ds << std::scientific << std::setprecision(14) << d;
			if (es.str() != ds.str()) {
				if (reportTestCases)
					std::cout << "    FAIL: " << d << " efloat=" << es.str() << " std=" << ds.str() << "\n";
				++failures;
			}
		}
	}

	// ---------------------------------------------------------------------
	// 2. high precision: to_string of pi matches the efloat_pi oracle after
	//    parsing back (round-trip) to well beyond double precision
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying high-precision round-trip...\n";
	{
		for (int prec : {20, 50, 100}) {
			std::string str = sci(efloat_pi<16>(), prec);
			E           rt;
			rt.set_precision(600);
			parse<16384>(str, rt);
			E d = rt - efloat_pi<16>();
			d.setsign(false);
			int64_t sc = d.iszero() ? -100000 : d.scale();
			// prec decimal digits ~ prec*3.32 bits of agreement; require prec-2 digits
			int64_t want = -static_cast<int64_t>((prec - 2) * 3.321928);
			if (sc > want) {
				if (reportTestCases)
					std::cout << "    FAIL: pi@" << prec << " round-trip scale=" << sc << " (want <=" << want << ")\n";
				++failures;
			}
		}
	}

	// ---------------------------------------------------------------------
	// 3. round-trip of arbitrary finite values: parse(to_string(x)) ~= x
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying round-trip of finite values...\n";
	{
		for (double d : {1.5, -3.25, 2.0 / 3.0, 6.022e23, -1e-15}) {
			E x(d);
			E rt;
			rt.set_precision(120);
			parse<16384>(sci(x, 30), rt);
			if (std::abs(double(rt) - d) > std::abs(d) * 1e-14 + 1e-300) {
				if (reportTestCases)
					std::cout << "    FAIL: round-trip " << d << " -> " << double(rt) << "\n";
				++failures;
			}
		}
	}

	// ---------------------------------------------------------------------
	// 4. fixed format
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying fixed format...\n";
	{
		std::ostringstream os;
		os << std::fixed << std::setprecision(4) << E(1.5);
		if (os.str() != "1.5000") {
			if (reportTestCases)
				std::cout << "    FAIL: fixed 1.5 -> " << os.str() << "\n";
			++failures;
		}
		std::ostringstream os2;
		os2 << std::fixed << std::setprecision(2) << E(-3.25);
		if (os2.str() != "-3.25") {
			if (reportTestCases)
				std::cout << "    FAIL: fixed -3.25 -> " << os2.str() << "\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 5. special values (including the -inf sign, which isneg() misses)
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying special values...\n";
	{
		auto out = [](const E& v) {
			std::ostringstream o;
			o << v;
			return o.str();
		};
		E pinf;
		pinf.setinf(false);
		E ninf;
		ninf.setinf(true);
		E nan;
		nan.setnan();
		if (out(pinf) != "inf") {
			if (reportTestCases)
				std::cout << "    FAIL: +inf -> " << out(pinf) << "\n";
			++failures;
		}
		if (out(ninf) != "-inf") {
			if (reportTestCases)
				std::cout << "    FAIL: -inf -> " << out(ninf) << "\n";
			++failures;
		}
		if (out(nan) != "nan") {
			if (reportTestCases)
				std::cout << "    FAIL: nan -> " << out(nan) << "\n";
			++failures;
		}
		std::ostringstream z;
		z << std::scientific << std::setprecision(3) << E(0.0);
		if (z.str() != "0.000e+00") {
			if (reportTestCases)
				std::cout << "    FAIL: zero -> " << z.str() << "\n";
			++failures;
		}
	}

	return failures;
}

}  // anonymous namespace

#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat decimal output (operator<<) library";
	std::string test_tag            = "to_string";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatToString(reportTestCases), "efloat", "to_string");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
} catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
