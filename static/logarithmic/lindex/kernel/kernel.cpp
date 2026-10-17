// kernel.cpp: accuracy of the level-index kernel against the mpmath oracle (#1349 part 2, #1693)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The kernel (number/shared/level_index_kernel.hpp) adds and subtracts in the psi domain.
// Its error is measured the way docs/level-index-design.md requires of every level-index
// test: |psi(computed) - psi(exact)| in units of 2^-fbits, never in the value domain, where
// one psi-ulp at the top of level 4 spans a factor of 10^16568.  psi(exact) comes from
// tools/generators/level_index_gen.py, which runs the plain Clenshaw-Olver sequences at
// 600 bits (and checks them against direct evaluation wherever a value is computable);
// the kernel carries subtraction in the expm1 form, so the two do not share a formulation.
//
// The pairs cover every level 0..7, near cancellation, operands straddling a level
// boundary, operands on one, and x - x.  Two working types, as the design doc's working-
// precision table prescribes: double for fbits 28 (lindex<32,3>) and dd_cascade for
// fbits 60 (lindex<64,3>).  double is also run against fbits 60 and only reported: it is
// the evidence that the wider formats need dd_cascade.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/shared/level_index_kernel.hpp>
#include <universal/verification/level_index_vectors.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using sw::universal::dd_cascade;
namespace li = sw::universal::level_index;
namespace lv = sw::universal::level_index_verification;

constexpr int LEVELS = 8;

// the encoding's psi as a kernel point: level from the integer part, index exactly
template<typename W>
li::point<W> to_point(std::uint64_t enc, int fbits) {
	const std::uint64_t mask = (std::uint64_t(1) << fbits) - 1;
	const std::uint64_t r = enc & mask;
	// two 30-bit halves, so the index is exact in dd_cascade; double rounds past 53 bits
	const W hi = W(static_cast<double>(r >> 30)) * W(std::ldexp(1.0, 30 - fbits));
	const W lo = W(static_cast<double>(r & ((std::uint64_t(1) << 30) - 1))) * W(std::ldexp(1.0, -fbits));
	return { static_cast<int>(enc >> fbits), W(hi + lo) };
}

// psi(computed) - psi(exact), carried in dd_cascade so the difference is not rounded away
template<typename W>
dd_cascade error_of(const li::point<W>& z, const double exact[3]) {
	dd_cascade e = dd_cascade(static_cast<double>(z.level)) + dd_cascade(z.index);
	return ((e - dd_cascade(exact[0])) - dd_cascade(exact[1])) - dd_cascade(exact[2]);
}

// the error in psi-ulps of a format with fbits index bits
double in_ulps(const dd_cascade& e, int fbits) { return std::abs(double(e)) * std::ldexp(1.0, fbits); }

struct Stats {
	double worst[2][LEVELS] = {};   // [add, sub][level of x]
	int not_faithful = 0;           // rounded result is neither neighbour of the exact psi
	int not_nearest = 0;            // rounded result differs from the correctly rounded one
	int nonzero_cancel = 0;         // x - x that did not come back exactly 0
	int count = 0;
};

template<typename W, std::size_t N>
Stats measure(const lv::ArithmeticVector (&vectors)[N], int fbits) {
	Stats s;
	for (const auto& v : vectors) {
		const li::point<W> x = to_point<W>(v.x, fbits), y = to_point<W>(v.y, fbits);
		const int level = static_cast<int>(v.x >> fbits);
		for (int op = 0; op < 2; ++op) {
			const li::point<W> z = (op == 0) ? li::add(x, y) : li::sub(x, y);
			const double* exact = (op == 0) ? v.sum : v.difference;
			const double err = in_ulps(error_of(z, exact), fbits);
			s.worst[op][level] = std::max(s.worst[op][level], err);
			if (op == 1 && v.x == v.y && !(z.level == 0 && z.index == W(0))) ++s.nonzero_cancel;
			// Round the kernel's psi to the encoding grid and compare it with the exact psi.
			// psi * 2^fbits reaches 2^63 at fbits 60, past double's 53 bits, so this stays in
			// dd_cascade, where such an integer and its residual are exact.  r is the exact
			// psi minus the chosen encoding, in psi-ulps: faithful when |r| < 1, correctly
			// rounded when |r| <= 1/2 (an exact tie may go either way).
			const dd_cascade scale(std::ldexp(1.0, fbits));
			const dd_cascade E = (dd_cascade(exact[0]) + dd_cascade(exact[1]) + dd_cascade(exact[2])) * scale;
			const dd_cascade G = floor((dd_cascade(static_cast<double>(z.level)) + dd_cascade(z.index)) * scale + dd_cascade(0.5));
			const double r = std::abs(double(E - G));
			if (!(r < 1.0)) ++s.not_faithful;
			if (!(r <= 0.5)) ++s.not_nearest;
			++s.count;
		}
	}
	return s;
}

