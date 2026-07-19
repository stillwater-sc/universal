// hypot.cpp: regression tests for the efloat hypotenuse function.
//
// hypot(x,y) = sqrt(x^2 + y^2), computed with scale-by-max so intermediates
// neither overflow nor underflow, on efloat's own arithmetic (full precision).
// Covers Pythagorean values, symmetry, zeros, overflow/underflow prevention,
// IEEE special values, and high-precision oracle identities. (Issue #1121)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

template<unsigned nlimbs>
bool IsClose(const sw::universal::efloat<nlimbs>& a, double target_val, double tolerance = 1e-12) {
	double diff = std::abs(double(a) - target_val);
	return diff <= tolerance * (1.0 + std::abs(target_val));
}

// |a - b| as a binary scale; very negative means agreement to that many bits.
template<unsigned nlimbs>
int64_t AgreementScale(const sw::universal::efloat<nlimbs>& a, const sw::universal::efloat<nlimbs>& b) {
	sw::universal::efloat<nlimbs> d = a - b;
	d.setsign(false);
	return d.iszero() ? -1000000 : d.scale();
}

int VerifyEfloatHypot(bool reportTestCases) {
	using namespace sw::universal;
	using E      = efloat<8>;   // 256-bit
	using EH     = efloat<16>;  // 512-bit for high-precision identities
	int failures = 0;

	// ---------------------------------------------------------------------
	// 1. exact Pythagorean values (3-4-5 and its 6-8-10 multiple are exact:
	//    r = min/max = 0.75 is exactly representable in binary)
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying exact Pythagorean values...\n";
	{
		if (hypot(E(3.0), E(4.0)) != E(5.0)) {
			if (reportTestCases)
				std::cout << "    FAIL: hypot(3,4) != 5\n";
			++failures;
		}
		if (hypot(E(4.0), E(3.0)) != E(5.0)) {
			if (reportTestCases)
				std::cout << "    FAIL: hypot(4,3) != 5\n";
			++failures;
		}
		if (hypot(E(6.0), E(8.0)) != E(10.0)) {
			if (reportTestCases)
				std::cout << "    FAIL: hypot(6,8) != 10\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 2. symmetry: hypot(x,y) == hypot(y,x) == hypot(x,-y) == hypot(-x,y)
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying symmetry...\n";
	{
		E a(2.5), b(7.25);
		E h = hypot(a, b);
		if (hypot(b, a) != h) {
			if (reportTestCases)
				std::cout << "    FAIL: hypot not symmetric in args\n";
			++failures;
		}
		if (hypot(a, -b) != h) {
			if (reportTestCases)
				std::cout << "    FAIL: hypot(x,-y) != hypot(x,y)\n";
			++failures;
		}
		if (hypot(-a, b) != h) {
			if (reportTestCases)
				std::cout << "    FAIL: hypot(-x,y) != hypot(x,y)\n";
			++failures;
		}
		if (hypot(-a, -b) != h) {
			if (reportTestCases)
				std::cout << "    FAIL: hypot(-x,-y) != hypot(x,y)\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 3. zero arguments: hypot(0,y) == |y|, hypot(x,0) == |x|, hypot(0,0)==0
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying zero arguments...\n";
	{
		if (hypot(E(0.0), E(7.0)) != E(7.0)) {
			if (reportTestCases)
				std::cout << "    FAIL: hypot(0,7) != 7\n";
			++failures;
		}
		if (hypot(E(-7.0), E(0.0)) != E(7.0)) {
			if (reportTestCases)
				std::cout << "    FAIL: hypot(-7,0) != 7\n";
			++failures;
		}
		if (!hypot(E(0.0), E(0.0)).iszero()) {
			if (reportTestCases)
				std::cout << "    FAIL: hypot(0,0) != 0\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 4. overflow/underflow prevention -- the whole reason hypot exists.
	//    A naive sqrt(x*x + y*y) would over/underflow; scale-by-max must not.
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying overflow/underflow prevention...\n";
	{
		E big = hypot(E(3e200), E(4e200));
		if (big.isnan() || big.isinf() || !IsClose(big / E(1e200), 5.0, 1e-12)) {
			if (reportTestCases)
				std::cout << "    FAIL: hypot(3e200,4e200) not ~5e200 (got scale " << big.scale() << ")\n";
			++failures;
		}
		E small = hypot(E(3e-200), E(4e-200));
		if (small.iszero() || !IsClose(small / E(1e-200), 5.0, 1e-12)) {
			if (reportTestCases)
				std::cout << "    FAIL: hypot(3e-200,4e-200) not ~5e-200\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 5. IEEE special values
	//    - +/-inf in either argument -> +inf (even if the other is NaN)
	//    - otherwise NaN in either argument -> NaN
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying special values...\n";
	{
		E pinf;
		pinf.setinf(false);
		E ninf;
		ninf.setinf(true);
		E nan;
		nan.setnan();

		if (!hypot(pinf, E(1.0)).isinf() || hypot(pinf, E(1.0)).sign() != 1) {
			if (reportTestCases)
				std::cout << "    FAIL: hypot(+inf,1) != +inf\n";
			++failures;
		}
		if (!hypot(E(1.0), ninf).isinf() || hypot(E(1.0), ninf).sign() != 1) {
			if (reportTestCases)
				std::cout << "    FAIL: hypot(1,-inf) != +inf\n";
			++failures;
		}
		// infinity suppresses NaN
		if (!hypot(pinf, nan).isinf()) {
			if (reportTestCases)
				std::cout << "    FAIL: hypot(+inf,NaN) != +inf\n";
			++failures;
		}
		if (!hypot(nan, ninf).isinf()) {
			if (reportTestCases)
				std::cout << "    FAIL: hypot(NaN,-inf) != +inf\n";
			++failures;
		}
		// NaN otherwise
		if (!hypot(nan, E(1.0)).isnan()) {
			if (reportTestCases)
				std::cout << "    FAIL: hypot(NaN,1) != NaN\n";
			++failures;
		}
		if (!hypot(E(1.0), nan).isnan()) {
			if (reportTestCases)
				std::cout << "    FAIL: hypot(1,NaN) != NaN\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 6. oracle-grade: hypot(x,y)^2 == x^2 + y^2 at 512-bit
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying hypot^2 == x^2+y^2 at 512-bit...\n";
	{
		EH      x(0.7), y(1.3);
		EH      h  = hypot(x, y);
		EH      h2 = h * h;
		EH      s  = x * x + y * y;
		int64_t sc = AgreementScale(h2, s);
		if (sc > -400) {
			if (reportTestCases)
				std::cout << "    FAIL: hypot^2 vs x^2+y^2 scale=" << sc << "\n";
			++failures;
		}
	}

	// ---------------------------------------------------------------------
	// 7. oracle-grade: hypot(1,1) == sqrt(2) to full working precision
	// ---------------------------------------------------------------------
	if (reportTestCases)
		std::cout << "  Verifying hypot(1,1) == sqrt(2)...\n";
	{
		EH      h  = hypot(EH(1.0), EH(1.0));
		EH      s2 = sqrt(EH(2.0));
		int64_t sc = AgreementScale(h, s2);
		if (sc > -400) {
			if (reportTestCases)
				std::cout << "    FAIL: hypot(1,1) vs sqrt(2) scale=" << sc << "\n";
			++failures;
		}
	}

	return failures;
}

}  // anonymous namespace

#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat mathematical hypot library";
	std::string test_tag            = "hypot";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyEfloatHypot(reportTestCases), "efloat", "hypot manual");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyEfloatHypot(reportTestCases), "efloat", "hypot foundational");
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif
} catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
