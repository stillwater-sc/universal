// sqrt.cpp: test suite runner for the correctly rounded posit square root
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <bit>
#include <cstdint>
#include <random>
#include <vector>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>

// The oracle is the posit rounding rule itself, independent of how sqrt computes its result.
// y is the correctly rounded sqrt(a) exactly when mid(y-1, y)^2 <= a <= mid(y, y+1)^2, where the
// midpoints are the (nbits+1)-bit patterns 2y-1 and 2y+1, and a tie (equality) goes to the even
// encoding. All comparisons are exact, in integers. The first form is for nbits <= 64; the wide
// form below applies the same rule at any width, on 32-bit words.

namespace {

	// value = sig * 2^(scale - 63), sig in [2^63, 2^64)
	struct Decoded { int scale; std::uint64_t sig; };

	// decode a positive posit from the bits after its sign, left-aligned; bits past the end are zero
	Decoded DecodeBody(std::uint64_t body, unsigned es) {
		const bool positiveRegime = (body >> 63) != 0;
		const int run = positiveRegime ? std::countl_one(body) : std::countl_zero(body);
		const int k = positiveRegime ? run - 1 : -run;
		std::uint64_t rest = (run + 1 >= 64) ? 0ull : (body << (run + 1));
		int exponent = 0;
		if (es > 0) {
			exponent = static_cast<int>(rest >> (64u - es));
			rest <<= es;
		}
		return { k * (1 << es) + exponent, (1ull << 63) | (rest >> 1) };
	}

	template<typename Posit>
	std::uint64_t Body(const Posit& a) { return static_cast<std::uint64_t>(a.encoding()) << (65u - Posit::nbits); }

	template<typename Posit>
	Decoded Decode(const Posit& a) { return DecodeBody(Body(a), Posit::es); }

	// the midpoint between a positive posit y and its upper or lower neighbour, as the
	// (nbits+1)-bit pattern 2y+1 or 2y-1 = 2(y-1)+1
	template<typename Posit>
	Decoded Midpoint(const Posit& y, bool upper) {
		constexpr unsigned nbits = Posit::nbits;
		std::uint64_t body = Body(y);
		if (!upper) body -= (1ull << (65u - nbits));
		body |= (1ull << (64u - nbits));
		return DecodeBody(body, Posit::es);
	}

