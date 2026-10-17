// correct_rounding.cpp: the lindex math library is correctly rounded (#1696)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Every result must EQUAL the exact function value rounded once into the lindex, to nearest
// with ties to even on the psi grid, saturating at maxpos: the encodings of
// tools/generators/lindex_math_gen.py, from mpmath at 600 bits.  The comparison is of
// encodings, never through double, which cannot hold a level-5 value and measures error in
// the wrong domain.
//
//   lindex<8,3>   every encoding, for each unary function; pow for every one of the 65536
//                 pairs; hypot for every pair (from the table of magnitudes)
//   sampled       lindex<16,3> and <32,3> (double, with the dd_cascade retry) and <64,3>
//                 (dd_cascade): every level, the level boundaries, and both signs
//
// exp and log of a value >= 1 must also be exact steps of one level, which is asserted on
// the encodings directly.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <functional>
#include <string>
#include <universal/number/lindex/lindex.hpp>
#include <universal/verification/lindex_math_vectors.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;
namespace lv = sw::universal::lindex_verification;

int hexdigit(char c) { return (c <= '9') ? (c - '0') : (c - 'a' + 10); }
std::uint64_t entry(const char* row, std::uint64_t i) {
	return static_cast<std::uint64_t>(hexdigit(row[2 * i]) * 16 + hexdigit(row[2 * i + 1]));
}

// the unary functions, in lv::math_unary_names order
template<typename L>
L unary(unsigned f, const L& x) {
	switch (f) {
	case 0:  return exp(x);
	case 1:  return exp2(x);
	case 2:  return exp10(x);
	case 3:  return expm1(x);
	case 4:  return log(x);
	case 5:  return log2(x);
	case 6:  return log10(x);
	case 7:  return log1p(x);
	case 8:  return sqrt(x);
	case 9:  return cbrt(x);
	case 10: return floor(x);
	case 11: return ceil(x);
	case 12: return trunc(x);
	default: return round(x);
	}
}

struct Tally {
	int fails = 0;
	void judge(bool ok, const std::string& what, bool report) {
		if (ok) return;
		if (report && fails < 20) std::cerr << "FAIL: " << what << '\n';
		++fails;
	}
};

template<typename L>
std::string describe(const char* name, const L& x, const L& got, std::uint64_t want) {
	std::stringstream s;
	s << type_tag(x) << ' ' << name << '(' << to_hex(x) << ") = " << to_hex(got) << ", want 0x" << std::hex << want;
	return s.str();
}

int VerifyUnary8(unsigned f, bool report) {
	using L = lindex<8, 3>;
	static_assert(lv::math_nr_unary == 14, "the unary table changed");
	Tally t;
	for (std::uint64_t e = 0; e < 256; ++e) {
		L x;
		x.setbits(e);
		const L got = unary(f, x);
		const std::uint64_t want = entry(lv::lindex8_unary[f], e);
		t.judge(got.raw_bits() == want, describe(lv::math_unary_names[f], x, got, want), report);
	}
	return t.fails;
}

int VerifyPow8(bool report) {
	using L = lindex<8, 3>;
	Tally t;
	for (std::uint64_t i = 0; i < 256; ++i) {
		L x;
		x.setbits(i);
		for (std::uint64_t j = 0; j < 256; ++j) {
			L y;
			y.setbits(j);
			const L got = pow(x, y);
			const std::uint64_t want = entry(lv::lindex8_pow[i], j);
			if (got.raw_bits() != want) t.judge(false, describe("pow", x, got, want) + " y " + to_hex(y), report);
		}
	}
	return t.fails;
}

int VerifyHypot8(bool report) {
	using L = lindex<8, 3>;
	Tally t;
	for (std::uint64_t i = 0; i < 256; ++i) {
		L x;
		x.setbits(i);
		for (std::uint64_t j = 0; j < 256; ++j) {
			L y;
			y.setbits(j);
			const L got = hypot(x, y);
			const std::uint64_t want = (x.isnar() || y.isnar()) ? 0x80 : entry(lv::lindex8_hypot[x.magnitude_bits()], y.magnitude_bits());
			if (got.raw_bits() != want) t.judge(false, describe("hypot", x, got, want) + " y " + to_hex(y), report);
		}
	}
	return t.fails;
}

template<unsigned nbits, unsigned lbits, std::size_t N, std::size_t M>
int VerifySampled(const lv::MathUnaryVector (&unaries)[N], const lv::MathBinaryVector (&binaries)[M], bool report) {
	using L = lindex<nbits, lbits>;
	Tally t;
	for (const auto& v : unaries) {
		L x;
		x.setbits(v.x);
		for (unsigned f = 0; f < lv::math_nr_unary; ++f) {
			const L got = unary(f, x);
			t.judge(got.raw_bits() == v.f[f], describe(lv::math_unary_names[f], x, got, v.f[f]), report);
		}
	}
	for (const auto& v : binaries) {
		L x, y;
		x.setbits(v.x);
		y.setbits(v.y);
		const L p = pow(x, y), h = hypot(x, y);
		t.judge(p.raw_bits() == v.pow, describe("pow", x, p, v.pow) + " y " + to_hex(y), report);
		t.judge(h.raw_bits() == v.hypot, describe("hypot", x, h, v.hypot) + " y " + to_hex(y), report);
	}
	return t.fails;
}

