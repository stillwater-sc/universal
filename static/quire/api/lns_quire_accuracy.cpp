// lns_quire_accuracy.cpp: pins the documented accuracy characteristics of quire<lns> (#1203)
//
// quire<lns> is NOT an exact accumulator. An lns product is 2^(k + m/2^rbits), irrational
// for m != 0, so it cannot be represented exactly in any linear fixed-point quire at any
// width (see number/lns/fdp.hpp). quire_mul rounds each product at ~the lns representation
// precision -- well below the quire's own precision -- so the quire-dot error is bounded by
// the PRODUCT-representation error, not by the accumulation, and for narrow lns it can be
// worse than a promoted-double accumulator.
//
// This is executable documentation of that contract (guarding against a regression to the
// old, incorrect "lns products are always exact" claim):
//   1. integer-exponent (power-of-2) operands with a power-of-2 result ARE exact;
//   2. fractional-exponent operands are NOT -- the quire-dot error, measured against a
//      long-double reference over the SAME quantized values (isolating product/accumulation
//      error from input quantization), is orders of magnitude above binary64 precision.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include <universal/number/lns/lns.hpp>
#include <universal/number/lns/fdp.hpp>   // explicit until #1201 has the aggregator pull it in
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	// (1) integer-exponent (power-of-2) product AND power-of-2 result -> exact
	template<unsigned nbits, unsigned rbits>
	int VerifyExactPowerOfTwo(const char* tag, bool reportTestCases) {
		using Lns = lns<nbits, rbits>;
		// 2*2 + 2*2 = 8 : products 4,4 (integer exponents, exact), sum 8 = 2^3 (representable)
		std::vector<Lns> x = { Lns(2), Lns(2) };
		std::vector<Lns> y = { Lns(2), Lns(2) };
		double d = double(fdp(x, y));
		if (d != 8.0) {
			if (reportTestCases) std::cout << "    FAIL exact power-of-2 dot<" << tag << "> = " << d << " expected 8\n";
			return 1;
		}
		return 0;
	}

	// (2) fractional-exponent operands -> quire<lns> is NOT exact and far from double accuracy.
	// Reference: long-double dot over the SAME quantized lns values (double(x[i])), so the
	// measured error is purely the quire's product/accumulation rounding, not input quantization.
	template<unsigned nbits, unsigned rbits>
	int VerifyQuireNotExact(const char* tag, bool reportTestCases, double floor_error) {
		using Lns = lns<nbits, rbits>;
		std::vector<Lns> x, y;
		for (int i = 1; i <= 64; ++i) {           // fractional values -> fractional log exponents
			x.push_back(Lns(1.0 + 0.5 / i));
			y.push_back(Lns(1.0 - 0.25 / i));
		}
		long double ref = 0.0L;
		for (std::size_t i = 0; i < x.size(); ++i)
			ref += (long double)double(x[i]) * (long double)double(y[i]);
		double d = double(fdp(x, y));
		double err = std::fabs((double)((long double)d - ref));
		// contract: the lns quire error sits WELL above binary64 precision (a promoted-double
		// accumulator would land near 1e-15). If this ever drops below floor_error, the quire
		// was made more accurate -- update the doc + this threshold.
		if (err < floor_error) {
			if (reportTestCases) std::cout << "    FAIL quire<lns> unexpectedly accurate for " << tag
				<< ": err=" << err << " < floor " << floor_error << " (did the product rounding change?)\n";
			return 1;
		}
		if (reportTestCases) std::cout << "    [characterization] quire<" << tag << "> dot error = " << err
			<< "  (a promoted-double accumulator would be ~1e-15)\n";
		return 0;
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
	std::string test_suite = "quire<lns> accuracy contract: rounded, not exact (#1203)";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	int fails = 0;
	fails += VerifyExactPowerOfTwo<16, 8>("lns<16,8>", reportTestCases);
	fails += VerifyExactPowerOfTwo<32, 16>("lns<32,16>", reportTestCases);
	// error floors sit far above binary64 (~1e-15) but well below each config's actual error
	// (lns<16,8> ~1e-6, lns<32,16> ~1e-10), so these are robust.
	fails += VerifyQuireNotExact<16, 8>("lns<16,8>", reportTestCases, 1e-9);
	fails += VerifyQuireNotExact<32, 16>("lns<32,16>", reportTestCases, 1e-13);

	nrOfFailedTestCases += ReportTestResult(fails, "quire<lns> exact for power-of-2, rounded otherwise", "lns_quire");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& err) {
	std::cerr << "Caught unexpected exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
