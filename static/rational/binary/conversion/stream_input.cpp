// stream_input.cpp: test suite runner for reading rationals from text: operator>> and parse()
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// rational's operator>> was declared but never defined, and the one definition took a const& and
// read a member rational does not have (#1929). parse() and operator>> are generic over the Base,
// so this runner covers all four: base2, base8, base10 and base16.
#include <universal/utility/directives.hpp>
#include <universal/number/rational/rational.hpp>
#include <universal/verification/test_suite.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct Case {
	const char* text;
	bool        ok;      // the parse must succeed
	double      value;   // and produce this value
};

// tokens a 16-bit rational holds exactly, and tokens no rational accepts
const std::vector<Case> cases16 = {
	{ "3",          true,   3.0 },
	{ "-7",         true,  -7.0 },
	{ "+12",        true,  12.0 },
	{ "3/4",        true,   0.75 },
	{ "-22/7",      true, -22.0 / 7.0 },
	{ "0/5",        true,   0.0 },
	{ "1.5",        true,   1.5 },
	{ "1.50",       true,   1.5 },
	{ "-0.125",     true,  -0.125 },
	{ ".5",         true,   0.5 },
	{ "2.",         true,   2.0 },
	{ "0.1",        true,   0.1 },
	{ "2.5e-3",     true,   0.0025 },
	{ "1E2",        true, 100.0 },
	{ "1.25e+1",    true,  12.5 },
	{ "0e-99999",   true,   0.0 },
	{ "1/0",        false,  0.0 },   // zero denominator
	{ "12x",        false,  0.0 },   // trailing text
	{ "/3",         false,  0.0 },
	{ "3/",         false,  0.0 },
	{ "3/-4",       false,  0.0 },   // the sign goes in front
	{ "1.5/2",      false,  0.0 },   // n/d takes integers
	{ "-",          false,  0.0 },
	{ ".",          false,  0.0 },
	{ "e5",         false,  0.0 },
	{ "1e",         false,  0.0 },
	{ "1e+",        false,  0.0 },
	{ "nan",        false,  0.0 },
	{ "100000000000000000000", false, 0.0 },   // does not fit
	{ "1e40",       false,  0.0 },   // does not fit
	{ "1e-40",      false,  0.0 },   // does not fit
};

// rational<8>: what fits and what does not
const std::vector<Case> cases8 = {
	{ "127",        true,  127.0 },
	{ "-127",       true, -127.0 },
	{ "-128",       true, -128.0 },    // the type minimum: its magnitude does not fit (#1973)
	{ "-129",       false,   0.0 },
	{ "-1.28e2",    true, -128.0 },
	{ "-12.8e1",    true, -128.0 },
	{ "-128/2",     true,  -64.0 },
	{ "-128/128",   false,   0.0 },    // the denominator is unsigned, and 128 does not fit
	{ "128",        false,   0.0 },
	{ "1.27e2",     true,  127.0 },
	{ "-0.125",     true,  -0.125 },   // 125 / 10^3 reduces to 1/8 as it goes
	{ "0.01",       true,    0.01 },
	{ "2.5e-3",     false,   0.0 },    // 1/400
	{ "1000/1000",  false,   0.0 },    // 1000 does not fit, although the value does
};

template<typename Rational>
int VerifyCases(const std::vector<Case>& cases, bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;
	for (const Case& c : cases) {
		// operator>>: a failed parse sets failbit and leaves the target unchanged
		Rational v(42);
		std::istringstream istr(c.text);
		istr >> v;
		bool ok = !istr.fail();
		// the expected values are the correctly rounded doubles of the exact rationals, and a
		// rational converts to double by one correctly rounded division, so == is exact here
		bool pass = (ok == c.ok) && (double(v) == (c.ok ? c.value : 42.0));
		// parse() directly gives the same answer
		Rational p(7);
		if (parse(std::string(c.text), p) != c.ok || double(p) != (c.ok ? c.value : 7.0)) pass = false;
		if (!pass) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL " << type_tag(Rational()) << " '" << c.text << "' -> "
				<< (ok ? "ok " : "fail ") << double(v) << ", expected " << (c.ok ? "ok " : "fail ") << c.value << '\n';
		}
	}
	return nrOfFailedTests;
}

