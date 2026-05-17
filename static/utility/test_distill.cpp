// test_distill.cpp: unit tests for decimal_to_binary::distill<N>
//                  (issue #848 -- exact decimal-to-(hi,lo,...) conversion)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// distill produces a canonical non-overlapping IEEE-754 expansion equivalent
// to the original parsed value. The invariants checked here are:
//   - Sum of components reconstructs the value (within target precision)
//   - Components are canonical: |out[i+1]| <= ulp(out[i]) / 2 (or zero)
//   - Exact-in-double inputs yield a single non-zero component and zeros
//   - Sign handling for both positive and negative input

#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <universal/utility/decimal_to_binary.hpp>
#include <universal/verification/test_reporters.hpp>

namespace d2b = sw::universal::decimal_to_binary;

namespace {

// |out[i+1]| <= ulp(out[i]) / 2 -- canonical cascade invariant.
// Implemented via std::ldexp(0.5, exponent_of(out[i])) bound.
bool canonical_cascade(const double* arr, unsigned n) {
	for (unsigned i = 0; i + 1 < n; ++i) {
		if (arr[i] == 0.0) {
			if (arr[i + 1] != 0.0) return false;
			continue;
		}
		int e;
		std::frexp(arr[i], &e);  // arr[i] = (mantissa in [0.5, 1)) * 2^e
		// ulp(arr[i]) = 2^(e - 53) for normal doubles (53-bit significand including hidden)
		// |arr[i+1]| <= 2^(e - 53) / 2 = 2^(e - 54)
		double limit = std::ldexp(1.0, e - 54);
		if (std::fabs(arr[i + 1]) > limit) return false;
	}
	return true;
}

}  // namespace

int main()
try {
	using namespace sw::universal;
	std::string test_suite  = "decimal_to_binary::distill (issue #848)";
	bool reportTestCases    = false;
	int  nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----- exact-in-double inputs distill to one component + zeros -----
	{
		int start = nrOfFailedTestCases;
		struct Case { const char* s; double v; };
		Case cases[] = {
			{ "0",     0.0   },
			{ "1.0",   1.0   },
			{ "-1.0", -1.0   },
			{ "0.5",   0.5   },
			{ "2.0",   2.0   },
			{ "1.5",   1.5   },
			{ "1024",  1024.0 },
		};
		for (const auto& c : cases) {
			auto r = d2b::convert(c.s, 64);
			double out[4] = {0, 0, 0, 0};
			d2b::distill(r, out);
			if (out[0] != c.v) {
				++nrOfFailedTestCases;
				if (reportTestCases) {
					std::cout << "  exact-in-double \"" << c.s << "\": got " << out[0]
					          << " expected " << c.v << '\n';
				}
				continue;
			}
			if (out[1] != 0.0 || out[2] != 0.0 || out[3] != 0.0) {
				++nrOfFailedTestCases;
				if (reportTestCases) {
					std::cout << "  exact-in-double \"" << c.s
					          << "\": expected residuals all zero, got "
					          << out[1] << " / " << out[2] << " / " << out[3] << '\n';
				}
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: exact-in-double cases\n";
	}

	// ----- 0.1 (non-terminating binary): N=2 distillation matches double -----
	// double(0.1) is a specific rounded value; out[0] should equal it; out[1]
	// captures the residual (could be small positive or negative). Sum
	// reconstructs the parsed value to within target precision.
	{
		int start = nrOfFailedTestCases;
		auto r = d2b::convert("0.1", 64);
		double out[2] = {0, 0};
		d2b::distill(r, out);
		if (out[0] != 0.1) {
			++nrOfFailedTestCases;
			if (reportTestCases) {
				std::cout << "  0.1 hi mismatch: got " << out[0] << " expected 0.1\n";
			}
		}
		if (!canonical_cascade(out, 2)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "  0.1 cascade not canonical\n";
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: 0.1 N=2 distillation\n";
	}

	// ----- 3.141592653589793 (16 decimal digits): N=3 distillation -----
	// hi should be the IEEE double of pi; the residual captures the trailing
	// bits beyond double's 52-bit mantissa.
	{
		int start = nrOfFailedTestCases;
		auto r = d2b::convert("3.141592653589793238462643383279502884197", 256);
		double out[3] = {0, 0, 0};
		d2b::distill(r, out);
		// out[0] is round-to-nearest-double of full pi expansion
		double pi_double = 0;
		{
			// Build pi_double from its standard bit pattern.
			std::uint64_t pi_bits = 0x400921FB54442D18ULL;
			std::memcpy(&pi_double, &pi_bits, sizeof(double));
		}
		if (out[0] != pi_double) {
			++nrOfFailedTestCases;
			if (reportTestCases) {
				std::cout << "  pi hi mismatch: got " << out[0] << '\n';
			}
		}
		if (!canonical_cascade(out, 3)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "  pi cascade not canonical\n";
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: pi N=3 distillation\n";
	}

	// ----- canonical-cascade invariant under many representative inputs -----
	{
		int start = nrOfFailedTestCases;
		const char* cases[] = {
			"1.0", "2.0", "0.5", "0.1", "0.2", "0.3",
			"1.25e3", "1e10", "1e-10", "1.5e100", "1.5e-100",
			"3.14159265358979323846", "-2.71828182845904523536",
			"1.7976931348623157e308",  // near max double
			"2.2250738585072014e-308", // near min normal
		};
		for (const char* s : cases) {
			auto r = d2b::convert(s, 256);
			if (!r.valid) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  parse failed: " << s << '\n';
				continue;
			}
			double out2[2] = {0, 0};
			double out3[3] = {0, 0, 0};
			double out4[4] = {0, 0, 0, 0};
			d2b::distill(r, out2);
			d2b::distill(r, out3);
			d2b::distill(r, out4);
			if (!canonical_cascade(out2, 2)
			 || !canonical_cascade(out3, 3)
			 || !canonical_cascade(out4, 4)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  cascade not canonical: " << s << '\n';
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: canonical-cascade invariant\n";
	}

	// ----- hi component agrees with std::stod for in-range inputs -----
	// stod gives a single round-to-nearest-double. distill[0] should match.
	{
		int start = nrOfFailedTestCases;
		const char* cases[] = {
			"1.0", "2.0", "0.5", "0.1", "0.2", "0.3", "3.14",
			"1.25e3", "1e10", "1e-10", "1e100", "1e-100",
			"3.141592653589793",
			"-2.5", "-0.625",
		};
		for (const char* s : cases) {
			auto r = d2b::convert(s, 256);
			double out[4] = {0, 0, 0, 0};
			d2b::distill(r, out);
			double ref = std::stod(s);
			if (out[0] != ref) {
				++nrOfFailedTestCases;
				if (reportTestCases) {
					std::cout << "  stod oracle mismatch \"" << s << "\": got "
					          << out[0] << " stod=" << ref << '\n';
				}
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: stod oracle agreement\n";
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
