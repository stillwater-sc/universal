// fdp.cpp: fused dot products of the bounded posit through the generalized quire
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A quire is only worth having if it is exact, so every dot product here is checked
// against an exact reference.  At the quire's scale 2^-radix_point every bposit value,
// and every product of two, is an integer, so the exact sum is an integer too; a
// 1024-bit integer holds it for every configuration below.  The resolved result must be
// that sum rounded once: ties to the even encoding, and saturated to +/-minpos or
// +/-maxpos, never to 0 or NaR.
//
// One test aims at the n-dependence of the quire size: for the standard <n, 6, 5> the
// exact quire spans 768 + 2 (n - 12) bits, not the 800 often quoted for every n.  There,
// a*b - c*d of four values near minpos is nonzero only in bits a fixed 800-bit quire
// would not have; the result must be minpos, not 0.
#include <universal/utility/directives.hpp>
#include <vector>
#include <universal/number/bposit/bposit.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using BigInt = sw::universal::integer<1024, std::uint32_t>;

// the exact quire geometry, independently of the traits
static_assert(sw::universal::quire_traits<sw::universal::bposit16>::range == 776, "bposit16 quire span");
static_assert(sw::universal::quire_traits<sw::universal::bposit32>::range == 808, "bposit32 quire span");
static_assert(sw::universal::quire_traits<sw::universal::bposit64>::range == 872, "bposit64 quire span");

template<typename B>
constexpr bool every_product_scale_fits() {
	using T = sw::universal::quire_traits<B>;
	return T::half_range >= 2u * (B::rs << B::es);              // quire::operator+= skips beyond half_range
}
static_assert(every_product_scale_fits<sw::universal::bposit16>() && every_product_scale_fits<sw::universal::bposit64>(), "half_range");

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

// is `got` the exact sum X (at scale 2^-radix_point) rounded once into B?
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
int VerifyRandomDotProducts(unsigned count, bool report) {
	int fails = 0;
	std::uint64_t state = 0x853C49E6748FEA9Bull;
	auto next = [&]() {
		state = state * 6364136223846793005ull + 1442695040888963407ull;
		B v; v.setbits(state >> (64 - B::nbits));
		if (v.isnar()) v.setzero();
		return v;
	};
	for (unsigned t = 0; t < count; ++t) {
		const std::size_t n = 1 + (t % 48);
		std::vector<B> x(n), y(n);
		BigInt X(0);
		for (std::size_t i = 0; i < n; ++i) { x[i] = next(); y[i] = next(); X = X + scaled_product(x[i], y[i]); }
		const B got = sw::universal::fdp(x, y);
		if (!correctly_rounded(got, X)) {
			++fails;
			if (report && fails < 5) std::cerr << "FAIL " << sw::universal::type_tag(got) << " fdp of length " << n << " = " << double(got) << '\n';
		}
	}
	return fails;
}

// Cancellation: big terms that cancel exactly, leaving a small exact remainder.
template<typename B>
int VerifyCancellation(bool report) {
	int fails = 0;
	const B big = B::pow2(B::maxscale / 2), three(3), seven(7), third = B(1) / B(3);
	const std::vector<B> x{ big, -big, third, big, -big };
	const std::vector<B> y{ three, three, seven, seven, seven };
	BigInt X(0);
	for (std::size_t i = 0; i < x.size(); ++i) X = X + scaled_product(x[i], y[i]);
	const B got = sw::universal::fdp(x, y);
	if (!correctly_rounded(got, X) || got != B(7) * third) {
		++fails;
		if (report) std::cerr << "FAIL " << sw::universal::type_tag(got) << " cancellation: got " << double(got) << '\n';
	}
	return fails;
}

// a*b - c*d near minpos: exactly 2 u^2 p^2, nonzero only far below minpos^2's scale
template<typename B>
int VerifyTinyResidue(bool report) {
	int fails = 0;
	// p = 2^(minscale + 1) is representable (capped regime, exponent 1, empty fraction)
	const B p = B::pow2(B::minscale + 1);
	B a(p), b(p), c(p);
	++a;                                                         // (1 + u) p,   u = 2^-F_min
	++b; ++b;                                                    // (1 + 2u) p
	++c; ++c; ++c;                                               // (1 + 3u) p
	const std::vector<B> x{ a, -c };
	const std::vector<B> y{ b, p };
	BigInt X(0);
	for (std::size_t i = 0; i < x.size(); ++i) X = X + scaled_product(x[i], y[i]);
	const B got = sw::universal::fdp(x, y);
	if (X == BigInt(0) || got != B(sw::universal::SpecificValue::minpos)) {
		++fails;
		if (report) std::cerr << "FAIL " << sw::universal::type_tag(got) << " tiny residue: got " << double(got) << " (must be minpos)\n";
	}
	return fails;
}

