// attribute_accessors.cpp: the posit attribute accessors, instantiated so they cannot rot
//
// significant(posit) did not compile. It calls positFraction::get_fixed_point(), which
// called blockbinary::set(unsigned, bool) -- an overload that does not exist, since
// blockbinary offers set(), set(unsigned) and setbit(unsigned, bool). A function
// template's body is not checked until it is instantiated, get_fixed_point()'s only
// caller is significant(posit), and significant(posit) had no callers anywhere in the
// tree, so neither had ever been compiled (#1592).
//
// That is the fifth instance of this found recently -- components(areal) in #1453,
// components(efloat) and to_binary(efloat) in #1556, type_field(bfloat16) in #1551 --
// and the remedy each time is the same: instantiate the surface somewhere that is built.
//
// So this suite calls every attribute accessor, and then checks the two that decompose a
// posit against the value they came from. Instantiating proves they compile;
// reconstructing the posit from its own scale and significand proves they are right,
// which is the only reason to have them.
//
// The companion attributes.cpp next door is a walk-through of the component values; this
// file is the regression check on the accessor surface.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <string>

#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>

// set to 1 to run the exploratory walk-through instead of the regression suite
#define MANUAL_TESTING 0

namespace {

	using namespace sw::universal;

	int expect_true(bool actual, const std::string& what, bool reportTestCases) {
		if (actual) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}

	// Rebuild the value from what the accessors report. significant() returns the
	// fraction with the hidden bit made explicit at index fbits, so a normal posit is
	// significand * 2^(scale - fbits).
	template<unsigned nbits, unsigned es>
	double reconstruct(const posit<nbits, es>& p) {
		constexpr unsigned fbits = nbits - 3u - es;
		const auto sig = significant(p);
		const int  sc  = scale(p);
		double significand = 0.0;
		for (unsigned i = 0; i <= fbits; ++i) {
			if (sig.test(i)) significand += std::ldexp(1.0, int(i) - int(fbits));
		}
		const double magnitude = std::ldexp(significand, sc);
		return sign(p) ? -magnitude : magnitude;
	}

	// every accessor is called, so every body is compiled
	template<unsigned nbits, unsigned es>
	int VerifyAccessorsInstantiate(const std::string& tag, bool reportTestCases) {
		using P = posit<nbits, es>;
		int fails = 0;
		const P v(1.5f);

		// fbits is defaulted now, so these are callable without naming all four template
		// arguments -- which is presumably why significant() never had a caller
		auto sig  = significant(v);
		auto frac = extract_fraction(v);
		const int  sc  = scale(v);
		const int  rs  = regime_scale(v);
		const int  xs  = exponent_scale(v);
		const bool sgn = sign(v);

		fails += expect_true(sig.nbits == (nbits - 3u - es) + 1u,
			tag + ": significant is fbits+1 wide", reportTestCases);
		fails += expect_true(frac.nbits == (nbits - 3u - es),
			tag + ": extract_fraction is fbits wide", reportTestCases);
		fails += expect_true(sc == rs + xs,
			tag + ": scale is the regime scale plus the exponent scale", reportTestCases);
		fails += expect_true(sgn == false, tag + ": 1.5 is positive", reportTestCases);
		// the hidden bit is explicit in significant(), so the top bit is set for a normal
		fails += expect_true(sig.test(nbits - 3u - es),
			tag + ": significant carries an explicit hidden bit", reportTestCases);

		return fails;
	}

	// and the decomposition has to reproduce the value it came from
	template<unsigned nbits, unsigned es>
	int VerifyDecomposition(const std::string& tag, bool reportTestCases) {
		using P = posit<nbits, es>;
		int fails = 0;
		int reported = 0;

		for (unsigned long long raw = 0; raw < (1ull << nbits); ++raw) {
			P p; p.setbits(raw);
			if (p.isnar() || p.iszero()) continue;
			const double rebuilt = reconstruct(p);
			if (rebuilt != double(p)) {
				++fails;
				if (reportTestCases && reported++ < 5) {
					std::cout << "    FAIL " << tag << " encoding " << raw
					          << ": reconstructed " << rebuilt << ", value is " << double(p) << '\n';
				}
			}
		}
		return fails;
	}

}  // anonymous namespace

#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "posit attribute accessors (#1592)";
	std::string test_tag    = "posit attributes";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	{
		posit<32, 2> p(3.14159f);
		std::cout << "posit<32,2>(3.14159)\n";
		std::cout << "  scale          : " << scale(p) << '\n';
		std::cout << "  regime scale   : " << regime_scale(p) << '\n';
		std::cout << "  exponent scale : " << exponent_scale(p) << '\n';
		std::cout << "  reconstructed  : " << reconstruct(p) << "  (value " << double(p) << ")\n";
	}
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyAccessorsInstantiate<8, 2>("posit<8,2>", reportTestCases),
		test_tag, "accessors instantiate, posit<8,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyAccessorsInstantiate<16, 2>("posit<16,2>", reportTestCases),
		test_tag, "accessors instantiate, posit<16,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyAccessorsInstantiate<32, 2>("posit<32,2>", reportTestCases),
		test_tag, "accessors instantiate, posit<32,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyDecomposition<8, 2>("posit<8,2>", reportTestCases),
		test_tag, "decomposition rebuilds the value, posit<8,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyDecomposition<10, 2>("posit<10,2>", reportTestCases),
		test_tag, "decomposition rebuilds the value, posit<10,2>");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyAccessorsInstantiate<8, 0>("posit<8,0>", reportTestCases),
		test_tag, "accessors instantiate, posit<8,0>");
	nrOfFailedTestCases += ReportTestResult(VerifyAccessorsInstantiate<16, 1>("posit<16,1>", reportTestCases),
		test_tag, "accessors instantiate, posit<16,1>");
	nrOfFailedTestCases += ReportTestResult(VerifyAccessorsInstantiate<64, 3>("posit<64,3>", reportTestCases),
		test_tag, "accessors instantiate, posit<64,3>");
	nrOfFailedTestCases += ReportTestResult(VerifyDecomposition<12, 2>("posit<12,2>", reportTestCases),
		test_tag, "decomposition rebuilds the value, posit<12,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyDecomposition<14, 1>("posit<14,1>", reportTestCases),
		test_tag, "decomposition rebuilds the value, posit<14,1>");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyDecomposition<16, 2>("posit<16,2>", reportTestCases),
		test_tag, "decomposition rebuilds the value, posit<16,2>");
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
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
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