double max_over_levels(const Stats& s, int op) { return *std::max_element(s.worst[op], s.worst[op] + LEVELS); }

void report(const std::string& title, const Stats& s) {
	std::cout << title << ": max error in psi-ulps by the level of x\n";
	std::cout << "  level        add            sub\n";
	for (int l = 0; l < LEVELS; ++l) {
		std::cout << "    " << l << "  " << std::setw(13) << std::setprecision(3) << s.worst[0][l]
		          << "  " << std::setw(13) << s.worst[1][l] << '\n';
	}
	std::cout << "  " << s.count << " results, " << s.not_faithful << " not faithful, " << s.not_nearest
	          << " not correctly rounded, " << s.nonzero_cancel << " x - x not exactly 0\n";
}

// an arithmetic suite passes when every result is faithful, x - x is exactly 0, and the
// worst error stays below the bound.  The bounds sit about 50x above the measured worst
// (3e-7 psi-ulp for double at fbits 28, 3e-14 for dd_cascade at fbits 60), so they catch
// a regression long before it threatens faithful rounding at one psi-ulp.
int verify(const std::string& title, const Stats& s, double bound_ulps, bool reportTestCases) {
	int fails = s.not_faithful + s.nonzero_cancel;
	for (int op = 0; op < 2; ++op) if (max_over_levels(s, op) > bound_ulps) ++fails;
	if (reportTestCases) report(title, s);
	return fails;
}

// psi and phi of native values, per level: absolute psi error, relative phi error
template<typename W>
int VerifyPsiPhi(const std::string& tag, double psi_bound, double phi_bound, bool reportTestCases) {
	double psi_worst[LEVELS] = {}, phi_worst[LEVELS] = {};
	for (const auto& v : lv::psi_vectors) {
		const li::point<W> z = li::psi(W(v.X));
		const double e = std::abs(double(error_of(z, v.psi)));
		psi_worst[z.level] = std::max(psi_worst[z.level], e);
	}
	for (const auto& v : lv::phi_vectors) {
		const int level = static_cast<int>(v.x);
		const W X = li::phi(li::point<W>{ level, W(v.x - level) });
		const dd_cascade exact = dd_cascade(v.phi[0]) + dd_cascade(v.phi[1]) + dd_cascade(v.phi[2]);
		const double rel = std::abs(double(((dd_cascade(X) - exact) / exact)));
		phi_worst[level] = std::max(phi_worst[level], rel);
	}
	int fails = 0;
	for (int l = 0; l < LEVELS; ++l) {
		if (psi_worst[l] > psi_bound) ++fails;
		if (phi_worst[l] > phi_bound * std::pow(16.0, l)) ++fails;   // phi amplifies an index error by phi'(x) / phi(x)
	}
	if (reportTestCases) {
		std::cout << tag << ": psi(X) absolute error and phi(x) relative error, by level\n";
		for (int l = 0; l < 5; ++l)
			std::cout << "    " << l << "  psi " << std::setw(11) << std::setprecision(3) << psi_worst[l]
			          << "    phi " << std::setw(11) << phi_worst[l] << '\n';
	}
	return fails;
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

	std::string test_suite  = "level-index kernel (#1693)";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	report("double, fbits 28", measure<double>(lv::arithmetic_fbits28, 28));

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(
		verify("W = double, fbits 28 (lindex<32,3>)", measure<double>(lv::arithmetic_fbits28, 28), 0x1p-16, reportTestCases),
		"level_index kernel", "add/sub, double, fbits 28");
	nrOfFailedTestCases += ReportTestResult(
		verify("W = dd_cascade, fbits 60 (lindex<64,3>)", measure<dd_cascade>(lv::arithmetic_fbits60, 60), 0x1p-40, reportTestCases),
		"level_index kernel", "add/sub, dd_cascade, fbits 60");
	if (reportTestCases) report("W = double, fbits 60 (reported, not asserted)", measure<double>(lv::arithmetic_fbits60, 60));
	nrOfFailedTestCases += ReportTestResult(VerifyPsiPhi<double>("W = double", 0x1p-50, 0x1p-50, reportTestCases), "level_index kernel", "psi and phi, double");
	nrOfFailedTestCases += ReportTestResult(VerifyPsiPhi<dd_cascade>("W = dd_cascade", 0x1p-100, 0x1p-100, reportTestCases), "level_index kernel", "psi and phi, dd_cascade");
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
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
