// fma.cpp: the bounded posit's fused multiply-add, fma(a, b, c) = a*b + c rounded once
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// fma accumulates a*b and c exactly in the quire and rounds once (#1250).  The reference
// is the exact value of a*b + c as an integer at the quire's scale 2^-radix_point, where
// every bposit value and every product of two is an integer (the same oracle as fdp.cpp).
// The result must be that value rounded once: to nearest, ties to the even encoding, and
// saturated to +/-minpos or +/-maxpos, never to 0 or NaR.
//
// The small configurations are checked over every (a, b) pair with c sampled, which
// covers the cases a double rounding gets wrong: a*b + c landing within a tie of a
// midpoint, and a*b cancelling c down to a residue below minpos.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <universal/number/bposit/bposit.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/traits/fma_traits.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using BigInt = sw::universal::integer<1024, std::uint32_t>;

static_assert(sw::universal::has_fma_v<sw::universal::bposit32>, "bposit has a type-preserving fma");

// v * 2^radix_point as an integer; exact for every bposit value
template<typename B>
BigInt scaled(const B& v) {
	BigInt r(0);
	if (v.iszero()) return r;
	const auto d = v.decode_fields();
	constexpr int rp = static_cast<int>(sw::universal::quire_traits<B>::radix_point);
	const std::uint64_t S = (1ull << d.nf) | d.frac;
	r = BigInt(static_cast<long long>(S >> 1));                  // keep the integer's sign bit clear
	r = r + r + BigInt(static_cast<long long>(S & 1ull));
	const int shift = d.scale - static_cast<int>(d.nf) + rp;    // >= rp/2 > 0
	r <<= shift;
	return d.negative ? -r : r;
}

// x * y * 2^radix_point; both values are integers at scale 2^-(rp/2)
template<typename B>
BigInt scaled_product(const B& x, const B& y) {
	if (x.iszero() || y.iszero()) return BigInt(0);
	constexpr int half = static_cast<int>(sw::universal::quire_traits<B>::radix_point) / 2;
	BigInt a = scaled(x), b = scaled(y);
	a >>= half; b >>= half;                                      // exact: both are multiples of 2^half
	return a * b;
}

BigInt babs(const BigInt& v) { return (v < BigInt(0)) ? -v : v; }

// is `got` the exact value X (at scale 2^-radix_point) rounded once into B?
template<typename B>
bool correctly_rounded(const B& got, const BigInt& X) {
	using sw::universal::SpecificValue;
	if (X == BigInt(0)) return got.iszero();
	if (got.isnar() || got.iszero()) return false;
	const bool neg = X < BigInt(0);
	if (got.sign() != neg) return false;
	const B maxpos(SpecificValue::maxpos), minpos(SpecificValue::minpos);
	const BigInt ax = babs(X);
	if (ax >= scaled(maxpos)) return got == (neg ? -maxpos : maxpos);
	if (ax <= scaled(minpos)) return got == (neg ? -minpos : minpos);
	const BigInt mine = babs(X - scaled(got));
	for (int k = -1; k <= 1; k += 2) {
		B nb(got);
		if (k < 0) --nb; else ++nb;
		if (nb.isnar() || nb.iszero()) continue;
		const BigInt d = babs(X - scaled(nb));
		if (d < mine) return false;
		if (d == mine && (got.raw() & 1ull)) return false;      // a tie must go to the even encoding
	}
	return true;
}

template<typename B>
bool VerifyTriple(const B& a, const B& b, const B& c, int& fails, bool report) {
	if (a.isnar() || b.isnar() || c.isnar()) return true;
	const B got = fma(a, b, c);
	const BigInt X = scaled_product(a, b) + scaled(c);
	if (correctly_rounded(got, X)) return true;
	++fails;
	if (report && fails < 10) {
		std::cerr << "FAIL " << sw::universal::type_tag(got) << " fma(" << to_binary(a) << ", " << to_binary(b) << ", "
		          << to_binary(c) << ") = " << to_binary(got) << '\n';
	}
	return false;
}

// every (a, b) pair, with `perPair` values of c drawn from the whole encoding range
template<typename B>
int VerifyAllPairs(unsigned perPair, bool report) {
	int fails = 0;
	std::uint64_t state = 0x853C49E6748FEA9Bull + B::nbits;
	constexpr std::uint64_t N = 1ull << B::nbits;
	for (std::uint64_t i = 0; i < N; ++i) {
		B a; a.setbits(i);
		for (std::uint64_t j = 0; j < N; ++j) {
			B b; b.setbits(j);
			for (unsigned k = 0; k < perPair; ++k) {
				state = state * 6364136223846793005ull + 1442695040888963407ull;
				B c; c.setbits(state >> (64 - B::nbits));
				VerifyTriple(a, b, c, fails, report);
			}
		}
	}
	return fails;
}