template<typename B>
int VerifySpecialCases(bool report) {
	using sw::universal::SpecificValue;
	int fails = 0;
	auto expect = [&](bool ok, const char* what) { if (!ok) { ++fails; if (report) std::cerr << "FAIL " << sw::universal::type_tag(B{}) << ' ' << what << '\n'; } };
	const B maxpos(SpecificValue::maxpos), nar(SpecificValue::nar), one(1), zero(0);
	expect(sw::universal::fdp(std::vector<B>{ one, nar }, std::vector<B>{ one, one }).isnar(), "a NaR term makes the sum NaR");
	expect(sw::universal::fdp(std::vector<B>{ zero, zero }, std::vector<B>{ one, maxpos }).iszero(), "all-zero products sum to zero");
	expect(sw::universal::fdp(std::vector<B>{ maxpos, maxpos }, std::vector<B>{ maxpos, maxpos }) == maxpos, "maxpos^2 + maxpos^2 saturates to maxpos");
	expect(sw::universal::fdp(std::vector<B>{ maxpos, -maxpos }, std::vector<B>{ maxpos, maxpos }).iszero(), "maxpos^2 - maxpos^2 is exactly zero");
	// many maxpos^2 terms stay inside the carry guard, and cancel exactly
	sw::universal::quire<B> q;
	for (int i = 0; i < 1000; ++i) q += sw::universal::quire_mul(maxpos, maxpos);
	for (int i = 0; i < 1000; ++i) q -= sw::universal::quire_mul(maxpos, maxpos);
	expect(q.iszero(), "1000 maxpos^2 terms added and subtracted cancel to zero");
	// strided form
	const std::vector<B> x{ one, B(100), B(2), B(100), B(3) };
	expect(sw::universal::fdp_stride(5, x, 2, x, 2) == B(14), "fdp_stride over every other element: 1 + 4 + 9");
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
	std::string test_suite  = "bposit fused dot product";
	std::string test_tag    = "fdp";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	using B8  = bposit<8, 4, 0, std::uint8_t>;
	[[maybe_unused]] typedef bposit<12, 4, 2, std::uint16_t> B12;   // used from regression level 2

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyRandomDotProducts<B8>(100, true), "bposit<8,4,0>", test_tag);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases<B8>(reportTestCases),        "bposit<8,4,0>",  "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases<bposit32>(reportTestCases),  "bposit32",       "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifyCancellation<bposit32>(reportTestCases),  "bposit32",       "exact cancellation");
	nrOfFailedTestCases += ReportTestResult(VerifyTinyResidue<bposit16>(reportTestCases),   "bposit16",       "tiny residue");
	nrOfFailedTestCases += ReportTestResult(VerifyTinyResidue<bposit32>(reportTestCases),   "bposit32",       "tiny residue");
	nrOfFailedTestCases += ReportTestResult(VerifyTinyResidue<bposit64>(reportTestCases),   "bposit64",       "tiny residue");
	nrOfFailedTestCases += ReportTestResult(VerifyRandomDotProducts<B8>(400, reportTestCases),       "bposit<8,4,0>",  "random dot products");
	nrOfFailedTestCases += ReportTestResult(VerifyRandomDotProducts<bposit16>(400, reportTestCases), "bposit16",       "random dot products");
#endif
#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyRandomDotProducts<B12>(400, reportTestCases),      "bposit<12,4,2>", "random dot products");
	nrOfFailedTestCases += ReportTestResult(VerifyRandomDotProducts<bposit32>(400, reportTestCases), "bposit32",       "random dot products");
	nrOfFailedTestCases += ReportTestResult(VerifyCancellation<bposit64>(reportTestCases),           "bposit64",       "exact cancellation");
#endif
#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyRandomDotProducts<bposit64>(200, reportTestCases), "bposit64",       "random dot products");
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
