// exponent_extremes.cpp: regression tests for efloat's print and parse at the ends of its exponent range.
//
// Guards issue #2005: efloat's binary exponent reaches +/-2^62, a decimal exponent of about
// +/-1.39e18, but to_string computed the decimal exponent in an int, so min() printed as
// 0.000000e+2147483647, and parse() read the exponent as an int32 and handed literals past the
// exact converter's reach (a decimal exponent past about +/-13000) back as NaN, 0e100000 too.
//
// What is checked:
//   - min(), max(), nextafter(0, 1) and 2^(2^40) print their leading digits and decimal exponent
//   - print/parse round trip: parse(to_string(v, d)) == v, exactly, for d = ceil(p log10(2)) + 2
//     significant digits at precision p. That many digits put the printed value within a tenth
//     of a unit in v's last place, and parse rounds it back to v. The values are the exponent
//     extremes above, their negatives, and ordinary values at the ends of the range and near 1.
//   - literals at any exponent: a zero is a zero (keeping its sign), 1e100000 and 1e-100000 are
//     values, an exponent past the range overflows to inf or underflows to 0, and an exponent
//     too large for any integer type saturates instead of overflowing
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

template<unsigned nlimbs>
std::string scientific(const efloat<nlimbs>& v, std::streamsize precision) {
	return to_string(v, precision, 0, false, true, false, false, false, false, ' ');
}

// 2^e at the type's precision
template<unsigned nlimbs>
efloat<nlimbs> power_of_two(std::int64_t e) {
	efloat<nlimbs> v(1);
	v.set_precision(nlimbs * 32u);
	v.setexponent(e);
	return v;
}

// the leading digits and exponent of values whose decimal exponent does not fit an int,
// against references from mpmath at 50 digits
int VerifyExtremePrint(bool reportTestCases) {
	int nrOfFailedTests = 0;
	using E = efloat<4>;
	struct expectation { E value; const char* label; const char* text; };
	const std::vector<expectation> cases = {
		{ std::numeric_limits<E>::min(),       "min()",           "1.7019382623e-1388255822130839283" },
		{ -std::numeric_limits<E>::min(),      "-min()",          "-1.7019382623e-1388255822130839283" },
		{ std::numeric_limits<E>::max(),       "max()",           "1.1751307578e+1388255822130839283" },
		{ nextafter(E(0), E(1)),               "nextafter(0, 1)", "1.7019382623e-1388255822130839283" },
		{ power_of_two<4>(std::int64_t(1) << 40),    "2^(2^40)",  "8.0572322451e+330985980541" },
	};
	for (const auto& c : cases) {
		const std::string s = scientific(c.value, 10);
		if (s != c.text) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: to_string(" << c.label << ") = " << s << " (want " << c.text << ")\n";
		}
	}
	// the fixed format of a value with more integer digits than a string can hold prints in
	// scientific; one far below 1 prints as zeros
	const std::string fmax = to_string(std::numeric_limits<E>::max(), 6, 0, true, false, false, false, false, false, ' ');
	if (fmax != "1.175131e+1388255822130839283") {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: fixed max() = " << fmax << '\n';
	}
	const std::string fmin = to_string(std::numeric_limits<E>::min(), 6, 0, true, false, false, false, false, false, ' ');
	if (fmin != "0.000000") {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: fixed min() = " << fmin << '\n';
	}
	return nrOfFailedTests;
}

// parse(to_string(v)) == v at enough digits
template<unsigned nlimbs>
int VerifyRoundTrip(bool reportTestCases) {
	int nrOfFailedTests = 0;
	using E = efloat<nlimbs>;
	const unsigned p = nlimbs * 32u;
	const std::streamsize digits = static_cast<std::streamsize>((static_cast<std::uint64_t>(p) * 30103u + 99999u) / 100000u) + 2;

	const E third = E(1) / E(3);
	const E pi(3.141592653589793);
	std::vector<E> values = {
		std::numeric_limits<E>::min(),
		std::numeric_limits<E>::max(),
		nextafter(E(0), E(1)),
		nextafter(std::numeric_limits<E>::max(), E(0)),
		nextafter(std::numeric_limits<E>::min(), E(1)),
		power_of_two<nlimbs>(std::int64_t(1) << 40),
		power_of_two<nlimbs>(-(std::int64_t(1) << 40)),
		third, pi, E(1e300), E(1e-5), E(123456789), E(0.1),
	};
	// a third and pi at the ends of the range and in between
	for (std::int64_t e : { E::max_scale, E::min_scale, std::int64_t(1) << 50, -(std::int64_t(1) << 50),
	                        std::int64_t(1000000), std::int64_t(-1000000), std::int64_t(40000), std::int64_t(-40000) }) {
		E t(third); t.setexponent(e); values.push_back(t);
		E q(pi);    q.setexponent(e); values.push_back(q);
	}
	const std::size_t n = values.size();
	for (std::size_t i = 0; i < n; ++i) values.push_back(-values[i]);

	for (const E& v : values) {
		const std::string s = scientific(v, digits - 1);
		E w;
		const bool ok = parse(s, w);
		if (!ok || w != v || w.sign() != v.sign()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: efloat<" << nlimbs << "> round trip of " << s << " (ok " << ok << ") reads back as " << scientific(w, digits - 1) << '\n';
		}
	}
	return nrOfFailedTests;
}