// c = -(a*b rounded): a*b + c is the product's rounding error, which is often below minpos
template<typename B>
int VerifyResidues(bool report) {
	int fails = 0;
	constexpr std::uint64_t N = 1ull << B::nbits;
	for (std::uint64_t i = 0; i < N; ++i) {
		B a; a.setbits(i);
		for (std::uint64_t j = 0; j < N; ++j) {
			B b; b.setbits(j);
			if (a.isnar() || b.isnar()) continue;
			const B p = a * b;
			VerifyTriple(a, b, -p, fails, report);
			B up(p), down(p);
			++up; --down;
			VerifyTriple(a, b, -up, fails, report);
			VerifyTriple(a, b, -down, fails, report);
		}
	}
	return fails;
}

template<typename B>
int VerifyRandom(unsigned count, bool report) {
	int fails = 0;
	std::uint64_t state = 0x2545F4914F6CDD1Dull + B::nbits;
	auto next = [&]() {
		state = state * 6364136223846793005ull + 1442695040888963407ull;
		B v; v.setbits(state >> (64 - B::nbits));
		return v;
	};
	for (unsigned t = 0; t < count; ++t) {
		const B a = next();
		const B b = next();
		const B c = next();
		VerifyTriple(a, b, c, fails, report);
		VerifyTriple(a, b, -(a * b), fails, report);             // and the product's rounding error
	}
	return fails;
}

template<typename B>
int VerifySpecialCases(bool report) {
	using sw::universal::SpecificValue;
	int fails = 0;
	auto expect = [&](bool ok, const char* what) {
		if (!ok) { ++fails; if (report) std::cerr << "FAIL " << sw::universal::type_tag(B{}) << ' ' << what << '\n'; }
	};
	const B maxpos(SpecificValue::maxpos), minpos(SpecificValue::minpos), nar(SpecificValue::nar);
	const B zero(0), one(1), two(2), three(3);
	expect(fma(nar, one, one).isnar(), "NaR * 1 + 1 is NaR");
	expect(fma(one, nar, one).isnar(), "1 * NaR + 1 is NaR");
	expect(fma(one, one, nar).isnar(), "1 * 1 + NaR is NaR");
	expect(fma(zero, nar, one).isnar(), "0 * NaR + 1 is NaR");
	expect(fma(two, three, one) == B(7), "2 * 3 + 1 is 7");
	expect(fma(zero, three, two) == two, "0 * 3 + 2 is 2");
	expect(fma(two, three, zero) == B(6), "2 * 3 + 0 is 6");
	expect(fma(two, three, B(-6)).iszero(), "2 * 3 - 6 is exactly 0");
	expect(fma(maxpos, maxpos, -maxpos) == maxpos, "maxpos^2 - maxpos saturates to maxpos");
	expect(fma(minpos, minpos, zero) == minpos, "minpos^2 saturates to minpos, not 0");
	expect(fma(minpos, -minpos, zero) == -minpos, "-minpos^2 saturates to -minpos, not 0");
	// the ADL two-step a generic kernel uses
	using std::fma;
	expect(fma(two, three, one) == B(7), "using std::fma; fma(...) finds the bposit overload");
	return fails;
}

}  // anonymous namespace

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
	std::string test_suite  = "bposit fused multiply-add";
	std::string test_tag    = "fma";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	using B6  = bposit<6, 2, 0, std::uint8_t>;
	using B8  = bposit<8, 4, 0, std::uint8_t>;
	using B8e = bposit<8, 2, 2, std::uint8_t>;

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases<bposit32>(true), "bposit32", "special cases");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases<B8>(reportTestCases),        "bposit<8,4,0>",  "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases<bposit16>(reportTestCases),  "bposit16",       "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases<bposit32>(reportTestCases),  "bposit32",       "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases<bposit64>(reportTestCases),  "bposit64",       "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifyAllPairs<B6>(8, reportTestCases),         "bposit<6,2,0>",  "all pairs");
	nrOfFailedTestCases += ReportTestResult(VerifyResidues<B6>(reportTestCases),            "bposit<6,2,0>",  "product residues");
	nrOfFailedTestCases += ReportTestResult(VerifyResidues<B8>(reportTestCases),            "bposit<8,4,0>",  "product residues");
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<bposit16>(2000, reportTestCases),  "bposit16",       "random");
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<bposit32>(2000, reportTestCases),  "bposit32",       "random");
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<bposit64>(1000, reportTestCases),  "bposit64",       "random");
#endif
#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyAllPairs<B8>(4, reportTestCases),         "bposit<8,4,0>",  "all pairs");
	nrOfFailedTestCases += ReportTestResult(VerifyAllPairs<B8e>(4, reportTestCases),        "bposit<8,2,2>",  "all pairs");
	nrOfFailedTestCases += ReportTestResult(VerifyResidues<B8e>(reportTestCases),           "bposit<8,2,2>",  "product residues");
#endif
#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<bposit32>(20000, reportTestCases), "bposit32",       "random");
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
