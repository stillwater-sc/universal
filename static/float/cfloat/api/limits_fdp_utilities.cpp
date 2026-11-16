// limits_fdp_utilities.cpp: cfloat numeric_limits, fused dot products, polyroot and the text utilities
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The findings of #2089:
//   - numeric_limits::max_exponent was one short for layouts with max-exponent values, is_bounded was
//     false, is_iec559 was false for the IEEE layouts, and epsilon was 0 where a layout without
//     subnormals cannot hold 2^-fbits.  The reference is the layout's parameters,
//     and std::numeric_limits<float> / <double> for the layouts that are float and double.
//   - fdp with one fraction bit ORed its sticky bit into the guard bit: cfloat<4,2> 0.5*0.5 + 3 was
//     inf.  The reference is the exact dot product, in double, rounded once.
//   - fdp_qc / fdp_stride / fdp relied on asserts: under NDEBUG a zero stride re-read x[0] and an
//     index past a vector read out of bounds.  They give NaN in every build.
//   - polyroot did not compile, and read out of bounds for a constant or empty polynomial.
//   - to_decimal_fixpnt_string printed inf and NaN as numbers; color_print put its exponent nibble
//     markers in the wrong places.
#include <universal/utility/directives.hpp>
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/cfloat/mathext/horners.hpp>
#include <universal/verification/test_suite.hpp>

#include <cfloat>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

