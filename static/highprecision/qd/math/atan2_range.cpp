// atan2_range.cpp: qd atan2 over the whole exponent range (#2032)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// atan2 normalized its arguments by sqrt(x^2 + y^2), and the squares overflow or underflow
// long before the arguments do: atan2(2e100, -1e200) came out -8e15, atan2(1e160, 3e160) 7.39
// and atan2(3e-200, 1e-200) NaN. atan2 is scale invariant, and it now brings both arguments
// near 1 by the same power of 2, which is exact.
//  - atan2(y 2^k, x 2^k) == atan2(y, x), every limb, for k across [-800, 900];
//  - far-apart arguments against the same ratio near 1, to the type's precision.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <string>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/test_suite.hpp>

namespace {
	using namespace sw::universal;

	qd scaled(const qd& v, int k) {
		return qd(std::ldexp(v[0], k), std::ldexp(v[1], k), std::ldexp(v[2], k), std::ldexp(v[3], k));
	}

	bool identical(const qd& a, const qd& b) {
		for (int i = 0; i < 4; ++i) {
			if (!(a[i] == b[i])) return false;
		}
		return true;
	}

	int VerifyAtan2Range(bool reportTestCases) {
		int n = 0;
		const qd one(1.0), three(3.0);
		const qd ys[] = { one, -one / three, qd(0.5), three, qd(-2.0) };
		const qd xs[] = { three, qd(0.75), qd(-0.75), qd(-1e-3), one / three };
		for (const qd& y : ys) {
			for (const qd& x : xs) {
				const qd ref = atan2(y, x);
				for (int k = -800; k <= 900; k += 25) {
					const qd got = atan2(scaled(y, k), scaled(x, k));
					if (!identical(got, ref)) {
						++n;
						if (reportTestCases) std::cout << "    FAIL atan2(" << double(y) << " 2^" << k << ", " << double(x) << " 2^" << k << ") = "
						                               << got << ", at 2^0 " << ref << '\n';
					}
				}
			}
		}
		// far-apart arguments, exact multiples by powers of 2 of a pair near 1, so the reference
		// is exact; 2e100 / -1e200 is pi less 2e-100, pi to any precision carried here
		struct { double y, x, y1, x1; } far[] = {
			{ 2e100, -1e200, 0.0, -1.0 },
			{ std::ldexp(1.0, 530), std::ldexp(3.0, 530), 1.0, 3.0 },
			{ std::ldexp(3.0, -665), std::ldexp(1.0, -665), 3.0, 1.0 },
			{ -std::ldexp(1.0, -997), std::ldexp(4.0, -997), -1.0, 4.0 },
			{ 1e300, -1e300, 1.0, -1.0 },
		};
		for (const auto& c : far) {
			const qd got = atan2(qd(c.y), qd(c.x));
			const qd ref = atan2(qd(c.y1), qd(c.x1));
			const qd d = got - ref;
			if (!(abs(d) <= abs(ref) * 1e-60)) {
				++n;
				if (reportTestCases) std::cout << "    FAIL atan2(" << c.y << ", " << c.x << ") = " << got << ", want " << ref << '\n';
			}
		}
		return n;
	}

}  // namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
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

	std::string test_suite  = "quad-double atan2 across the exponent range (#2032)";
	std::string test_tag    = "atan2";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyAtan2Range(reportTestCases), "qd", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyAtan2Range(reportTestCases), "qd", test_tag);
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