	// the 128-bit product a * b as (hi, lo)
	void Square(std::uint64_t a, std::uint64_t& hi, std::uint64_t& lo) {
		const std::uint64_t a0 = a & 0xFFFF'FFFFull, a1 = a >> 32;
		const std::uint64_t p00 = a0 * a0, p01 = a0 * a1, p11 = a1 * a1;
		const std::uint64_t mid = (p00 >> 32) + 2 * (p01 & 0xFFFF'FFFFull);
		hi = p11 + 2 * (p01 >> 32) + (mid >> 32);
		lo = (mid << 32) | (p00 & 0xFFFF'FFFFull);
	}

	// the sign of a - m^2, exactly
	int CompareSquare(const Decoded& a, const Decoded& m) {
		// a = a.sig * 2^(a.scale - 63), m^2 = m.sig^2 * 2^(2 m.scale - 126): compare a.sig * 2^(d + 63) with m.sig^2
		const int d = a.scale - 2 * m.scale;
		if (d < 0) return -1;   // a.sig * 2^62 < 2^126 <= m.sig^2
		if (d > 1) return 1;    // a.sig * 2^65 >= 2^128 > m.sig^2
		const std::uint64_t ahi = (d == 1) ? a.sig : (a.sig >> 1);
		const std::uint64_t alo = (d == 1) ? 0ull : (a.sig << 63);
		std::uint64_t mhi, mlo;
		Square(m.sig, mhi, mlo);
		if (ahi != mhi) return ahi < mhi ? -1 : 1;
		if (alo != mlo) return alo < mlo ? -1 : 1;
		return 0;
	}

	// is y the correctly rounded square root of the positive posit a?
	template<typename Posit>
	bool IsCorrectlyRounded(const Posit& a, const Posit& y) {
		if (y.isnar() || y.iszero() || y.isneg()) return false;
		const Decoded da = Decode(a);
		const bool odd = (y.encoding() & 1ull) != 0;
		Posit extreme;
		if (y != extreme.minpos()) {   // nothing rounds to zero, so minpos has no lower boundary
			const int c = CompareSquare(da, Midpoint(y, false));
			if (c < 0 || (c == 0 && odd)) return false;
		}
		if (y != extreme.maxpos()) {
			const int c = CompareSquare(da, Midpoint(y, true));
			if (c > 0 || (c == 0 && odd)) return false;
		}
		return true;
	}

	template<typename Posit>
	int VerifyOne(const Posit& a, bool reportTestCases) {
		const Posit y = sw::universal::sqrt(a);
		if (IsCorrectlyRounded(a, y)) return 0;
		if (reportTestCases) {
			std::cerr << "FAIL sqrt(" << sw::universal::to_binary(a) << ") = " << sw::universal::to_binary(y)
			          << "  (" << a << " -> " << y << ")\n";
		}
		return 1;
	}

	// the oracle's decoder against the posit's own conversion to double, where that is exact
	template<typename Posit>
	int VerifyOracleDecoder(bool reportTestCases) {
		static_assert(Posit::nbits <= 32, "the double comparison is exact only for narrow posits");
		int nrOfFailedTests = 0;
		constexpr std::uint64_t positives = (1ull << (Posit::nbits - 1)) - 1ull;
		const std::uint64_t step = (positives > (1ull << 16)) ? (positives >> 16) : 1ull;
		for (std::uint64_t bits = 1; bits <= positives; bits += step) {
			Posit a;
			a.setbits(bits);
			const Decoded d = Decode(a);
			if (std::ldexp(double(d.sig), d.scale - 63) != double(a)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL decode " << sw::universal::to_binary(a) << '\n';
			}
		}
		return nrOfFailedTests;
	}

	// sqrt(0) = 0, sqrt(NaR) = NaR, a negative argument is NaR, and exact squares are exact
	template<typename Posit>
	int VerifySpecialCases(bool reportTestCases) {
		int nrOfFailedTests = 0;
		auto check = [&](bool ok, const char* what) {
			if (!ok) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL " << sw::universal::type_tag(Posit()) << ' ' << what << '\n';
			}
		};
		Posit zero(0), nar(sw::universal::SpecificValue::nar), mone(-1), one(1);
		check(sw::universal::sqrt(zero).iszero(), "sqrt(0) == 0");
		check(sw::universal::sqrt(nar).isnar(), "sqrt(NaR) == NaR");
		check(sw::universal::sqrt(mone).isnar(), "sqrt(-1) == NaR");
		check(sw::universal::sqrt(one) == one, "sqrt(1) == 1");
		Posit minneg;
		minneg.minneg();
		check(sw::universal::sqrt(minneg).isnar(), "sqrt(minneg) == NaR");
		if constexpr (Posit::nbits >= 8) {
			check(sw::universal::sqrt(Posit(4)) == Posit(2), "sqrt(4) == 2");
			check(sw::universal::sqrt(Posit(0.25)) == Posit(0.5), "sqrt(0.25) == 0.5");
		}
		return nrOfFailedTests;
	}

	// every positive encoding, plus the special cases
	template<typename Posit>
	int VerifySqrtExhaustive(bool reportTestCases) {
		int nrOfFailedTests = VerifySpecialCases<Posit>(reportTestCases);
		constexpr std::uint64_t positives = (1ull << (Posit::nbits - 1)) - 1ull;
		for (std::uint64_t bits = 1; bits <= positives; ++bits) {
			Posit a;
			a.setbits(bits);
			nrOfFailedTests += VerifyOne(a, reportTestCases);
		}
		return nrOfFailedTests;
	}

	// the encodings in [first, last]
	template<typename Posit>
	int VerifySqrtRange(std::uint64_t first, std::uint64_t last, bool reportTestCases) {
		int nrOfFailedTests = 0;
		for (std::uint64_t bits = first; bits <= last; ++bits) {
			Posit a;
			a.setbits(bits);
			nrOfFailedTests += VerifyOne(a, reportTestCases);
		}
		return nrOfFailedTests;
	}

	// random encodings; the inputs nearest the squares of midpoints, whose roots sit next to a
	// rounding boundary; and fractions of all ones, whose roots approach the top of a binade
	template<typename Posit>
	int VerifySqrtRandom(unsigned nrSamples, std::uint64_t seed, bool reportTestCases) {
		constexpr unsigned nbits = Posit::nbits;
		constexpr std::uint64_t positives = (1ull << (nbits - 1)) - 1ull;
		int nrOfFailedTests = VerifySpecialCases<Posit>(reportTestCases);
		std::mt19937_64 rng(seed);
		Posit maxpos;
		maxpos.maxpos();
		for (unsigned i = 0; i < nrSamples; ++i) {
			Posit a;
			a.setbits(1ull + rng() % positives);
			nrOfFailedTests += VerifyOne(a, reportTestCases);

			// near a tie: a = m^2 rounded to the posit, for the upper midpoint m of a random y
			Posit y;
			y.setbits(1ull + rng() % positives);
			if (y == maxpos) continue;
			const Decoded m = Midpoint(y, true);
			std::uint64_t hi, lo;
			Square(m.sig, hi, lo);
			const bool top = (hi >> 63) != 0;   // m.sig^2 in [2^127, 2^128): one more binade
			const std::uint64_t fraction = top ? ((hi << 1) | (lo >> 63)) : ((hi << 2) | (lo >> 62));
			Posit t;
			t.setpositive(2 * m.scale + (top ? 1 : 0), fraction, 64u);
			Posit below(t), above(t);
			--below;
			++above;
			nrOfFailedTests += VerifyOne(t, reportTestCases);
			if (!below.iszero()) nrOfFailedTests += VerifyOne(below, reportTestCases);
			if (!above.isnar()) nrOfFailedTests += VerifyOne(above, reportTestCases);

			// all ones below the hidden bit, at a random scale
			Posit ones;
			ones.setpositive(static_cast<int>(rng() % 64) - 32, ~0ull, 64u);
			--ones;
			if (!ones.iszero()) nrOfFailedTests += VerifyOne(ones, reportTestCases);
		}
		return nrOfFailedTests;
	}

	// the encodings nearest minpos and maxpos
	template<typename Posit>
	int VerifySqrtExtremes(std::uint64_t count, bool reportTestCases) {
		constexpr std::uint64_t positives = (1ull << (Posit::nbits - 1)) - 1ull;
		return VerifySqrtRange<Posit>(1ull, count, reportTestCases)
		     + VerifySqrtRange<Posit>(positives - count + 1ull, positives, reportTestCases);
	}

	template<typename Posit>
	std::uint64_t EncodingOf(double v) { return static_cast<std::uint64_t>(Posit(v).encoding()); }

	////////////////////////////////////////////////////////////////////////////////////////
	// any width: value = sig * 2^(scale - (32 W - 1)), sig in W 32-bit words, least significant
	// first, with its top bit set. W words hold the hidden bit and nbits more.
	struct WideDecoded { int scale; std::vector<std::uint32_t> sig; };

	template<typename Posit>
	constexpr unsigned WideWords() { return (Posit::nbits + 2u + 31u) / 32u; }

	// the bits after the sign of a positive posit, most significant first
	template<typename Posit>
	std::vector<bool> WideBody(const Posit& a) {
		const auto bits = a.bits();
		std::vector<bool> body;
		for (unsigned i = Posit::nbits - 1u; i > 0; --i) body.push_back(bits.test(i - 1u));
		return body;
	}

	WideDecoded WideDecodeBody(const std::vector<bool>& body, unsigned es, unsigned W) {
		const std::size_t len = body.size();
		const bool positiveRegime = body[0];
		std::size_t run = 0;
		while (run < len && body[run] == positiveRegime) ++run;
		const int k = positiveRegime ? static_cast<int>(run) - 1 : -static_cast<int>(run);
		std::size_t pos = run + 1;   // past the regime's terminating bit
		int exponent = 0;
		for (unsigned i = 0; i < es; ++i, ++pos) exponent = (exponent << 1) | ((pos < len && body[pos]) ? 1 : 0);
		WideDecoded d{ k * (1 << es) + exponent, std::vector<std::uint32_t>(W, 0u) };
		unsigned bit = 32u * W - 1u;
		d.sig[bit / 32u] |= 1u << (bit % 32u);
		for (; pos < len; ++pos) {
			--bit;
			if (body[pos]) d.sig[bit / 32u] |= 1u << (bit % 32u);
		}
		return d;
	}

	template<typename Posit>
	WideDecoded WideDecode(const Posit& a) { return WideDecodeBody(WideBody(a), Posit::es, WideWords<Posit>()); }

	// the midpoint between a positive posit y and its upper or lower neighbour: the
	// (nbits+1)-bit pattern of y, or of y's predecessor, with a 1 appended
	template<typename Posit>
	WideDecoded WideMidpoint(const Posit& y, bool upper) {
		Posit base(y);
		if (!upper) --base;
		std::vector<bool> body = WideBody(base);
		body.push_back(true);
		return WideDecodeBody(body, Posit::es, WideWords<Posit>());
	}

	// the 2W-word square of a W-word integer, schoolbook
	std::vector<std::uint32_t> WideSquare(const std::vector<std::uint32_t>& a) {
		const std::size_t W = a.size();
		std::vector<std::uint32_t> p(2 * W, 0u);
		for (std::size_t i = 0; i < W; ++i) {
			std::uint64_t carry = 0;
			for (std::size_t j = 0; j < W; ++j) {
				const std::uint64_t t = std::uint64_t(a[i]) * a[j] + p[i + j] + carry;
				p[i + j] = static_cast<std::uint32_t>(t);
				carry = t >> 32;
			}
			p[i + W] = static_cast<std::uint32_t>(carry);
		}
		return p;
	}

	// the sign of a - m^2, exactly
	int WideCompareSquare(const WideDecoded& a, const WideDecoded& m) {
		// a = a.sig * 2^(a.scale - (32W-1)), m^2 = m.sig^2 * 2^(2 m.scale - 2(32W-1)):
		// compare a.sig * 2^(d + 32W - 1) with m.sig^2, which lies in [2^(64W-2), 2^(64W))
		const int d = a.scale - 2 * m.scale;
		if (d < 0) return -1;
		if (d > 1) return 1;
		const std::size_t W = a.sig.size();
		std::vector<std::uint32_t> A(2 * W, 0u);
		for (std::size_t i = 0; i < W; ++i) A[W + i] = a.sig[i];
		if (d == 0) {   // a.sig * 2^(32W - 1): halve
			for (std::size_t i = 0; i < 2 * W; ++i) A[i] = (A[i] >> 1) | ((i + 1 < 2 * W) ? (A[i + 1] << 31) : 0u);
		}
		const std::vector<std::uint32_t> P = WideSquare(m.sig);
		for (std::size_t i = 2 * W; i > 0; --i) {
			if (A[i - 1] != P[i - 1]) return A[i - 1] < P[i - 1] ? -1 : 1;
		}
		return 0;
	}

	template<typename Posit>
	bool WideIsCorrectlyRounded(const Posit& a, const Posit& y) {
		if (y.isnar() || y.iszero() || y.isneg()) return false;
		const WideDecoded da = WideDecode(a);
		const bool odd = y.bits().test(0);
		Posit extreme;
		if (y != extreme.minpos()) {
			const int c = WideCompareSquare(da, WideMidpoint(y, false));
			if (c < 0 || (c == 0 && odd)) return false;
		}
		if (y != extreme.maxpos()) {
			const int c = WideCompareSquare(da, WideMidpoint(y, true));
			if (c > 0 || (c == 0 && odd)) return false;
		}
		return true;
	}

	template<typename Posit>
	int WideVerifyOne(const Posit& a, bool reportTestCases) {
		const Posit y = sw::universal::sqrt(a);
		if (WideIsCorrectlyRounded(a, y)) return 0;
		if (reportTestCases) {
			std::cerr << "FAIL sqrt(" << sw::universal::to_binary(a) << ") = " << sw::universal::to_binary(y) << '\n';
		}
		return 1;
	}

	// the positive posit 1.f * 2^scale, f the words of sig below its top bit, rounded once
	template<typename Posit>
	Posit WideEncode(int scale, const std::vector<std::uint32_t>& sig, unsigned topBit) {
		constexpr unsigned F = 64u * WideWords<Posit>();
		sw::universal::blocksignificand<F, typename Posit::BlockType> fraction;
		fraction.clear();
		for (unsigned i = 0; i < topBit; ++i) {   // bit topBit - 1 of sig lands at F - 1
			if ((sig[i / 32u] >> (i % 32u)) & 1u) fraction.setbit(F - topBit + i);
		}
		Posit p;
		return sw::universal::convert_<Posit::nbits, Posit::es, typename Posit::BlockType, F>(false, scale, fraction, p);
	}

	// every positive encoding through the wide oracle: its check against the narrow one
	template<typename Posit>
	int WideVerifyExhaustive(bool reportTestCases) {
		int nrOfFailedTests = 0;
		constexpr std::uint64_t positives = (1ull << (Posit::nbits - 1)) - 1ull;
		for (std::uint64_t bits = 1; bits <= positives; ++bits) {
			Posit a;
			a.setbits(bits);
			nrOfFailedTests += WideVerifyOne(a, reportTestCases);
		}
		return nrOfFailedTests;
	}

	// random encodings, the inputs nearest the squares of midpoints, and fractions of all ones,
	// as VerifySqrtRandom
	template<typename Posit>
	int WideVerifyRandom(unsigned nrSamples, std::uint64_t seed, bool reportTestCases) {
		constexpr unsigned nbits = Posit::nbits;
		constexpr unsigned W = WideWords<Posit>();
		int nrOfFailedTests = VerifySpecialCases<Posit>(reportTestCases);
		std::mt19937_64 rng(seed);
		auto randomPositive = [&rng]() {
			decltype(Posit().bits()) bits;
			bits.clear();
			for (unsigned i = 0; i + 1u < nbits; ++i) bits.setbit(i, (rng() & 1u) != 0);
			if (bits.iszero()) bits.setbit(0);
			Posit p;
			p.setbits(bits);
			return p;
		};
		Posit maxpos;
		maxpos.maxpos();
		for (unsigned i = 0; i < nrSamples; ++i) {
			nrOfFailedTests += WideVerifyOne(randomPositive(), reportTestCases);

			// near a tie: m^2 rounded to the posit, for the upper midpoint m of a random y
			const Posit y = randomPositive();
			if (y == maxpos) continue;
			const WideDecoded m = WideMidpoint(y, true);
			const std::vector<std::uint32_t> P = WideSquare(m.sig);
			const bool top = (P[2 * W - 1] >> 31) != 0;   // m.sig^2 in [2^(64W-1), 2^(64W)): one more binade
			Posit t = WideEncode<Posit>(2 * m.scale + (top ? 1 : 0), P, top ? 64u * W - 1u : 64u * W - 2u);
			Posit below(t), above(t);
			--below;
			++above;
			nrOfFailedTests += WideVerifyOne(t, reportTestCases);
			if (!below.iszero()) nrOfFailedTests += WideVerifyOne(below, reportTestCases);
			if (!above.isnar()) nrOfFailedTests += WideVerifyOne(above, reportTestCases);

			// all ones below the hidden bit, at a random scale
			Posit ones = WideEncode<Posit>(static_cast<int>(rng() % 64) - 32, std::vector<std::uint32_t>(2 * W, ~0u), 64u * W - 1u);
			--ones;
			if (!ones.iszero()) nrOfFailedTests += WideVerifyOne(ones, reportTestCases);
		}
		return nrOfFailedTests;
	}

	// the encodings nearest minpos and maxpos
	template<typename Posit>
	int WideVerifyExtremes(unsigned count, bool reportTestCases) {
		int nrOfFailedTests = 0;
		Posit lo, hi;
		lo.minpos();
		hi.maxpos();
		for (unsigned i = 0; i < count; ++i, ++lo, --hi) {
			nrOfFailedTests += WideVerifyOne(lo, reportTestCases) + WideVerifyOne(hi, reportTestCases);
		}
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

	std::string test_suite  = "posit sqrt: correctly rounded";
	std::string test_tag    = "sqrt";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		// #1776: the shim through double gave 0x40000003 -> 0x40000002
		posit<32, 2> a;
		a.setbits(0x4000'0003u);
		posit<32, 2> y = sqrt(a);
		std::cout << to_binary(a) << " -> " << to_binary(y) << (IsCorrectlyRounded(a, y) ? "  correct\n" : "  WRONG\n");
	}
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRandom<posit<64, 3>>(1000, 1776, reportTestCases), "posit<64,3>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	// the oracle itself
	nrOfFailedTestCases += ReportTestResult(VerifyOracleDecoder<posit<16, 1>>(reportTestCases), "posit<16,1>", "oracle decoder");
	nrOfFailedTestCases += ReportTestResult(VerifyOracleDecoder<posit<32, 2>>(reportTestCases), "posit<32,2>", "oracle decoder");
	nrOfFailedTestCases += ReportTestResult(VerifyOracleDecoder<posit<32, 3>>(reportTestCases), "posit<32,3>", "oracle decoder");

	// every encoding
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExhaustive<posit< 2, 0>>(reportTestCases), "posit< 2,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExhaustive<posit< 3, 0>>(reportTestCases), "posit< 3,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExhaustive<posit< 3, 1>>(reportTestCases), "posit< 3,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExhaustive<posit< 4, 0>>(reportTestCases), "posit< 4,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExhaustive<posit< 4, 1>>(reportTestCases), "posit< 4,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExhaustive<posit< 5, 0>>(reportTestCases), "posit< 5,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExhaustive<posit< 5, 2>>(reportTestCases), "posit< 5,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExhaustive<posit< 6, 3>>(reportTestCases), "posit< 6,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExhaustive<posit< 8, 0>>(reportTestCases), "posit< 8,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExhaustive<posit< 8, 1>>(reportTestCases), "posit< 8,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExhaustive<posit< 8, 2>>(reportTestCases), "posit< 8,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExhaustive<posit< 8, 5>>(reportTestCases), "posit< 8,5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExhaustive<posit<10, 1>>(reportTestCases), "posit<10,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExhaustive<posit<12, 2>>(reportTestCases), "posit<12,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExhaustive<posit<16, 1>>(reportTestCases), "posit<16,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExhaustive<posit<16, 2>>(reportTestCases), "posit<16,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExhaustive<posit<16, 1, std::uint16_t>>(reportTestCases), "posit<16,1,uint16_t>", test_tag);

	// random, near ties and near binade tops
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRandom<posit<24, 2>>(20000, 1, reportTestCases), "posit<24,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRandom<posit<32, 2>>(20000, 2, reportTestCases), "posit<32,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRandom<posit<32, 3>>(20000, 3, reportTestCases), "posit<32,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRandom<posit<32, 2, std::uint32_t>>(20000, 4, reportTestCases), "posit<32,2,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRandom<posit<40, 2>>(20000, 5, reportTestCases), "posit<40,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRandom<posit<48, 2>>(20000, 6, reportTestCases), "posit<48,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRandom<posit<56, 2>>(20000, 7, reportTestCases), "posit<56,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRandom<posit<64, 0>>(20000, 8, reportTestCases), "posit<64,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRandom<posit<64, 2>>(20000, 9, reportTestCases), "posit<64,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRandom<posit<64, 3>>(20000, 10, reportTestCases), "posit<64,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRandom<posit<64, 3, std::uint32_t>>(20000, 11, reportTestCases), "posit<64,3,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRandom<posit<64, 3, std::uint64_t>>(20000, 12, reportTestCases), "posit<64,3,uint64_t>", test_tag);

	// #1776: three of the posit<32,2> the shim through double misrounds, and the encodings just above 1
	{
		int nrOfFailures = 0;
		for (std::uint64_t bits : { 0x4000'0003ull, 0x4cc7'1534ull, 0x4dff'6575ull }) {
			posit<32, 2> a;
			a.setbits(bits);
			nrOfFailures += VerifyOne(a, reportTestCases);
		}
		nrOfFailures += VerifySqrtRange<posit<32, 2>>(0x4000'0000u, 0x4000'FFFFu, reportTestCases);
		nrOfFailedTestCases += ReportTestResult(nrOfFailures, "posit<32,2> #1776", test_tag);
	}

	// the wide oracle, against the narrow one's verdicts
	nrOfFailedTestCases += ReportTestResult(WideVerifyExhaustive<posit<12, 2>>(reportTestCases), "posit<12,2> wide oracle", test_tag);
	nrOfFailedTestCases += ReportTestResult(WideVerifyExhaustive<posit<16, 1>>(reportTestCases), "posit<16,1> wide oracle", test_tag);
	nrOfFailedTestCases += ReportTestResult(WideVerifyRandom<posit<32, 2>>(2000, 13, reportTestCases), "posit<32,2> wide oracle", test_tag);
	nrOfFailedTestCases += ReportTestResult(WideVerifyRandom<posit<64, 3>>(2000, 14, reportTestCases), "posit<64,3> wide oracle", test_tag);

	// posits wider than 64 bits (#1846): one limb past 64, the limb boundaries, and the widest
	nrOfFailedTestCases += ReportTestResult(WideVerifyRandom<posit< 65, 2>>(2000, 15, reportTestCases), "posit<65,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(WideVerifyRandom<posit< 80, 3>>(2000, 16, reportTestCases), "posit<80,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(WideVerifyRandom<posit<100, 3>>(2000, 17, reportTestCases), "posit<100,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(WideVerifyRandom<posit<128, 4>>(2000, 18, reportTestCases), "posit<128,4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(WideVerifyRandom<posit<128, 4, std::uint32_t>>(2000, 19, reportTestCases), "posit<128,4,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(WideVerifyRandom<posit<128, 4, std::uint64_t>>(2000, 20, reportTestCases), "posit<128,4,uint64_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(WideVerifyRandom<posit<128, 0>>(2000, 21, reportTestCases), "posit<128,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(WideVerifyRandom<posit<192, 4>>(1000, 22, reportTestCases), "posit<192,4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(WideVerifyRandom<posit<256, 5>>(1000, 23, reportTestCases), "posit<256,5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(WideVerifyRandom<posit< 72, 6>>(2000, 24, reportTestCases), "posit<72,6>", test_tag);
	nrOfFailedTestCases += ReportTestResult(WideVerifyExtremes<posit<128, 4>>(64, reportTestCases), "posit<128,4> extremes", test_tag);
	{
		// past double's range: maxpos = 2^2016 for posit<128,4>, whose root 2^1008 is exact
		int nrOfFailures = 0;
		posit<128, 4> maxpos;
		maxpos.maxpos();
		posit<128, 4> root = sqrt(maxpos);
		if (root * root != maxpos) ++nrOfFailures;
		nrOfFailures += WideVerifyOne(maxpos, reportTestCases);
		if (sqrt(posit<128, 4>(1024)) != posit<128, 4>(32)) ++nrOfFailures;
		nrOfFailedTestCases += ReportTestResult(nrOfFailures, "posit<128,4> range", test_tag);
	}
#endif

#if REGRESSION_LEVEL_2
	// every posit<32,2> in [1, 4): 2^28 encodings
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRange<posit<32, 2>>(EncodingOf<posit<32, 2>>(1.0), EncodingOf<posit<32, 2>>(4.0) - 1u, reportTestCases), "posit<32,2> [1, 4)", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExhaustive<posit<20, 1>>(reportTestCases), "posit<20,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtRandom<posit<64, 3>>(1'000'000, 1846, reportTestCases), "posit<64,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(WideVerifyRandom<posit<128, 4>>(100'000, 1847, reportTestCases), "posit<128,4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(WideVerifyRandom<posit<256, 5>>(20'000, 1848, reportTestCases), "posit<256,5>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExtremes<posit<32, 2>>(1u << 16, reportTestCases), "posit<32,2> extremes", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExtremes<posit<48, 2>>(1u << 16, reportTestCases), "posit<48,2> extremes", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExtremes<posit<64, 3>>(1u << 16, reportTestCases), "posit<64,3> extremes", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExtremes<posit<64, 0>>(1u << 16, reportTestCases), "posit<64,0> extremes", test_tag);
	nrOfFailedTestCases += ReportTestResult(WideVerifyExtremes<posit<128, 4>>(1u << 12, reportTestCases), "posit<128,4> extremes", test_tag);
	nrOfFailedTestCases += ReportTestResult(WideVerifyExtremes<posit<256, 5>>(1u << 10, reportTestCases), "posit<256,5> extremes", test_tag);
#endif

#if REGRESSION_LEVEL_4
	// every positive posit<32,2>: 2^31 - 1 encodings
	nrOfFailedTestCases += ReportTestResult(VerifySqrtExhaustive<posit<32, 2>>(reportTestCases), "posit<32,2>", test_tag);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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