// every value rational<8> encodes, written as n/d, reads back as the same value, down to the
// numerator -128
template<typename Rational>
int VerifyRoundTrip(bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;
	for (int n = -128; n <= 127; ++n) {
		for (int d = 1; d <= 127; ++d) {
			Rational a;
			a.set(n, d);
			std::stringstream s;
			s << n << '/' << d;
			Rational b(0);
			s >> b;
			if (s.fail() || !(a == b)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL round trip " << n << '/' << d << " -> " << double(b) << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// every integer from 0 to limit, written in decimal: the parse succeeds exactly when the value
// fits (-maxInt - 1 <= v <= maxInt, two's complement), and then gives the value. The arithmetic wraps on overflow, so this is
// what proves the overflow checks: 12*10 + 8 wraps to -128 in 8 bits, and undoing it with a
// subtraction wraps back, which an earlier version of the check accepted
template<typename Rational>
int VerifyIntegerRange(long long maxInt, long long limit, bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;
	for (long long i = 0; i <= limit; ++i) {
		for (int sign = 0; sign < 2; ++sign) {
			std::string txt = (sign ? "-" : "") + std::to_string(i);
			Rational v(0);
			bool ok = parse(txt, v);
			bool fits = sign ? (i <= maxInt + 1) : (i <= maxInt);   // -128 fits in 8 bits, 128 does not
			double expected = sign ? -double(i) : double(i);
			if (ok != fits || (ok && double(v) != expected)) {
				++nrOfFailedTests;
				if (reportTestCases && nrOfFailedTests < 10) std::cerr << "FAIL " << type_tag(Rational()) << " '" << txt << "' -> "
					<< (ok ? "ok " : "fail ") << double(v) << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// several tokens from one stream, and a bad one in the middle stops the extraction
template<typename Rational>
int VerifyStream(bool reportTestCases) {
	int nrOfFailedTests = 0;
	std::istringstream istr("3/4 -1.5 2e1 bogus 7");
	Rational a(0), b(0), c(0), d(5), e(9);
	istr >> a >> b >> c;
	if (istr.fail() || double(a) != 0.75 || double(b) != -1.5 || double(c) != 20.0) ++nrOfFailedTests;
	istr >> d;
	if (!istr.fail() || double(d) != 5.0) ++nrOfFailedTests;   // bogus: failbit, d unchanged
	istr >> e;
	if (double(e) != 9.0) ++nrOfFailedTests;                    // a failed stream extracts nothing
	if (nrOfFailedTests && reportTestCases) std::cerr << "FAIL stream extraction\n";
	return nrOfFailedTests;
}

} // namespace

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

	std::string test_suite  = "rational stream input and parse";
	std::string test_tag    = "operator>>";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using rb8  = rational<8,  base2,  std::uint8_t>;
	using rb16 = rational<16, base2,  std::uint16_t>;
	using ro16 = rational<16, base8,  std::uint16_t>;
	using rd16 = rational<16, base10, std::uint16_t>;
	using rh16 = rational<16, base16, std::uint16_t>;

#if MANUAL_TESTING

	rb16 v;
	std::istringstream istr("-22/7");
	istr >> v;
	std::cout << istr.fail() << ' ' << to_binary(v) << " : " << v << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyCases<rb16>(cases16, reportTestCases), type_tag(rb16()), test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyCases<ro16>(cases16, reportTestCases), type_tag(ro16()), test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyCases<rd16>(cases16, reportTestCases), type_tag(rd16()), test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyCases<rh16>(cases16, reportTestCases), type_tag(rh16()), test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyCases<rb8>(cases8, reportTestCases), type_tag(rb8()), "range");

	nrOfFailedTestCases += ReportTestResult(VerifyIntegerRange<rb8>(127, 2000, reportTestCases), type_tag(rb8()), "integer range");
	nrOfFailedTestCases += ReportTestResult(VerifyIntegerRange<rb16>(32767, 400000, reportTestCases), type_tag(rb16()), "integer range");

	nrOfFailedTestCases += ReportTestResult(VerifyStream<rb16>(reportTestCases), type_tag(rb16()), "stream");
	nrOfFailedTestCases += ReportTestResult(VerifyStream<rd16>(reportTestCases), type_tag(rd16()), "stream");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrip<rb8>(reportTestCases), type_tag(rb8()), "n/d round trip");
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
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
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
