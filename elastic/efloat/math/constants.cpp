// constants.cpp: regression tests for efloat high-precision constants (pi, e, phi).
//
// The constants are validated by INDEPENDENT methods (no reliance on the stored
// literal being its own oracle):
//   - phi:  identity phi^2 - phi - 1 == 0
//   - e:    vs the native series sum 1/k!  (uses only +,*,/)
//   - pi:   vs Machin's formula 16*atan(1/5) - 4*atan(1/239)  (atan of a small
//           argument is a pure Taylor series and does not use pi)
//
// Issue #1139: efloat as the library's highest-precision constant basis.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <limits>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	// native e = sum_{k>=0} 1/k!  at the operand's precision (independent of the literal)
	template<unsigned nlimbs>
	sw::universal::efloat<nlimbs> series_e(unsigned bits) {
		using E = sw::universal::efloat<nlimbs>;
		E e(1.0), term(1.0); e.set_precision(bits);
		for (unsigned k = 1; k < 4000; ++k) {
			term = term / E(static_cast<double>(k));
			if (term.iszero()) break;
			if (term.scale() < e.scale() - static_cast<int64_t>(e.get_precision())) break;
			e += term;
		}
		return e;
	}

	// native pi via Machin: 16*atan(1/5) - 4*atan(1/239)
	template<unsigned nlimbs>
	sw::universal::efloat<nlimbs> machin_pi(unsigned bits) {
		using E = sw::universal::efloat<nlimbs>;
		E fifth = E(1.0) / E(5.0);   fifth.set_precision(bits);
		E t239  = E(1.0) / E(239.0); t239.set_precision(bits);
		return E(16.0) * atan(fifth) - E(4.0) * atan(t239);
	}

	// |a - b| binary scale (very negative == agreement to many bits)
	template<unsigned nlimbs>
	int64_t diff_scale(sw::universal::efloat<nlimbs> a, const sw::universal::efloat<nlimbs>& b) {
		a = a - b; a.setsign(false);
		return a.iszero() ? -1000000 : a.scale();
	}

	// Verify the three constants at capacity `nlimbs`, requiring agreement to
	// `min_agree_bits` bits (negative scale threshold).
	template<unsigned nlimbs>
	int VerifyConstants(unsigned bits, int64_t min_agree_bits, bool reportTestCases) {
		using namespace sw::universal;
		using E = efloat<nlimbs>;
		int failures = 0;
		const int64_t thr = -min_agree_bits;   // require scale <= thr

		E pi  = efloat_pi<nlimbs>();
		E e   = efloat_e<nlimbs>();
		E phi = efloat_phi<nlimbs>();

		// low-order sanity: match the host doubles
		if (std::abs(double(pi)  - M_PI)               > 1e-13) { if (reportTestCases) std::cout << "    FAIL: pi  vs double\n"; ++failures; }
		if (std::abs(double(e)   - std::exp(1.0))      > 1e-13) { if (reportTestCases) std::cout << "    FAIL: e   vs double\n"; ++failures; }
		if (std::abs(double(phi) - (1.0+std::sqrt(5.0))/2.0) > 1e-13) { if (reportTestCases) std::cout << "    FAIL: phi vs double\n"; ++failures; }

		// phi identity: phi^2 - phi - 1 == 0
		{
			E id = phi * phi - phi - E(1.0); id.setsign(false);
			int64_t sc = id.iszero() ? -1000000 : id.scale();
			if (sc > thr) { if (reportTestCases) std::cout << "    FAIL: phi^2-phi-1 scale=" << sc << " (want <=" << thr << ")\n"; ++failures; }
		}
		// e vs native series
		{
			int64_t sc = diff_scale<nlimbs>(e, series_e<nlimbs>(bits));
			if (sc > thr) { if (reportTestCases) std::cout << "    FAIL: e vs series scale=" << sc << " (want <=" << thr << ")\n"; ++failures; }
		}
		// pi vs Machin
		{
			int64_t sc = diff_scale<nlimbs>(pi, machin_pi<nlimbs>(bits));
			if (sc > thr) { if (reportTestCases) std::cout << "    FAIL: pi vs Machin scale=" << sc << " (want <=" << thr << ")\n"; ++failures; }
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

	std::string test_suite          = "efloat mathematical constants library";
	std::string test_tag            = "constants";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1
	// Fast: efloat<32> (~1024 bits ~ 308 digits) -- require ~300-digit agreement.
	if (reportTestCases) std::cout << "  Verifying constants at ~308 digits (efloat<32>)...\n";
	nrOfFailedTestCases += ReportTestResult(VerifyConstants<32>(1000u, 990, reportTestCases), "efloat", "constants ~308 digits");
#endif

#if REGRESSION_LEVEL_4
	// Full: efloat<128> (~3300 bits ~ 1000 digits) -- require ~1000-digit agreement.
	if (reportTestCases) std::cout << "  Verifying constants at ~1000 digits (efloat<128>)...\n";
	nrOfFailedTestCases += ReportTestResult(VerifyConstants<128>(3300u, 3250, reportTestCases), "efloat", "constants ~1000 digits");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