namespace {

// numeric_limits against the layout's parameters
template<typename C>
int VerifyLimits(bool reportTestCases) {
	using L = std::numeric_limits<C>;
	constexpr int fbits = static_cast<int>(C::fbits);
	constexpr int bias = (1 << (C::es - 1)) - 1;
	// the largest finite exponent field: all ones with max-exponent values, one less without
	constexpr int topField = (1 << C::es) - (C::hasMaxExpValues ? 1 : 2);
	constexpr int maxScale = topField - bias;
	const double maxpos = double(L::max());
	int fails = 0;
	auto check = [&](bool ok, const char* what) {
		if (!ok) {
			if (reportTestCases) std::cerr << "FAIL: " << type_tag(C()) << ' ' << what << '\n';
			++fails;
		}
	};
	check(L::max_exponent == maxScale + 1, "max_exponent");
	check(L::max_exponent10 == static_cast<int>(std::floor(std::log10(maxpos))), "max_exponent10");
	check(L::min_exponent == 1 - bias + 1, "min_exponent");
	check(L::is_bounded, "is_bounded");
	// IEEE: subnormals, inf and NaN in the top binade, and a quiet bit, which needs two fraction bits
	check(L::is_iec559 == (C::hasSubnormals && !C::hasMaxExpValues && !C::isSaturating && C::fbits >= 2), "is_iec559");
	check(L::round_style == std::round_to_nearest, "round_style");
	// 2^-fbits, or min() where a layout without subnormals cannot hold it
	const bool epsRepresentable = C::hasSubnormals || -fbits >= 1 - bias;
	check(double(L::epsilon()) == (epsRepresentable ? std::ldexp(1.0, -fbits) : std::ldexp(1.0, 1 - bias)), "epsilon");
	check(double(L::min()) == std::ldexp(1.0, 1 - bias), "min");
	check(double(L::denorm_min()) == (C::hasSubnormals ? std::ldexp(1.0, 1 - bias - fbits) : std::ldexp(1.0, 1 - bias)), "denorm_min");
	check(double(L::round_error()) == 0.5, "round_error");
	return fails;
}

// the layouts that are float and double report what float and double do
template<typename C, typename Native>
int VerifyLimitsAgainstNative(bool reportTestCases) {
	using L = std::numeric_limits<C>;
	using N = std::numeric_limits<Native>;
	const bool ok = L::digits == N::digits && L::max_exponent == N::max_exponent && L::max_exponent10 == N::max_exponent10
	             && L::min_exponent == N::min_exponent && L::min_exponent10 == N::min_exponent10
	             && L::is_iec559 == N::is_iec559 && L::is_bounded == N::is_bounded && L::round_style == N::round_style
	             && double(L::max()) == double(N::max()) && double(L::min()) == double(N::min())
	             && double(L::epsilon()) == double(N::epsilon()) && double(L::denorm_min()) == double(N::denorm_min());
	if (!ok && reportTestCases) std::cerr << "FAIL: " << type_tag(C()) << " numeric_limits differ from the native type's\n";
	return ok ? 0 : 1;
}

// every x*y + z of a small layout through fdp, against the exact value rounded once
template<typename C>
int VerifyFdpSmall(bool reportTestCases) {
	static_assert(C::nbits <= 6, "exhaustive over three operands");
	int fails = 0;
	const unsigned N = 1u << C::nbits;
	C x, y, z;
	for (unsigned a = 0; a < N; ++a) for (unsigned b = 0; b < N; ++b) for (unsigned c = 0; c < N; ++c) {
		x.setbits(a); y.setbits(b); z.setbits(c);
		if (x.isnan() || y.isnan() || z.isnan() || x.isinf() || y.isinf() || z.isinf()) continue;
		const std::vector<C> u = { x, z }, v = { y, C(1) };
		const C ref(double(x) * double(y) + double(z));   // exact in double, rounded once here
		const C got = fdp(u, v);
		if (!(got == ref) && !(got.iszero() && ref.iszero())) {
			if (reportTestCases && fails < 10) std::cerr << "FAIL: " << type_tag(C()) << ' ' << double(x) << " * " << double(y) << " + " << double(z)
			                                             << " = " << double(got) << ", expected " << double(ref) << '\n';
			++fails;
		}
	}
	return fails;
}

// a wide layout: integer vectors, whose products and sums are exact in double
template<typename C>
int VerifyFdpWide(unsigned samples, bool reportTestCases) {
	int fails = 0;
	std::mt19937_64 rng(2089);
	for (unsigned k = 0; k < samples; ++k) {
		const std::size_t n = 1 + rng() % 16u;
		std::vector<C> x(n), y(n);
		double exact = 0.0;
		for (std::size_t i = 0; i < n; ++i) {
			const long long a = static_cast<long long>(rng() >> 44) - (1ll << 19), b = static_cast<long long>(rng() >> 44) - (1ll << 19);
			x[i] = C(a); y[i] = C(b);
			exact += double(a) * double(b);   // < 2^45 in magnitude: exact
		}
		const C got = fdp(x, y);
		if (double(got) != exact) {
			if (reportTestCases && fails < 10) std::cerr << "FAIL: " << type_tag(C()) << " fdp = " << double(got) << ", expected " << exact << '\n';
			++fails;
		}
	}
	return fails;
}

// strides (n is the index bound) and the guards, which hold in every build
template<typename C>
int VerifyFdpStridesAndGuards(bool reportTestCases) {
	int fails = 0;
	auto check = [&](bool ok, const char* what) {
		if (!ok) {
			if (reportTestCases) std::cerr << "FAIL: " << what << '\n';
			++fails;
		}
	};
	const std::vector<C> x = { C(1), C(2), C(3), C(4), C(5), C(6) };
	const std::vector<C> y = { C(10), C(20), C(30), C(40), C(50), C(60) };
	const std::vector<C> shortY = { C(1), C(2) };
	// k*2 < 6 and k*3 < 6: k = 0, 1, so x[0]*y[0] + x[2]*y[3] = 10 + 120
	check(double(fdp_stride(6, x, 2, y, 3)) == 130.0, "fdp_stride index bound");
	check(double(fdp_stride(5, x, 1, y, 1)) == 10.0 + 40.0 + 90.0 + 160.0 + 250.0, "fdp_stride unit stride");
	check(fdp_stride(4, x, 0, y, 1).isnan(), "fdp_stride zero incx");
	check(fdp_stride(4, x, 1, y, 0).isnan(), "fdp_stride zero incy");
	check(fdp_stride(8, x, 1, y, 1).isnan(), "fdp_stride index past the vectors");
	check(fdp(x, shortY).isnan(), "fdp short y");
	check(double(fdp_stride(0, x, 0, y, 0)) == 0.0, "fdp_stride n = 0");
	sw::universal::quire<C> q;
	fdp_qc(q, 4, x, 0, y, 1);
	check(quire_resolve(q).isnan(), "fdp_qc zero stride");
	sw::universal::quire<C> q2;
	fdp_qc(q2, 8, x, 1, y, 1);
	check(quire_resolve(q2).isnan(), "fdp_qc index past the vectors");
	return fails;
}

// polyroot compiles, finds a root, and handles constant and empty polynomials
int VerifyPolyroot(bool reportTestCases) {
	using C = sw::universal::cfloat<32, 8, uint32_t, true, false, false>;
	int fails = 0;
	const std::vector<C> p = { C(-2), C(0), C(1) };                       // x^2 - 2
	const C r = sw::universal::polyroot(p, C(1), 50, 0.0);
	if (!std::isfinite(double(r)) || std::fabs(double(r) - std::sqrt(2.0)) > 1e-6) ++fails;   // a NaN root fails
	if (!sw::universal::polyroot(std::vector<C>{ C(3) }, C(1), 50, 0.0).isnan()) ++fails;     // no root
	if (!(sw::universal::polyroot(std::vector<C>{ C(0) }, C(1.5), 50, 0.0) == C(1.5))) ++fails;   // every x
	if (!sw::universal::polyroot(std::vector<C>{}, C(1), 50, 0.0).isnan()) ++fails;
	if (fails && reportTestCases) std::cerr << "FAIL: polyroot\n";
	return fails;
}

// to_decimal_fixpnt_string of the special values, and color_print's nibble markers
int VerifyText(bool reportTestCases) {
	using sw::universal::SpecificValue;
	using H = sw::universal::cfloat<16, 5, uint16_t, true, false, false>;
	int fails = 0;
	if (to_decimal_fixpnt_string(H(SpecificValue::infpos), 4) != "inf") ++fails;
	if (to_decimal_fixpnt_string(H(SpecificValue::infneg), 4) != "-inf") ++fails;
	if (to_decimal_fixpnt_string(H(SpecificValue::qnan), 4) != "nan") ++fails;
	if (to_decimal_fixpnt_string(H(SpecificValue::snan), 4) != "snan") ++fails;
	// color_print, its colour codes removed, is to_binary without the "0b" and the field dots
	using W = sw::universal::cfloat<32, 11, uint32_t, true, false, false>;   // es = 11: markers in the exponent
	for (const double v : { 1.5, -3.25, 1e-300, 7e300 }) {
		const W c(v);
		std::string colored = color_print(c, true), plain;
		for (std::size_t i = 0; i < colored.size(); ++i) {
			if (colored[i] == '\x1b') { while (i < colored.size() && colored[i] != 'm') ++i; continue; }
			plain += colored[i];
		}
		std::string bin = to_binary(c, true), expected;
		for (std::size_t i = 2; i < bin.size(); ++i) if (bin[i] != '.') expected += bin[i];
		if (plain != expected) {
			if (reportTestCases) std::cerr << "FAIL: color_print " << plain << " vs to_binary " << expected << '\n';
			++fails;
		}
	}
	if (fails && reportTestCases) std::cerr << "FAIL: text utilities\n";
	return fails;
}

}  // namespace

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

	std::string test_suite  = "cfloat limits, fdp and utilities";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	std::cout << "max_exponent of cfloat<8,4,uint8_t,t,t,f>: " << std::numeric_limits<cfloat<8, 4, uint8_t, true, true, false>>::max_exponent << '\n';
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<cfloat<8, 4, uint8_t, true, true, false>>(reportTestCases),    "cfloat<8,4,t,t,f>",   "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<cfloat<8, 4, uint8_t, true, true, true>>(reportTestCases),     "cfloat<8,4,t,t,t>",   "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<cfloat<12, 4, uint8_t, false, true, true>>(reportTestCases),   "cfloat<12,4,f,t,t>",  "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<cfloat<16, 5, uint16_t, true, false, false>>(reportTestCases), "cfloat<16,5,t,f,f>",  "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<cfloat<16, 5, uint16_t, false, false, false>>(reportTestCases), "cfloat<16,5,f,f,f>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<cfloat<16, 8, uint16_t, true, true, false>>(reportTestCases),  "cfloat<16,8,t,t,f>",  "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLimits<cfloat<4, 2, uint8_t, true, false, false>>(reportTestCases),   "cfloat<4,2,t,f,f>",   "numeric_limits");   // one fraction bit: not IEEE
	nrOfFailedTestCases += ReportTestResult((VerifyLimitsAgainstNative<cfloat<32, 8, uint32_t, true, false, false>, float>(reportTestCases)),   "cfloat<32,8,t,f,f>",  "vs float");
	nrOfFailedTestCases += ReportTestResult((VerifyLimitsAgainstNative<cfloat<64, 11, uint64_t, true, false, false>, double>(reportTestCases)), "cfloat<64,11,t,f,f>", "vs double");

	nrOfFailedTestCases += ReportTestResult(VerifyFdpSmall<cfloat<4, 2, uint8_t, true, false, false>>(reportTestCases), "cfloat<4,2,t,f,f>", "fdp small");
	nrOfFailedTestCases += ReportTestResult(VerifyFdpSmall<cfloat<5, 3, uint8_t, true, false, false>>(reportTestCases), "cfloat<5,3,t,f,f>", "fdp small");
	nrOfFailedTestCases += ReportTestResult(VerifyFdpSmall<cfloat<6, 3, uint8_t, true, false, false>>(reportTestCases), "cfloat<6,3,t,f,f>", "fdp small");
	nrOfFailedTestCases += ReportTestResult(VerifyFdpSmall<cfloat<6, 2, uint8_t, true, true, true>>(reportTestCases),   "cfloat<6,2,t,t,t>", "fdp small");
	nrOfFailedTestCases += ReportTestResult(VerifyFdpWide<cfloat<80, 11, uint32_t, true, false, false>>(500, reportTestCases), "cfloat<80,11,t,f,f>", "fdp wide");
	nrOfFailedTestCases += ReportTestResult(VerifyFdpStridesAndGuards<cfloat<32, 8, uint32_t, true, false, false>>(reportTestCases), "cfloat<32,8,t,f,f>", "fdp strides/guards");

	nrOfFailedTestCases += ReportTestResult(VerifyPolyroot(reportTestCases), "cfloat<32,8>", "polyroot");
	nrOfFailedTestCases += ReportTestResult(VerifyText(reportTestCases),     "cfloat",       "text utilities");
#endif

#if REGRESSION_LEVEL_2
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