// exp and log of a value >= 1 are exact steps of one level on the encoding, at every level
template<unsigned nbits, unsigned lbits>
int VerifyExactShifts(bool report) {
	using L = lindex<nbits, lbits>;
	constexpr std::uint64_t one = L::one_bits, top = L::maxpos_bits;
	constexpr std::uint64_t step = (nbits <= 12) ? 1ull : (top / 4093ull);
	Tally t;
	for (std::uint64_t m = 0; m <= top; m += step) {
		L x;
		x.setbits(m);
		const std::uint64_t up = (m > top - one) ? top : m + one;
		t.judge(exp(x).raw_bits() == up, describe("exp", x, exp(x), up), report);
		if (m >= one) t.judge(log(x).raw_bits() == m - one, describe("log", x, log(x), m - one), report);
		if (m >= one && m <= top - one) t.judge(log(exp(x)) == x && exp(log(x)) == x, describe("log(exp)", x, log(exp(x)), m), report);
	}
	return t.fails;
}

// classification, min/max and nextafter on the encoding
int VerifyEncodingFunctions(bool report) {
	using L = lindex<16, 3>;
	Tally t;
	const L nar(SpecificValue::nar), zero(0), one(1.0), two(2.0), maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
	t.judge(isnan(nar) && !isnan(one) && !isinf(maxpos) && isfinite(maxpos) && !isfinite(nar), "isnan, isinf, isfinite", report);
	t.judge(isnormal(one) && !isnormal(zero) && !isnormal(nar) && signbit(-one) && !signbit(one), "isnormal, signbit", report);
	t.judge(fpclassify(nar) == FP_NAN && fpclassify(zero) == FP_ZERO && fpclassify(L(SpecificValue::minpos)) == FP_NORMAL, "fpclassify", report);
	t.judge(min(one, two) == one && max(one, two) == two && min(nar, one).isnar(), "min, max", report);
	t.judge(fmin(nar, one) == one && fmax(two, nar) == two && fmin(-two, one) == -two, "fmin, fmax", report);
	L up(one);
	++up;
	t.judge(nextafter(one, two) == up && nextafter(up, zero) == one && nextafter(one, one) == one, "nextafter", report);
	t.judge(nextafter(maxpos, maxpos) == maxpos && nextafter(maxneg, nar).isnar(), "nextafter at the ends", report);
	// the special values of the functions
	t.judge(log(zero).isnar() && log(-one).isnar() && sqrt(-one).isnar() && log1p(-one).isnar(), "domain errors are NaR", report);
	t.judge(pow(zero, -one).isnar() && pow(-two, L(0.5)).isnar() && pow(-two, one) == -two && pow(nar, zero).isnar(), "pow special cases", report);
	t.judge(exp(maxpos) == maxpos && exp(zero) == one && log(one).iszero() && expm1(maxpos) == maxpos, "exp, log at the ends", report);
	return t.fails;
}

}  // anonymous namespace

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

	std::string test_suite  = "lindex math library, correct rounding (#1696)";
	std::string test_tag    = "correct_rounding";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyUnary8(8, reportTestCases), "lindex<8,3>", "sqrt");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	for (unsigned f = 0; f < lv::math_nr_unary; ++f) {
		nrOfFailedTestCases += ReportTestResult(VerifyUnary8(f, reportTestCases), "lindex<8,3>", std::string(lv::math_unary_names[f]) + ", every encoding");
	}
	nrOfFailedTestCases += ReportTestResult(VerifyPow8(reportTestCases), "lindex<8,3>", "pow, every pair");
	nrOfFailedTestCases += ReportTestResult(VerifyHypot8(reportTestCases), "lindex<8,3>", "hypot, every pair");
	nrOfFailedTestCases += ReportTestResult((VerifySampled<16, 3>(lv::math_unary_16_3, lv::math_binary_16_3, reportTestCases)), "lindex<16,3>", "math library, sampled");
	nrOfFailedTestCases += ReportTestResult((VerifySampled<32, 3>(lv::math_unary_32_3, lv::math_binary_32_3, reportTestCases)), "lindex<32,3>", "math library, sampled");
	nrOfFailedTestCases += ReportTestResult((VerifySampled<64, 3>(lv::math_unary_64_3, lv::math_binary_64_3, reportTestCases)), "lindex<64,3>", "math library, sampled");
	nrOfFailedTestCases += ReportTestResult((VerifyExactShifts<8, 3>(reportTestCases)), "lindex<8,3>", "exp/log exact shifts, every encoding");
	nrOfFailedTestCases += ReportTestResult((VerifyExactShifts<16, 3>(reportTestCases)), "lindex<16,3>", "exp/log exact shifts, sampled");
	nrOfFailedTestCases += ReportTestResult((VerifyExactShifts<64, 3>(reportTestCases)), "lindex<64,3>", "exp/log exact shifts, sampled");
	nrOfFailedTestCases += ReportTestResult(VerifyEncodingFunctions(reportTestCases), "lindex<16,3>", "classify, minmax, next, special values");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult((VerifyExactShifts<12, 3>(reportTestCases)), "lindex<12,3>", "exp/log exact shifts, every encoding");
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
