// decimal_parse.cpp: tests for reading decimal text into a fixpnt: parse() and operator>>
//
// fixpnt's parse() read decimal integers only, so operator>> rejected "1.5" (#1931). It now takes
// a fraction and an exponent, and rounds the exact decimal value to nearest, ties to even, without
// going through a double.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.

#include <universal/utility/directives.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/test_reporters.hpp>
#include <cstdint>
#include <random>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct Case {
	const char* text;
	bool        ok;
	double      value;   // ignored when !ok
};

// fixpnt<8,4>: ulp 1/16, range [-8, 7.9375]
const std::vector<Case> modulo8_4 = {
	{ "1.5",          true,   1.5 },
	{ "-0.125",       true,  -0.125 },
	{ ".5",           true,   0.5 },
	{ "2.",           true,   2.0 },
	{ "2.5e-1",       true,   0.25 },
	{ "0.0625",       true,   0.0625 },     // one ulp
	{ "0.03125",      true,   0.0 },        // half an ulp: tie to even, 0
	{ "0.09375",      true,   0.125 },      // one and a half: tie to even, 2 ulps
	{ "0.03125001",   true,   0.0625 },     // above the tie
	{ "0.03124999",   true,   0.0 },        // below it
	{ "0.99999",      true,   1.0 },        // rounding carries into the integer part
	{ "-7.99",        true,  -8.0 },
	{ "-8",           true,  -8.0 },
	{ "12",           true,  -4.0 },        // Modulo: 12 - 16
	{ "1e2",          true,   4.0 },        // 100 mod 16
	{ "1e100000000",  true,   0.0 },        // 10^n mod 16 is 0 for n >= 4
	{ "1e-100000000", true,   0.0 },
	{ "00000000000000000000005", true, 5.0 },
	{ "-0",           true,   0.0 },
	{ "1.2.3",        false,  0.0 },
	{ "1e",           false,  0.0 },
	{ "1e+",          false,  0.0 },
	{ "e5",           false,  0.0 },
	{ ".",            false,  0.0 },
	{ "1.5x",         false,  0.0 },
};

const std::vector<Case> saturate8_4 = {
	{ "1.5",          true,   1.5 },
	{ "-8",           true,  -8.0 },        // maxneg exactly, not -maxpos
	{ "8",            true,   7.9375 },
	{ "12",           true,   7.9375 },
	{ "-12",          true,  -8.0 },
	{ "-10",          true,  -8.0 },        // 10 does not fit: Fixpnt(10) is maxpos, and -1 * maxpos is not maxneg
	{ "-1e1",         true,  -8.0 },
	{ "-1.0e1",       true,  -8.0 },
	{ "10",           true,   7.9375 },
	{ "7.96875",      true,   7.9375 },     // the tie above maxpos rounds and then clamps
	{ "1e100000000",  true,   7.9375 },
	{ "-1e100000000", true,  -8.0 },
	{ "-1e-100000000",true,   0.0 },
};

template<typename Fixpnt>
int VerifyCases(const std::vector<Case>& cases, bool reportTestCases) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;
	for (const Case& c : cases) {
		Fixpnt v(3);
		bool ok = parse(std::string(c.text), v);
		bool pass = (ok == c.ok) && (ok ? double(v) == c.value : double(v) == 3.0);
		if (!pass) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL " << type_tag(Fixpnt()) << " '" << c.text << "' -> "
				<< (ok ? "ok " : "fail ") << double(v) << ", expected " << (c.ok ? "ok " : "fail ") << c.value << '\n';
		}
	}
	return nrOfFailedTests;
}

