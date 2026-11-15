// conversion.cpp: encoding and conversion of the bounded posit against an independent reference
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The reference decoder below walks the bits as a string, exactly as the spec reads them
// (docs/number-systems/bposit.md), and shares no code with bposit_impl.hpp.  Against it,
// for every encoding of each small configuration:
//   - the decoded value matches
//   - encodings are monotonic, and every one keeps at least F_min fraction bits
//   - maxpos and minpos match their formulas
//   - double -> bposit round-trips every value
//   - every midpoint between neighbours rounds to the even encoding, and a hair either
//     side rounds to the nearer neighbour
//   - saturation: never to 0 or NaR, clamped to +/-minpos and +/-maxpos
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>
#include <universal/number/bposit/bposit.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// value of pattern p of a b-posit <n, rs, es>, read field by field from a bit string
long double reference_value(std::uint64_t p, unsigned n, unsigned rs, unsigned es, bool& nar, unsigned& fraction_bits) {
	nar = false;
	fraction_bits = 0;
	const std::uint64_t mask = (n == 64) ? ~0ull : ((1ull << n) - 1ull);
	if ((p & mask) == 0ull) return 0.0l;
	if ((p & mask) == (1ull << (n - 1))) { nar = true; return 0.0l; }
	const bool negative = ((p >> (n - 1)) & 1ull) != 0ull;
	const std::uint64_t a = negative ? ((~p + 1ull) & mask) : (p & mask);
	std::string bits;
	for (unsigned i = n - 1; i-- > 0;) bits.push_back(((a >> i) & 1ull) ? '1' : '0');
	const char first = bits[0];
	unsigned run = 0;
	while (run < bits.size() && run < rs && bits[run] == first) ++run;
	const unsigned regime_size = (run == rs) ? rs : run + 1u;
	const int r = (first == '1') ? static_cast<int>(run) - 1 : -static_cast<int>(run);
	int e = 0;
	for (unsigned i = 0; i < es; ++i) e = 2 * e + (bits[regime_size + i] == '1' ? 1 : 0);
	long double f = 0.0l, w = 0.5l;
	for (unsigned i = regime_size + es; i < bits.size(); ++i, w *= 0.5l) {
		if (bits[i] == '1') f += w;
		++fraction_bits;
	}
	const long double v = std::ldexp(1.0l + f, r * (1 << es) + e);
	return negative ? -v : v;
}

template<unsigned nbits, unsigned rs, unsigned es, typename bt = std::uint32_t>
int VerifyEncoding(bool report) {
	using B = sw::universal::bposit<nbits, rs, es, bt>;
	int fails = 0;
	auto fail = [&](const std::string& what) { ++fails; if (report) std::cerr << "FAIL " << sw::universal::type_tag(B{}) << ": " << what << '\n'; };

	// every encoding, in signed (value) order, NaR excluded
	std::vector<B> ordered;
	const std::int64_t half = std::int64_t(1) << (nbits - 1);
	for (std::int64_t t = -half + 1; t < half; ++t) {
		B v;
		v.setbits(static_cast<std::uint64_t>(t));
		ordered.push_back(v);
	}
	long double previous = -std::numeric_limits<long double>::infinity();
	for (const B& v : ordered) {
		bool nar = false;
		unsigned fb = 0;
		const long double want = reference_value(v.raw(), nbits, rs, es, nar, fb);
		if (nar || v.isnar()) { fail("NaR inside the real range"); continue; }
		const long double got = static_cast<long double>(v);
		if (got != want) fail("pattern " + std::to_string(v.raw()) + " decodes to " + std::to_string(double(got)) + ", want " + std::to_string(double(want)));
		if (!(got > previous)) fail("encodings are not monotonic at pattern " + std::to_string(v.raw()));
		previous = got;
		if (!v.iszero() && fb < B::fbitsmin) fail("pattern " + std::to_string(v.raw()) + " carries fewer than F_min fraction bits");
		// round trip through double
		if (B(double(v)) != v) fail("double round trip of pattern " + std::to_string(v.raw()));
	}

	// maxpos and minpos formulas
	const long double maxpos = std::ldexp(2.0l - std::ldexp(1.0l, -int(B::fbitsmin)), B::maxscale);
	const long double minpos = std::ldexp(1.0l + std::ldexp(1.0l, -int(B::fbitsmin)), B::minscale);
	if (static_cast<long double>(B(sw::universal::SpecificValue::maxpos)) != maxpos) fail("maxpos formula");
	if (static_cast<long double>(B(sw::universal::SpecificValue::minpos)) != minpos) fail("minpos formula");

	// midpoints: ties go to the even encoding; a hair off goes to the nearer neighbour
	for (std::size_t i = 0; i + 1 < ordered.size(); ++i) {
		const B lo = ordered[i], hi = ordered[i + 1];
		if (lo.iszero() || hi.iszero()) continue;                   // around zero, saturation decides
		const double dlo = double(lo), dhi = double(hi);
		const double mid = 0.5 * (dlo + dhi);                       // exact: few significant bits
		const B even = (lo.raw() & 1ull) ? hi : lo;
		if (B(mid) != even) fail("midpoint between patterns " + std::to_string(lo.raw()) + " and " + std::to_string(hi.raw()) + " is not ties-to-even");
		if (B(std::nextafter(mid, dlo)) != lo) fail("just below a midpoint does not round down");
		if (B(std::nextafter(mid, dhi)) != hi) fail("just above a midpoint does not round up");
	}

	// saturation
	const B mp(sw::universal::SpecificValue::minpos), MP(sw::universal::SpecificValue::maxpos);
	if (B(double(minpos) / 3.0) != mp || B(-double(minpos) / 3.0) != -mp) fail("below minpos must clamp to +/-minpos");
	if (B(std::ldexp(1.0, B::minscale)) != mp) fail("2^minscale, the zero pattern's power of two, must clamp to minpos");
	if (B(double(maxpos) * 3.0) != MP || B(-double(maxpos) * 3.0) != -MP) fail("above maxpos must clamp to +/-maxpos");
	if (B(std::numeric_limits<double>::denorm_min()) != mp) fail("the smallest double clamps to minpos");

	// integers
	for (int k = -1000; k <= 1000; ++k) {
		if (B(k) != B(double(k))) { fail("integer " + std::to_string(k) + " converts differently from its double"); break; }
	}
	return fails;
}

