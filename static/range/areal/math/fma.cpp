// fma.cpp: functional tests for the areal fused multiply-add fma(a,b,c)
//
// areal (faithful float with an uncertainty bit) had no fma (#1194, sub-issue of the
// universal fma epic #1189). fma widens the operands to double, forms a*b + c with
// std::fma, and faithfully rounds the result once into areal via the value
// constructor. areal is a faithful encoding: the LSB is the uncertainty bit (ubit),
// which is 0 when the value is exact and 1 when the stored value is the (truncated)
// representative of the open interval to the adjacent encoding that contains the true
// result. This suite documents and validates that semantics:
//   - exactness: when a*b + c is exactly representable, fma is exact and ubit == 0;
//   - uncertainty: when it is not, ubit == 1 and the true value lies within one ulp;
//   - faithfulness: for random operands, |value(fma) - true| < ulp, with
//     (ubit == 0) iff the result is exact -- checked against a long-double reference
//     computed independently of the implementation's double/std::fma path;
//   - IEEE special values (inf*0 -> NaN, inf / NaN propagation).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <random>
#include <string>

#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	// value-significand precision p and the ulp (gap between adjacent value encodings)
	// at a given result: the fraction LSB is the ubit, so p = nbits - es - 1 and the
	// value-ulp at v is 2^(ilogb(v) - (p - 1)).
	template<unsigned nbits, unsigned es>
	double value_ulp(double v) {
		constexpr int p = int(nbits) - int(es) - 1;
		return std::ldexp(1.0, std::ilogb(v) - (p - 1));
	}

	// ---- 1. exactness: exactly-representable results are exact (ubit == 0) -----
	template<unsigned nbits, unsigned es, typename bt = uint16_t>
	int VerifyExactness(bool reportTestCases) {
		using Areal = areal<nbits, es, bt>;
		int fails = 0;
		// (a, b, c, expected) with a*b + c exactly representable in areal<nbits,es>
		const double cases[][4] = {
			{ 1.5,  1.5,  0.25, 2.5   },   // 2.25 + 0.25
			{ 2.0,  3.0,  1.0,  7.0   },
			{ 0.5,  0.5,  0.5,  0.75  },
			{-1.5,  2.0,  0.5, -2.5   },
			{ 4.0,  0.25, 1.0,  2.0   },
			{ 0.0,  3.0,  2.0,  2.0   },   // 0*b + c
			{ 1.0,  3.0,  2.0,  5.0   },   // 1*b + c
		};
		for (auto& tc : cases) {
			Areal a(tc[0]), b(tc[1]), c(tc[2]);
			Areal r = fma(a, b, c);
			if (double(r) != tc[3] || r.at(0) != false) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL exact fma(" << tc[0] << ", " << tc[1] << ", " << tc[2]
					<< ") = " << double(r) << " ubit=" << r.at(0) << "  expected " << tc[3] << " ubit=0\n";
			}
		}
		return fails;
	}

	// ---- 2 & 3. faithfulness + uncertainty-bit semantics vs long-double ref ----
	template<unsigned nbits, unsigned es, typename bt = uint16_t>
	int VerifyFaithful(bool reportTestCases, int nrTests, uint64_t seed, int& inexactSeen) {
		using Areal = areal<nbits, es, bt>;
		int fails = 0;
		inexactSeen = 0;
		std::mt19937_64 rng(seed);
		std::uniform_real_distribution<double> U(-4.0, 4.0);
		for (int t = 0; t < nrTests; ++t) {
			Areal a(U(rng)), b(U(rng)), c(U(rng));
			Areal r = fma(a, b, c);
			double v = double(r);
			// independent reference: exact a*b + c in long double (exact for these small configs)
			long double tru = (long double)double(a) * (long double)double(b) + (long double)double(c);
			bool ubit = r.at(0);
			if (v == 0.0) {                      // exact zero result: must be exact
				if (tru != 0.0L || ubit) { ++fails; if (reportTestCases) std::cout << "    FAIL zero-result faithfulness\n"; }
				continue;
			}
			long double err = std::fabs((long double)v - tru);
			double ulp = value_ulp<nbits, es>(v);
			// faithful: value is within one ulp of the true fused product-sum
			if (err > (long double)ulp) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL faithfulness: |v - true| = " << double(err)
					<< " > ulp " << ulp << "  (v=" << v << ", true=" << double(tru) << ")\n";
			}
			// uncertainty bit: 0 iff the result is exact
			bool exact = (err == 0.0L);
			if (ubit == exact) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL ubit semantics: ubit=" << ubit << " but exact=" << exact
					<< "  (v=" << v << ", true=" << double(tru) << ")\n";
			}
			if (ubit) ++inexactSeen;
		}
		return fails;
	}

	// ---- 4. IEEE special values -----------------------------------------------
	template<unsigned nbits, unsigned es, typename bt = uint16_t>
	int VerifySpecialValues(bool reportTestCases) {
		using Areal = areal<nbits, es, bt>;
		int fails = 0;
		const Areal inf(std::numeric_limits<double>::infinity());
		const Areal ninf(-std::numeric_limits<double>::infinity());
		const Areal zero(0.0), one(1.0), two(2.0), three(3.0);
		auto expect_inf = [&](Areal r, bool neg, const char* tag) {
			bool ok = r.isinf() && (neg ? (double(r) < 0) : (double(r) > 0));
			if (!ok) { ++fails; if (reportTestCases) std::cout << "    FAIL special " << tag << " -> " << double(r) << '\n'; }
		};
		expect_inf(fma(inf, two, three), false, "inf*2+3 -> +inf");
		expect_inf(fma(two, three, inf), false, "2*3+inf -> +inf");
		expect_inf(fma(two, three, ninf), true, "2*3-inf -> -inf");
		expect_inf(fma(ninf, two, one), true, "-inf*2+1 -> -inf");
		// inf*0 + finite -> NaN
		if (!fma(inf, zero, one).isnan()) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(inf,0,1) not NaN\n"; }
		// NaN propagation
		const Areal nan(std::numeric_limits<double>::quiet_NaN());
		if (!fma(nan, one, one).isnan()) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(NaN,1,1) not NaN\n"; }
		if (!fma(one, nan, one).isnan()) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(1,NaN,1) not NaN\n"; }
		if (!fma(one, one, nan).isnan()) { ++fails; if (reportTestCases) std::cout << "    FAIL fma(1,1,NaN) not NaN\n"; }
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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;
	std::string test_suite = "areal fused multiply-add fma(a,b,c) (#1194)";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;
	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += VerifyExactness<16, 5>(true);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

	int base = 10000;
#if REGRESSION_LEVEL_2
	base = 50000;
#endif

	nrOfFailedTestCases += ReportTestResult(VerifyExactness<16, 5>(reportTestCases), "fma exactness areal<16,5>",  "fma");
	nrOfFailedTestCases += ReportTestResult(VerifyExactness<20, 6>(reportTestCases), "fma exactness areal<20,6>",  "fma");
	{
		int inexact = 0;
		nrOfFailedTestCases += ReportTestResult(
			VerifyFaithful<16, 5>(reportTestCases, base, 0xA1EA1, inexact), "fma faithful+ubit areal<16,5>", "fma");
		std::cout << "    (areal<16,5>: " << inexact << " of " << base << " random results were inexact -> ubit=1)\n";
	}
	{
		int inexact = 0;
		nrOfFailedTestCases += ReportTestResult(
			VerifyFaithful<20, 6>(reportTestCases, base, 0xB2EB2, inexact), "fma faithful+ubit areal<20,6>", "fma");
		std::cout << "    (areal<20,6>: " << inexact << " of " << base << " random results were inexact -> ubit=1)\n";
	}
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<16, 5>(reportTestCases), "fma IEEE special values", "fma");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