// the exact decimal of every encoding of fixpnt<16,8> reads back as that encoding
int VerifyRoundTrip(bool reportTestCases) {
	using namespace sw::universal;
	using Fixpnt = fixpnt<16, 8, Modulo, std::uint16_t>;
	int nrOfFailedTests = 0;
	for (unsigned raw = 0; raw < 65536u; ++raw) {
		Fixpnt a;
		a.setbits(raw);
		std::ostringstream os;
		os << std::setprecision(20) << double(a);   // k/256 has at most 8 fraction digits: exact
		Fixpnt b(0);
		if (!parse(os.str(), b) || !(a == b)) {
			if (reportTestCases && nrOfFailedTests < 5) std::cerr << "FAIL round trip " << os.str() << " -> " << double(b) << '\n';
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

// every midpoint between neighbouring encodings of fixpnt<16,8> ties to the even one, and a
// value 10^-25 above or below it rounds away from the tie. The double nearest those strings
// is the midpoint itself, so a parse through double would tie-break them instead.
int VerifyMidpoints(bool reportTestCases) {
	using namespace sw::universal;
	using Fixpnt = fixpnt<16, 8, Modulo, std::uint16_t>;
	int nrOfFailedTests = 0;
	for (int k = -32768; k < 32767; ++k) {
		Fixpnt lo, hi;
		lo.setbits(static_cast<std::uint16_t>(k));
		hi.setbits(static_cast<std::uint16_t>(k + 1));
		std::ostringstream os;
		os << std::setprecision(20) << (k + 0.5) / 256.0;   // (2k+1)/512: exact in 10 fraction digits
		std::string mid = os.str();
		std::string above = mid, below;
		// nudge the magnitude by 10^-25: append to the digits, or borrow for below
		if (mid.find('.') == std::string::npos) mid += ".", above = mid;
		above = mid + "0000000000000000000000001";
		// below: the midpoint's magnitude minus 10^-25 is the digits ...4999...9
		{
			std::string m = mid;
			std::size_t dot = m.find('.');
			std::string frac = m.substr(dot + 1);
			frac.append(25 - (frac.size() < 25 ? frac.size() : 25), '0');
			// subtract 1 at the 25th fraction digit
			int pos = 24;
			while (pos >= 0 && frac[static_cast<std::size_t>(pos)] == '0') { frac[static_cast<std::size_t>(pos)] = '9'; --pos; }
			if (pos >= 0) frac[static_cast<std::size_t>(pos)] = static_cast<char>(frac[static_cast<std::size_t>(pos)] - 1);
			below = m.substr(0, dot + 1) + frac;
		}
		const bool negative = (mid[0] == '-');
		// toward +inf / -inf for the magnitude nudges
		Fixpnt tie   = ((k & 1) == 0) ? lo : hi;
		Fixpnt up    = negative ? lo : hi;    // larger magnitude
		Fixpnt down  = negative ? hi : lo;    // smaller magnitude
		Fixpnt a(0), b(0), c(0);
		bool ok = parse(mid, a) && parse(above, b) && parse(below, c);
		if (!ok || !(a == tie) || !(b == up) || !(c == down)) {
			if (reportTestCases && nrOfFailedTests < 5) std::cerr << "FAIL midpoint " << mid << " -> " << double(a) << ' ' << double(b) << ' ' << double(c) << '\n';
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

// rbits beyond a double's precision: fixpnt<80,64> holds 2^-64 exactly
int VerifyWide(bool reportTestCases) {
	using namespace sw::universal;
	using Fixpnt = fixpnt<80, 64, Modulo, std::uint32_t>;
	int nrOfFailedTests = 0;
	struct Wide { const char* text; unsigned bit; };   // the value 2^(bit - 64), one set bit
	const Wide cases[] = {
		{ "5.42101086242752217003726400434970855712890625e-20", 0 },     // 2^-64
		{ "1.084202172485504434007452800869941711425781250e-19", 1 },    // 2^-63
		{ "0.5", 63 },
		{ "1", 64 },
		{ "32768", 79 - 0 },   // 2^15: Modulo wraps it to -2^15, the sign bit alone
	};
	for (const Wide& w : cases) {
		Fixpnt want, got(0);
		want.clear();
		want.setbit(w.bit, true);
		if (!parse(std::string(w.text), got) || !(got == want)) {
			if (reportTestCases) std::cerr << "FAIL " << type_tag(Fixpnt()) << " '" << w.text << "'\n";
			++nrOfFailedTests;
		}
	}
	// 2^-65 is half an ulp: ties to even (0); a hair above rounds up to 2^-64
	Fixpnt zero, ulp, v(0);
	zero.clear();
	ulp.clear();
	ulp.setbit(0, true);
	if (!parse(std::string("2.710505431213761085018632002174854278564453125e-20"), v) || !(v == zero)) ++nrOfFailedTests;
	if (!parse(std::string("2.7105054312137610850186320021748542785644531251e-20"), v) || !(v == ulp)) ++nrOfFailedTests;
	return nrOfFailedTests;
}

// An independent oracle over every rbits of an 8-bit fixpnt, under both policies. The expected
// encoding is computed in integers: the token is N * 10^p, its scaled value N * 10^p * 2^rbits
// is rounded to nearest, ties to even, and then clamped (Saturate) or wrapped (Modulo) to 8 bits.
// Two CodeRabbit findings on PR #1937 were edge cases of rbits that hand-picked tokens missed:
// fixpnt<8,8> with its sign bit as the top fraction bit, and fixpnt<8,7>, where -0.999 rounds
// to -1 but its magnitude does not fit.
template<unsigned rbits, bool arithmetic>
int VerifyOracle8(std::mt19937_64& rng, bool reportTestCases) {
	using namespace sw::universal;
	using Fixpnt = fixpnt<8, rbits, arithmetic, std::uint8_t>;
	int nrOfFailedTests = 0;
	for (int i = 0; i < 4000; ++i) {
		// [-]int.frac[e(+|-)x]: int 0..300 (often 0 or 1), up to 5 fraction digits, exponent -3..3
		long long ip = (i % 4 == 0) ? 0 : (i % 4 == 1) ? 1 : static_cast<long long>(rng() % 301);
		int fd = static_cast<int>(rng() % 6);
		long long fp = 0, pow10fd = 1;
		for (int k = 0; k < fd; ++k) { fp = fp * 10 + static_cast<long long>(rng() % 10); pow10fd *= 10; }
		int ex = (i % 3 == 0) ? static_cast<int>(rng() % 7) - 3 : 0;
		bool negative = (rng() & 1u) != 0;
		std::string text = (negative ? "-" : "") + std::to_string(ip);
		if (fd > 0) {
			std::string f = std::to_string(fp);
			text += "." + std::string(static_cast<std::size_t>(fd) - f.size(), '0') + f;
		}
		if (ex != 0) text += "e" + std::to_string(ex);
		// value = N * 10^p, N = ip * 10^fd + fp, p = ex - fd
		long long N = ip * pow10fd + fp;
		int p = ex - fd;
		// scaled = N * 2^rbits * 10^p: an integer quotient q and remainder r over den = 10^-p
		long long num = N * (1ll << rbits), den = 1;
		for (int k = 0; k < p; ++k) num *= 10;
		for (int k = 0; k < -p; ++k) den *= 10;
		long long q = num / den, r = num % den;
		if (2 * r > den || (2 * r == den && (q & 1))) ++q;   // round to nearest, ties to even
		long long sq = negative ? -q : q;
		long long expected;
		if (arithmetic == Saturate) expected = (sq > 127) ? 127 : (sq < -128) ? -128 : sq;
		else                        expected = static_cast<long long>(static_cast<std::int8_t>(static_cast<std::uint8_t>(sq & 0xFF)));
		Fixpnt want, got(0);
		want.setbits(static_cast<std::uint64_t>(static_cast<std::uint8_t>(expected & 0xFF)));
		if (!parse(text, got) || !(got == want)) {
			if (reportTestCases && nrOfFailedTests < 3) std::cerr << "FAIL " << type_tag(Fixpnt()) << " '" << text << "' -> " << to_binary(got) << ", expected " << to_binary(want) << '\n';
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

template<bool arithmetic>
int VerifyOracleAllRbits(bool reportTestCases) {
	std::mt19937_64 rng(arithmetic ? 1931 : 19310);
	return VerifyOracle8<0, arithmetic>(rng, reportTestCases) + VerifyOracle8<1, arithmetic>(rng, reportTestCases)
	     + VerifyOracle8<2, arithmetic>(rng, reportTestCases) + VerifyOracle8<3, arithmetic>(rng, reportTestCases)
	     + VerifyOracle8<4, arithmetic>(rng, reportTestCases) + VerifyOracle8<5, arithmetic>(rng, reportTestCases)
	     + VerifyOracle8<6, arithmetic>(rng, reportTestCases) + VerifyOracle8<7, arithmetic>(rng, reportTestCases)
	     + VerifyOracle8<8, arithmetic>(rng, reportTestCases);
}

// operator>>: several tokens, and a bad one sets failbit and leaves the target unchanged
int VerifyStream(bool reportTestCases) {
	using namespace sw::universal;
	using Fixpnt = fixpnt<16, 8, Modulo, std::uint16_t>;
	int nrOfFailedTests = 0;
	std::istringstream istr("1.5 -0.125 2.5e1 bogus");
	Fixpnt a(0), b(0), c(0), d(7);
	istr >> a >> b >> c;
	if (istr.fail() || double(a) != 1.5 || double(b) != -0.125 || double(c) != 25.0) ++nrOfFailedTests;
	std::streambuf* saved = std::cerr.rdbuf(nullptr);   // operator>> reports the bad token on stderr
	istr >> d;
	std::cerr.rdbuf(saved);
	if (!istr.fail() || double(d) != 7.0) ++nrOfFailedTests;
	if (nrOfFailedTests && reportTestCases) std::cerr << "FAIL stream extraction\n";
	return nrOfFailedTests;
}

} // namespace

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "fixpnt<> decimal parse() and operator>>";
	std::string test_tag    = "decimal parse";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	nrOfFailedTestCases += ReportTestResult(VerifyCases< fixpnt<8, 4, Modulo, std::uint8_t> >(modulo8_4, reportTestCases), "fixpnt<8,4,Modulo>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyCases< fixpnt<8, 4, Saturate, std::uint8_t> >(saturate8_4, reportTestCases), "fixpnt<8,4,Saturate>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRoundTrip(reportTestCases), "fixpnt<16,8>", "round trip");
	nrOfFailedTestCases += ReportTestResult(VerifyMidpoints(reportTestCases), "fixpnt<16,8>", "midpoints");
	nrOfFailedTestCases += ReportTestResult(VerifyWide(reportTestCases), "fixpnt<80,64>", "wide");
	nrOfFailedTestCases += ReportTestResult(VerifyStream(reportTestCases), "fixpnt<16,8>", "operator>>");
	nrOfFailedTestCases += ReportTestResult(VerifyOracleAllRbits<Saturate>(reportTestCases), "fixpnt<8,0..8,Saturate>", "integer oracle");
	nrOfFailedTestCases += ReportTestResult(VerifyOracleAllRbits<Modulo>(reportTestCases), "fixpnt<8,0..8,Modulo>", "integer oracle");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