// wide configurations: sampled round trips and field invariants
template<unsigned nbits, unsigned rs, unsigned es, typename bt>
int VerifySampled(bool report) {
	using B = sw::universal::bposit<nbits, rs, es, bt>;
	int fails = 0;
	std::uint64_t state = 0x9E3779B97F4A7C15ull;
	for (int i = 0; i < 200000; ++i) {
		state = state * 6364136223846793005ull + 1442695040888963407ull;
		B v;
		v.setbits(state >> (64 - nbits));
		if (v.isnar() || v.iszero()) continue;
		// a round trip is exact only through a type that holds the full significand, fbits + 1 bits
		if constexpr (B::fbits + 1 <= unsigned(std::numeric_limits<double>::digits)) {
			if (B(double(v)) != v) { ++fails; if (report) std::cerr << "FAIL " << sw::universal::type_tag(v) << " round trip\n"; }
		}
		if constexpr (B::fbits + 1 <= unsigned(std::numeric_limits<long double>::digits)) {   // x87, not Apple ARM / MSVC
			if (B(static_cast<long double>(v)) != v) { ++fails; if (report) std::cerr << "FAIL " << sw::universal::type_tag(v) << " long double round trip\n"; }
		}
		if (sw::universal::fraction_bits(v) < B::fbitsmin) { ++fails; if (report) std::cerr << "FAIL " << sw::universal::type_tag(v) << " fewer than F_min fraction bits\n"; }
	}
	return fails;
}

}  // anonymous namespace