// literals at any exponent
template<unsigned nlimbs>
int VerifyLiteralExponents(bool reportTestCases) {
	int nrOfFailedTests = 0;
	using E = efloat<nlimbs>;
	auto fail = [&](const char* txt, const char* why) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: efloat<" << nlimbs << "> parse(" << txt << "): " << why << '\n';
	};

	// zeros, whatever their exponent, keep their sign
	for (const char* txt : { "0e100000", "0.000e-100000", "0e99999999999999999999", "0", "0e5" }) {
		E v;
		if (!parse(txt, v) || !v.iszero() || v.sign() == -1) fail(txt, "not +0");
	}
	for (const char* txt : { "-0e100000", "-0e5", "-0.0e-99999999999999999999" }) {
		E v;
		if (!parse(txt, v) || !v.iszero() || v.sign() != -1) fail(txt, "not -0");
	}

	// values past the exact converter's reach, against the value's own arithmetic: 10^100000 is
	// (10^1000)^100, each 10^1000 exact to the converter, the powers at the type's precision
	{
		E v, w, x;
		if (!parse("1e100000", v) || v.isnan() || v.isinf() || v.iszero()) fail("1e100000", "not a value");
		if (!parse("1e-100000", w) || w.isnan() || w.isinf() || w.iszero()) fail("1e-100000", "not a value");
		if (!parse("-2.5e100000", x) || x.sign() != -1 || x.isinf()) fail("-2.5e100000", "not a negative value");
		// 10^100000 lies in [2^332192, 2^332193)
		if (v.scale() != 332192) fail("1e100000", "wrong scale");
		if (w.scale() != -332193) fail("1e-100000", "wrong scale");
		// x == -2.5 * v and v * w == 1, within a few units in the last place
		const E eps = std::numeric_limits<E>::epsilon();
		const E r1 = abs(x / v + E(2.5)) / E(2.5);
		const E r2 = abs(v * w - E(1));
		if (r1 > E(4) * eps) fail("-2.5e100000", "not -2.5 * 1e100000");
		if (r2 > E(4) * eps) fail("1e-100000", "not 1 / 1e100000");
		// the printed digits
		if (scientific(v, 10) != "1.0000000000e+100000") fail("1e100000", scientific(v, 10).c_str());
		if (scientific(w, 10) != "1.0000000000e-100000") fail("1e-100000", scientific(w, 10).c_str());
	}

	// past the exponent range: inf and 0, with the literal's sign; the exponent saturates
	// rather than overflowing int64, and the digits before it count
	struct beyond { const char* txt; bool inf; bool negative; };
	const beyond cases[] = {
		{ "1e99999999999999999999",       true,  false },
		{ "-1e99999999999999999999",      true,  true  },
		{ "1e-99999999999999999999",      false, false },
		{ "-1e-99999999999999999999",     false, true  },
		{ "1e2000000000000000000",        true,  false },
		{ "1e-2000000000000000000",       false, false },
		{ "1e9223372036854775807",        true,  false },
		{ "1e-9223372036854775808",       false, false },
		{ "1e+18446744073709551616",      true,  false },
		{ "123.45e1388255822130839283",   true,  false },   // 1.2345e1388255822130839285: above max()
		{ "0.0001e-1388255822130839280",  false, false },   // 1e-1388255822130839284: below min()
	};
	for (const auto& c : cases) {
		E v;
		const bool ok = parse(c.txt, v);
		const bool right = ok && (c.inf ? v.isinf() : v.iszero()) && ((v.sign() == -1) == c.negative);
		if (!right) fail(c.txt, c.inf ? "not inf" : "not 0");
	}

	// just inside the range: values, not inf or 0
	for (const char* txt : { "1.1e1388255822130839283", "1.8e-1388255822130839283", "-1e1388255822130839283" }) {
		E v;
		if (!parse(txt, v) || v.isinf() || v.iszero() || v.isnan()) fail(txt, "not a finite value");
	}

	// malformed exponents are still rejected, and leave a zero
	for (const char* txt : { "1e", "1e+", "1e-", "1e5e5", "1e5.0", "1ee5", "e5", "1e 5" }) {
		E v;
		if (parse(txt, v) || !v.iszero()) fail(txt, "accepted");
	}
	return nrOfFailedTests;
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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "efloat print and parse at the exponent extremes";
	std::string test_tag    = "exponent extremes";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyExtremePrint(true), "efloat<4>", "print");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrip<4>(true), "efloat<4>", "round trip");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExtremePrint(reportTestCases), "efloat<4>", "print");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrip<2>(reportTestCases), "efloat<2>", "round trip");
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrip<4>(reportTestCases), "efloat<4>", "round trip");
	nrOfFailedTestCases += ReportTestResult(VerifyLiteralExponents<4>(reportTestCases), "efloat<4>", "literal exponents");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrip<8>(reportTestCases), "efloat<8>", "round trip");
	nrOfFailedTestCases += ReportTestResult(VerifyLiteralExponents<2>(reportTestCases), "efloat<2>", "literal exponents");
	nrOfFailedTestCases += ReportTestResult(VerifyLiteralExponents<8>(reportTestCases), "efloat<8>", "literal exponents");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrip<16>(reportTestCases), "efloat<16>", "round trip");
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
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
