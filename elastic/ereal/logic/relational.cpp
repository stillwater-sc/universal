// relational.cpp: ereal comparisons with NaN, +-inf and +-0, against ereal and double (#2005)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Every relational operator is checked against the host's own comparison of the same values,
// which IEEE 754 defines: NaN is unordered (== and every ordering false, != true), -0 == +0,
// and the infinities order beyond every finite value. Before #2005 compare_adaptive read a
// NaN limb as equal to anything, so nan == 5, nan >= 5 and nan <= 5 were all true. min and
// max follow C99 fmin and fmax: a NaN operand is ignored, whatever the argument order.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	const double inf = std::numeric_limits<double>::infinity();
	const double nan = std::numeric_limits<double>::quiet_NaN();
	// the large and tiny values are in range of the limb type: 1e300 is inf in a float limb
	template<typename Real>
	std::vector<double> test_values() {
		using F = typename Real::limb_type;
		const double big = static_cast<double>(std::numeric_limits<F>::max()) / 4.0;
		const double tiny = static_cast<double>(std::numeric_limits<F>::min()) * 4.0;
		return { nan, -inf, -big, -2.5, -1.0, -0.0, 0.0, tiny, 1.0, 2.5, big, inf };
	}

	std::string show(double v) {
		if (std::isnan(v)) return "nan";
		if (v == 0.0) return std::signbit(v) ? "-0" : "+0";
		return std::to_string(v);
	}

	struct Ops { bool eq, ne, lt, gt, le, ge; };
	inline bool operator!=(const Ops& a, const Ops& b) {
		return a.eq != b.eq || a.ne != b.ne || a.lt != b.lt || a.gt != b.gt || a.le != b.le || a.ge != b.ge;
	}

	template<typename L, typename R>
	Ops relations(const L& a, const R& b) {
		return Ops{ a == b, a != b, a < b, a > b, a <= b, a >= b };
	}

	// ereal vs ereal, ereal vs double, double vs ereal: each must agree with double vs double
	template<typename Real>
	int VerifyRelational(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const std::vector<double> values = test_values<Real>();
		for (double x : values) {
			for (double y : values) {
				const Ops expected = relations(x, y);
				const Ops ee = relations(Real(x), Real(y));
				const Ops ed = relations(Real(x), y);
				const Ops de = relations(x, Real(y));
				const char* which = nullptr;
				if (ee != expected) which = "ereal vs ereal";
				else if (ed != expected) which = "ereal vs double";
				else if (de != expected) which = "double vs ereal";
				if (which != nullptr) {
					++nrOfFailedTestCases;
					if (reportTestCases) std::cerr << "FAIL: " << which << ": " << show(x) << " vs " << show(y) << '\n';
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// A multi-limb value against its leading limb: the comparison reads the whole expansion
	template<typename Real>
	int VerifyMultiLimb(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const Real one(1.0);
		const Real above = one + Real(std::ldexp(1.0, -200));
		const Real below = one - Real(std::ldexp(1.0, -200));
		const Real n(nan);
		if (!(above > one && one < above && below < one && above != one && !(above == one) && above >= one && below <= one)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 1 +- 2^-200 against 1\n";
		}
		if (above == n || above < n || above > n || above <= n || above >= n || !(above != n)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 1 + 2^-200 against nan\n";
		}
		return nrOfFailedTestCases;
	}

	// min, max, fmin and fmax: C99 F.10.9.2 and F.10.9.3, a NaN operand is ignored
	template<typename Real>
	int VerifyMinMax(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		auto same = [](const Real& got, double expected) {
			if (std::isnan(expected)) return got.isnan();
			return !got.isnan() && got == Real(expected);
		};
		const std::vector<double> values = test_values<Real>();
		for (double x : values) {
			for (double y : values) {
				const double lo = std::fmin(x, y), hi = std::fmax(x, y);
				// fmin/fmax of -0 and +0 may return either, in C99 and here
				if (x == 0.0 && y == 0.0) continue;
				const bool ok = same(min(Real(x), Real(y)), lo) && same(max(Real(x), Real(y)), hi)
				             && same(fmin(Real(x), Real(y)), lo) && same(fmax(Real(x), Real(y)), hi);
				if (!ok) {
					++nrOfFailedTestCases;
					if (reportTestCases) std::cerr << "FAIL: min/max(" << show(x) << ", " << show(y) << ")\n";
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// signbit, abs and printing see -0
	template<typename Real>
	int VerifySignedZero(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const Real nz(-0.0), pz(0.0);
		if (!signbit(nz) || signbit(pz)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: signbit(-0) or signbit(+0)\n";
		}
		if (abs(nz).signbit() || !abs(nz).iszero() || abs(Real(-2.0)) != Real(2.0)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: abs(-0) is not +0\n";
		}
		const std::string s = nz.to_string(3);
		if (s.find('-') == std::string::npos || pz.to_string(3).find('-') != std::string::npos) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: to_string(-0) = '" << s << "'\n";
		}
		if (!(nz == pz) || nz < pz || pz < nz) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: -0 and +0 compare unequal\n";
		}
		return nrOfFailedTestCases;
	}

}  // namespace

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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "ereal relational operators";
	std::string test_tag    = "relational";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ereal<> n(nan), f(5.0);
	std::cout << "nan == 5: " << (n == f) << ", nan >= 5: " << (n >= f) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyRelational<ereal<>>(reportTestCases), "ereal<8>", "relational");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiLimb<ereal<>>(reportTestCases), "ereal<8>", "multi-limb");
	nrOfFailedTestCases += ReportTestResult(VerifyMinMax<ereal<>>(reportTestCases), "ereal<8>", "min/max");
	nrOfFailedTestCases += ReportTestResult(VerifySignedZero<ereal<>>(reportTestCases), "ereal<8>", "signed zero");
	nrOfFailedTestCases += ReportTestResult(VerifyRelational<ereal<4, float>>(reportTestCases), "ereal<4, float>", "relational");
	nrOfFailedTestCases += ReportTestResult(VerifyMinMax<ereal<4, float>>(reportTestCases), "ereal<4, float>", "min/max");
	nrOfFailedTestCases += ReportTestResult(VerifySignedZero<ereal<4, float>>(reportTestCases), "ereal<4, float>", "signed zero");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyRelational<ereal<2>>(reportTestCases), "ereal<2>", "relational");
	nrOfFailedTestCases += ReportTestResult(VerifyRelational<ereal<19>>(reportTestCases), "ereal<19>", "relational");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiLimb<ereal<19>>(reportTestCases), "ereal<19>", "multi-limb");
#if LONG_DOUBLE_SUPPORT
	nrOfFailedTestCases += ReportTestResult(VerifyRelational<ereal<4, long double>>(reportTestCases), "ereal<4, long double>", "relational");
#endif
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
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught unexpected universal internal exception: " << err.what() << std::endl;
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