// The limb transfers between bposit and blocktriple (normalize and assign, a limb at a time since
// #2080) are reached only by arithmetic, so the limb types are checked there: the same operands
// on four limb widths must give the same encodings, and small integer arithmetic must be exact.
// bposit<64,6,5> on uint8_t spans eight limbs, and its products and quotients the most.
template<unsigned nbits, unsigned rs, unsigned es>
int VerifyLimbArithmetic(bool report) {
	using B8  = sw::universal::bposit<nbits, rs, es, std::uint8_t>;
	using B16 = sw::universal::bposit<nbits, rs, es, std::uint16_t>;
	using B32 = sw::universal::bposit<nbits, rs, es, std::uint32_t>;
	using B64 = sw::universal::bposit<nbits, rs, es, std::uint64_t>;
	int fails = 0;
	std::uint64_t state = 0x2080u;
	auto next = [&state]() { state = state * 6364136223846793005ull + 1442695040888963407ull; return state; };
	const std::uint64_t mask = (nbits == 64) ? ~0ull : ((1ull << nbits) - 1ull);
	for (int i = 0; i < 4000; ++i) {
		const std::uint64_t pa = next() & mask, pb = next() & mask;
		for (int op = 0; op < 4; ++op) {
			auto run = [&](auto tag) {
				using B = decltype(tag);
				B a, b;
				a.setbits(pa);
				b.setbits(pb);
				const B c = (op == 0) ? a + b : (op == 1) ? a - b : (op == 2) ? a * b : a / b;
				return static_cast<std::uint64_t>(c.bits()) & mask;
			};
			const std::uint64_t r8 = run(B8{}), r16 = run(B16{}), r32 = run(B32{}), r64 = run(B64{});
			if (r8 != r32 || r16 != r32 || r64 != r32) {
				++fails;
				if (report && fails < 8) std::cerr << "FAIL " << sw::universal::type_tag(B32{}) << " op " << op << " differs across limb types\n";
			}
		}
	}
	// exact small-integer arithmetic on the narrowest limbs
	for (int x = -40; x <= 40; ++x) {
		for (int y = 1; y <= 12; ++y) {
			const B8 a(x), b(y);
			if (static_cast<int>(a + b) != x + y || static_cast<int>(a - b) != x - y || static_cast<int>(a * b) != x * y) {
				++fails;
				if (report && fails < 8) std::cerr << "FAIL " << sw::universal::type_tag(a) << " " << x << " op " << y << " not exact\n";
			}
			if (x % y == 0 && static_cast<int>(a / b) != x / y) ++fails;
		}
	}
	return fails;
}

// Integer casts truncate toward zero and saturate; NaR gives the type's minimum, 0 for the unsigned
// types (posit's rule from #2073, adopted for bposit in #2080).  They cast a double, which made NaR
// and every out-of-range value undefined behaviour and dropped the bits past 53.  Every encoding of a
// small bposit is checked against its exact long double value, truncated and saturated.
template<typename Integer, typename B>
bool CastMatches(const B& x) {
	using Limits = std::numeric_limits<Integer>;
	const Integer got = static_cast<Integer>(x);
	if (x.isnar()) return got == Limits::min();
	const long double t = std::trunc(static_cast<long double>(x));
	const long double limit = std::ldexp(1.0l, Limits::digits);          // 2^digits: past the range
	Integer expected;
	if (t >= limit) expected = Limits::max();
	else if (t < 0.0l && !Limits::is_signed) expected = Limits::min();
	else if (t <= -limit) expected = Limits::min();
	else expected = static_cast<Integer>(t);
	return got == expected;
}

template<unsigned nbits, unsigned rs, unsigned es, typename bt>
int VerifyIntegerCasts(bool report) {
	using B = sw::universal::bposit<nbits, rs, es, bt>;
	int fails = 0;
	for (std::uint64_t i = 0; i < (std::uint64_t(1) << nbits); ++i) {
		B x;
		x.setbits(i);
		const bool ok = CastMatches<int>(x) && CastMatches<long>(x) && CastMatches<long long>(x) &&
		                CastMatches<unsigned int>(x) && CastMatches<unsigned long>(x) && CastMatches<unsigned long long>(x);
		if (!ok) {
			++fails;
			if (report && fails < 8) std::cerr << "FAIL " << sw::universal::type_tag(x) << " integer casts of " << static_cast<double>(x) << '\n';
		}
	}
	return fails;
}

