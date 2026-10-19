// functions.cpp: the trigonometric, hyperbolic, error and gamma functions, atan2, fmod and
// remainder of lindex are correctly rounded (#1715)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Every result must EQUAL the exact function value rounded once into the lindex, to nearest
// with ties to even on the psi grid, saturating at maxpos: the encodings of
// tools/generators/lindex_math2_gen.py, from mpmath, with the domain decisions that
// math/*.hpp document (NaR past the double range where the result depends on X mod 2 pi or
// X mod Y, at the poles, and outside the domain).
//
//   lindex<8,3>   every encoding, for each unary function; atan2 for every one of the 65536
//                 pairs, and fmod and remainder for every fifth (every pair at level 2)
//   sampled       lindex<16,3> and <32,3> (double, with the dd_cascade retry) and <64,3>
//                 (dd_cascade): every level, the level boundaries, the double range edge
//                 and both signs
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <string>
#include <universal/number/lindex/lindex.hpp>
#include <universal/verification/lindex_math2_vectors.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;
namespace lv = sw::universal::lindex_verification;

int hexdigit(char c) { return (c <= '9') ? (c - '0') : (c - 'a' + 10); }
std::uint64_t entry(const char* row, std::uint64_t i) {
	return static_cast<std::uint64_t>(hexdigit(row[2 * i]) * 16 + hexdigit(row[2 * i + 1]));
}

// the unary functions, in lv::math2_unary_names order
template<typename L>
L unary(unsigned f, const L& x) {
	switch (f) {
	case 0:  return sin(x);
	case 1:  return cos(x);
	case 2:  return tan(x);
	case 3:  return asin(x);
	case 4:  return acos(x);
	case 5:  return atan(x);
	case 6:  return sinh(x);
	case 7:  return cosh(x);
	case 8:  return tanh(x);
	case 9:  return asinh(x);
	case 10: return acosh(x);
	case 11: return atanh(x);
	case 12: return erf(x);
	case 13: return erfc(x);
	case 14: return tgamma(x);
	default: return lgamma(x);
	}
}