// the 64-bit configuration: exact past 2^53, and the range ends
int VerifyWideIntegerCasts(bool report) {
	using B = sw::universal::bposit<64, 6, 5, std::uint64_t>;
	using sw::universal::SpecificValue;
	int fails = 0;
	auto expect = [&](bool ok, const char* what) { if (!ok) { ++fails; if (report) std::cerr << "FAIL bposit<64,6,5> " << what << '\n'; } };
	const long long big = (1ll << 55) + 1;                                   // 56 bits: exact in bposit<64,6,5>, not in a double
	expect(static_cast<long long>(B(big)) == big, "2^55 + 1 casts back exactly");
	expect(static_cast<long long>(B(-big)) == -big, "-(2^55 + 1) casts back exactly");
	expect(static_cast<unsigned long long>(B(static_cast<unsigned long long>(big))) == static_cast<unsigned long long>(big), "unsigned 2^55 + 1");
	expect(static_cast<int>(B(SpecificValue::nar)) == std::numeric_limits<int>::min(), "int(NaR) is INT_MIN");
	expect(static_cast<long long>(B(SpecificValue::nar)) == std::numeric_limits<long long>::min(), "long long(NaR) is LLONG_MIN");
	expect(static_cast<unsigned>(B(SpecificValue::nar)) == 0u, "unsigned(NaR) is 0");
	expect(static_cast<long long>(B(SpecificValue::maxpos)) == std::numeric_limits<long long>::max(), "maxpos saturates");
	expect(static_cast<long long>(B(SpecificValue::maxneg)) == std::numeric_limits<long long>::min(), "maxneg saturates");
	expect(static_cast<int>(B(1e10)) == std::numeric_limits<int>::max(), "1e10 saturates int");
	expect(static_cast<unsigned>(B(-5)) == 0u, "-5 saturates unsigned to 0");
	expect(static_cast<int>(B(-2.75)) == -2, "-2.75 truncates to -2");
	expect(static_cast<int>(B(SpecificValue::minpos)) == 0, "minpos truncates to 0");
	static_assert(static_cast<int>(B(-7)) == -7, "the casts are constexpr");
	return fails;
}

#define MANUAL_TESTING 0
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
	std::string test_suite  = "bposit encoding and conversion";
	std::string test_tag    = "conversion";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyEncoding<8, 4, 0>(true), "bposit<8,4,0>", test_tag);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEncoding<8, 4, 0>(reportTestCases),  "bposit<8,4,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyEncoding<8, 3, 1>(reportTestCases),  "bposit<8,3,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyEncoding<8, 2, 2>(reportTestCases),  "bposit<8,2,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyEncoding<10, 3, 2>(reportTestCases), "bposit<10,3,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyEncoding<12, 4, 2>(reportTestCases), "bposit<12,4,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyEncoding<12, 6, 1>(reportTestCases), "bposit<12,6,1>", test_tag);
	// the limb types: the encodings themselves, and below, the arithmetic that moves the significand
	nrOfFailedTestCases += ReportTestResult((VerifyEncoding<12, 4, 2, std::uint8_t>(reportTestCases)),  "bposit<12,4,2,uint8>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyEncoding<10, 3, 2, std::uint16_t>(reportTestCases)), "bposit<10,3,2,uint16>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyEncoding<12, 6, 1, std::uint64_t>(reportTestCases)), "bposit<12,6,1,uint64>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyLimbArithmetic<16, 4, 2>(reportTestCases)), "bposit<16,4,2>", "arithmetic across limb types");
	nrOfFailedTestCases += ReportTestResult((VerifyLimbArithmetic<32, 6, 5>(reportTestCases)), "bposit<32,6,5>", "arithmetic across limb types");
	nrOfFailedTestCases += ReportTestResult((VerifyLimbArithmetic<64, 6, 5>(reportTestCases)), "bposit<64,6,5>", "arithmetic across limb types");
	nrOfFailedTestCases += ReportTestResult((VerifyIntegerCasts<12, 4, 2, std::uint8_t>(reportTestCases)),  "bposit<12,4,2>", "integer casts");
	nrOfFailedTestCases += ReportTestResult((VerifyIntegerCasts<16, 6, 5, std::uint16_t>(reportTestCases)), "bposit<16,6,5>", "integer casts");
	nrOfFailedTestCases += ReportTestResult((VerifyIntegerCasts<16, 4, 2, std::uint32_t>(reportTestCases)), "bposit<16,4,2>", "integer casts");
	nrOfFailedTestCases += ReportTestResult(VerifyWideIntegerCasts(reportTestCases), "bposit<64,6,5>", "integer casts");
#endif
#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyEncoding<16, 6, 5>(reportTestCases), "bposit<16,6,5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyEncoding<16, 4, 2>(reportTestCases), "bposit<16,4,2>", test_tag);
#endif
#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifySampled<32, 6, 5, std::uint32_t>(reportTestCases), "bposit<32,6,5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySampled<64, 6, 5, std::uint64_t>(reportTestCases), "bposit<64,6,5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySampled<32, 6, 5, std::uint8_t>(reportTestCases), "bposit<32,6,5,uint8>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySampled<64, 6, 5, std::uint16_t>(reportTestCases), "bposit<64,6,5,uint16>", test_tag);
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
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