// the binary functions, in lv::math2_binary_names order
template<typename L>
L binary(unsigned f, const L& x, const L& y) {
	switch (f) {
	case 0:  return atan2(x, y);
	case 1:  return fmod(x, y);
	default: return remainder(x, y);
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
	static_assert(lv::math2_nr_unary == 16, "the unary table changed");
	Tally t;
	for (std::uint64_t e = 0; e < 256; ++e) {
		L x;
		x.setbits(e);
		const L got = unary(f, x);
		const std::uint64_t want = entry(lv::lindex8_unary2[f], e);
		t.judge(got.raw_bits() == want, describe(lv::math2_unary_names[f], x, got, want), report);
	}
	return t.fails;
}

// every pair, or every stride-th pair (stride coprime to 256, so every row and column is hit)
int VerifyBinary8(unsigned f, const char* const (&table)[256], bool report, std::uint64_t stride = 1) {
	using L = lindex<8, 3>;
	Tally t;
	for (std::uint64_t i = 0; i < 256; ++i) {
		L x;
		x.setbits(i);
		for (std::uint64_t j = (i % stride); j < 256; j += stride) {
			L y;
			y.setbits(j);
			const L got = binary(f, x, y);
			const std::uint64_t want = entry(table[i], j);
			if (got.raw_bits() != want) t.judge(false, describe(lv::math2_binary_names[f], x, got, want) + " y " + to_hex(y), report);
		}
	}
	return t.fails;
}

template<unsigned nbits, unsigned lbits, std::size_t N, std::size_t M>
int VerifySampled(const lv::Math2UnaryVector (&unaries)[N], const lv::Math2BinaryVector (&binaries)[M], bool report) {
	using L = lindex<nbits, lbits>;
	Tally t;
	for (const auto& v : unaries) {
		L x;
		x.setbits(v.x);
		for (unsigned f = 0; f < lv::math2_nr_unary; ++f) {
			const L got = unary(f, x);
			t.judge(got.raw_bits() == v.f[f], describe(lv::math2_unary_names[f], x, got, v.f[f]), report);
		}
	}
	for (const auto& v : binaries) {
		L x, y;
		x.setbits(v.x);
		y.setbits(v.y);
		for (unsigned f = 0; f < lv::math2_nr_binary; ++f) {
			const L got = binary(f, x, y);
			t.judge(got.raw_bits() == v.f[f], describe(lv::math2_binary_names[f], x, got, v.f[f]) + " y " + to_hex(y), report);
		}
	}
	return t.fails;
}

// the domain decisions, on values whose results the header comments state
int VerifyDomains(bool report) {
	using L = lindex<32, 3>;
	Tally t;
	const L nar(SpecificValue::nar), zero(0), one(1), two(2), maxpos(SpecificValue::maxpos);
	const L big(1.0e300), past(maxpos);   // within, and far past, the double range
	t.judge(!sin(big).isnar() && sin(past).isnar() && cos(past).isnar() && tan(past).isnar(), "sin/cos/tan NaR past the double range only", report);
	t.judge(asin(two).isnar() && acos(-two).isnar() && acosh(L(0.5)).isnar() && atanh(one).isnar() && atanh(-two).isnar(), "inverse domains are NaR", report);
	t.judge(lgamma(zero).isnar() && tgamma(-one).isnar() && lgamma(-past).isnar() && !lgamma(past).isnar(), "gamma poles, and negative X past the range", report);
	t.judge(fmod(one, zero).isnar() && fmod(past, two).isnar() && fmod(two, past) == two && remainder(past, past).iszero() == false, "fmod domain", report);
	t.judge(cos(zero) == one && sin(zero).iszero() && cosh(zero) == one && erfc(zero) == one && tanh(past) == one && erf(-past) == -one, "special values", report);
	t.judge(sin(nar).isnar() && atanh(nar).isnar() && lgamma(nar).isnar() && erf(nar).isnar() && fmod(nar, one).isnar() && atan2(one, nar).isnar() && remainder(one, nar).isnar(), "NaR propagates", report);
	t.judge(atan2(zero, -one) == atan2(zero, -two) && atan2(zero, one).iszero() && atan2(one, zero) == -atan2(-one, zero), "atan2 on the axes", report);
	return t.fails;
}

// The exact path's widening (value_domain.hpp, round_exact): a value 2^-100 grid units above a
// midpoint, whose evaluation at b bits errs by 2^-(b - 8) below it, rounds down until the two
// evaluations agree closely enough to decide, and then up.  No function result in the vectors
// comes that close to a midpoint, so this is the only test of the loop.
int VerifyWidening(bool report) {
	using L = lindex<16, 3>;
	using E = lindex_math::exact_float;
	Tally t;
	unsigned calls = 0, widest = 0;
	const L got = lindex_math::round_exact<L>([&](unsigned bits) {
		++calls;
		if (bits > widest) widest = bits;
		// (2k + 1) / 2 + 2^-100 on the grid, k = 1000, less the evaluation error
		E v(2001);
		v.set_precision(bits + 256u);
		v.setexponent(v.scale() - static_cast<std::int64_t>(L::fbits) - 1);
		E nudge(1);
		nudge.set_precision(bits + 256u);
		nudge.setexponent(-static_cast<std::int64_t>(L::fbits) - 100);
		E error(1);
		error.set_precision(bits + 256u);
		error.setexponent(-static_cast<std::int64_t>(bits) + 8);
		return v + nudge - error;
	}, L::fbits + 64u);
	t.judge(got.raw_bits() == 1001u, "round_exact widened to decide a value 2^-100 above a midpoint: got " + to_hex(got) + ", want 0x3E9", report);
	t.judge(calls > 2 && widest > L::fbits + 100u, "round_exact widened past the first attempt", report);
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

	std::string test_suite  = "lindex math library: trigonometric, hyperbolic, error and gamma functions, fmod and remainder (#1715)";
	std::string test_tag    = "functions";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyUnary8(0, reportTestCases), "lindex<8,3>", "sin");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	for (unsigned f = 0; f < lv::math2_nr_unary; ++f) {
		nrOfFailedTestCases += ReportTestResult(VerifyUnary8(f, reportTestCases), "lindex<8,3>", std::string(lv::math2_unary_names[f]) + ", every encoding");
	}
	nrOfFailedTestCases += ReportTestResult(VerifyBinary8(0, lv::lindex8_atan2, reportTestCases), "lindex<8,3>", "atan2, every pair");
	// fmod and remainder form X to 1024 + fbits bits in efloat: every fifth pair here, all at level 2
	nrOfFailedTestCases += ReportTestResult(VerifyBinary8(1, lv::lindex8_fmod, reportTestCases, 5), "lindex<8,3>", "fmod, every fifth pair");
	nrOfFailedTestCases += ReportTestResult(VerifyBinary8(2, lv::lindex8_remainder, reportTestCases, 5), "lindex<8,3>", "remainder, every fifth pair");
	nrOfFailedTestCases += ReportTestResult((VerifySampled<16, 3>(lv::math2_unary_16_3, lv::math2_binary_16_3, reportTestCases)), "lindex<16,3>", "functions, sampled");
	nrOfFailedTestCases += ReportTestResult((VerifySampled<32, 3>(lv::math2_unary_32_3, lv::math2_binary_32_3, reportTestCases)), "lindex<32,3>", "functions, sampled");
	nrOfFailedTestCases += ReportTestResult((VerifySampled<64, 3>(lv::math2_unary_64_3, lv::math2_binary_64_3, reportTestCases)), "lindex<64,3>", "functions, sampled");
	nrOfFailedTestCases += ReportTestResult(VerifyDomains(reportTestCases), "lindex<32,3>", "domains and special values");
	nrOfFailedTestCases += ReportTestResult(VerifyWidening(reportTestCases), "lindex<16,3>", "exact path widens until decided");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyBinary8(1, lv::lindex8_fmod, reportTestCases), "lindex<8,3>", "fmod, every pair");
	nrOfFailedTestCases += ReportTestResult(VerifyBinary8(2, lv::lindex8_remainder, reportTestCases), "lindex<8,3>", "remainder, every pair");
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
