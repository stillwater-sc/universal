// limb_layout.cpp: areal results must not depend on the limb type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_suite.hpp>

/*
   The limb type is a storage choice: areal<32,8,uint8_t> and areal<32,8,uint32_t> hold the
   same values and must compute the same results. Where the exponent field straddles a limb
   boundary, + - * came out wrong across the whole range (#1506): areal<10,3,uint8_t> gave
   3 + 5 = [0.03125], and areal<32,8,uint8_t> gave (0, minpos) for everything. scale() read
   the straddling field through a Signed blockbinary, whose int() sign-extends, so every code
   with its top bit set was taken as code - 2^es; and it counted a subnormal's scale down from
   -1 instead of from MIN_EXP_NORMAL - 1. Division stayed close only because it subtracts the
   two operands' scales, where the constant error cancels.

   scale() is checked against ilogb of the exact value for every encoding of configurations
   with and without a straddling exponent. The four operators are then checked for limb
   invariance: random operand pairs, computed over uint8_t limbs and over one wide limb, must
   give the same encoding bit for bit. static/range/areal/conversion/saturation.cpp checks
   + - * against an exact reference on straddling configurations as well.
*/

namespace sw {
namespace universal {

namespace limb_layout {

// scale() of every finite encoding with a non-zero value, against ilogb of that value
template<typename A>
int VerifyScale(bool reportTestCases) {
	int fails = 0;
	for (std::uint64_t bits = 0; bits < (std::uint64_t(1) << A::nbits); ++bits) {
		A a;
		a.setbits(bits);
		if (a.isnan() || a.isinf())
			continue;
		const double v = double(a);
		if (v == 0.0)
			continue;  // zero, and (0, minpos), whose lower bound is zero
		const int expected = std::ilogb(v);
		if (a.scale() != expected) {
			if (reportTestCases || fails < 6)
				std::cerr << "FAIL: scale() of " << to_binary(a) << ' ' << a << " is " << a.scale() << ", expected "
				          << expected << '\n';
			++fails;
		}
	}
	return fails;
}

// + - * / of random operand pairs over uint8_t limbs against the same over a single wide limb
template<unsigned nbits, unsigned es, typename WideLimb>
int VerifyLimbInvariance(unsigned samples, bool reportTestCases) {
	using Narrow = areal<nbits, es, std::uint8_t>;
	using Wide   = areal<nbits, es, WideLimb>;
	static_assert(nbits <= 64, "operands are drawn as 64-bit patterns");
	std::mt19937_64     rng(1000u * nbits + es);
	const std::uint64_t mask  = ~std::uint64_t(0) >> (64u - nbits);  // nbits low bits; the shift stays in [0, 63]
	int                 fails = 0;

	auto verify = [&](char op, std::uint64_t x, std::uint64_t y, const Narrow& n, const Wide& w) {
		const std::string nb = to_binary(n), wb = to_binary(w);
		if (nb == wb)
			return;
		if (reportTestCases || fails < 6) {
			Narrow a, b;
			a.setbits(x);
			b.setbits(y);
			std::cerr << "FAIL: " << a << ' ' << op << ' ' << b << " gave " << nb << " over uint8_t limbs, " << wb
			          << " over one limb\n";
		}
		++fails;
	};
	for (unsigned i = 0; i < samples; ++i) {
		const std::uint64_t x = rng() & mask, y = rng() & mask;
		Narrow              a, b;
		Wide                c, d;
		a.setbits(x);
		b.setbits(y);
		c.setbits(x);
		d.setbits(y);
		verify('+', x, y, a + b, c + d);
		verify('-', x, y, a - b, c - d);
		verify('*', x, y, a * b, c * d);
		verify('/', x, y, a / b, c / d);
	}
	return fails;
}

}  // namespace limb_layout

}  // namespace universal
}  // namespace sw

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
// #undef REGRESSION_LEVEL_OVERRIDE
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
	using namespace sw::universal::limb_layout;

	std::string test_suite          = "areal limb layout";
	std::string test_tag            = "limb layout";
	bool        reportTestCases     = false;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using A = areal<10, 3, std::uint8_t>;
	std::cout << "3 + 5 = " << (A(3.0) + A(5.0)) << ", 3 * 5 = " << (A(3.0) * A(5.0)) << '\n';
	nrOfFailedTestCases += ReportTestResult(VerifyScale<A>(true), "areal<10,3,uint8_t>", "scale()");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore failures
#else

#	if REGRESSION_LEVEL_1
	// scale() of every encoding: straddling exponent fields first, then single-limb ones
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyScale<areal<9, 3, std::uint8_t>>(reportTestCases), "areal< 9,3,uint8_t >", "scale()");
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyScale<areal<10, 3, std::uint8_t>>(reportTestCases), "areal<10,3,uint8_t >", "scale()");
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyScale<areal<11, 4, std::uint8_t>>(reportTestCases), "areal<11,4,uint8_t >", "scale()");
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyScale<areal<20, 8, std::uint8_t>>(reportTestCases), "areal<20,8,uint8_t >", "scale()");
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyScale<areal<10, 3, std::uint16_t>>(reportTestCases), "areal<10,3,uint16_t>", "scale()");
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyScale<areal<16, 5, std::uint8_t>>(reportTestCases), "areal<16,5,uint8_t >", "scale()");
	// + - * / over uint8_t limbs against one wide limb
	nrOfFailedTestCases += ReportTestResult(VerifyLimbInvariance<16, 5, std::uint16_t>(20000, reportTestCases),
	                                        "areal<16,5> u8 vs u16", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLimbInvariance<20, 8, std::uint32_t>(20000, reportTestCases),
	                                        "areal<20,8> u8 vs u32", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLimbInvariance<32, 8, std::uint32_t>(20000, reportTestCases),
	                                        "areal<32,8> u8 vs u32", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLimbInvariance<64, 11, std::uint64_t>(20000, reportTestCases),
	                                        "areal<64,11> u8 vs u64", test_tag);
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyScale<areal<12, 4, std::uint8_t>>(reportTestCases), "areal<12,4,uint8_t >", "scale()");
	nrOfFailedTestCases += ReportTestResult(VerifyLimbInvariance<24, 6, std::uint32_t>(100000, reportTestCases),
	                                        "areal<24,6> u8 vs u32", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyLimbInvariance<40, 11, std::uint64_t>(100000, reportTestCases),
	                                        "areal<40,11> u8 vs u64", test_tag);
#	endif

#	if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyLimbInvariance<32, 8, std::uint32_t>(1000000, reportTestCases),
	                                        "areal<32,8> u8 vs u32", test_tag);
#	endif

#	if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyLimbInvariance<64, 11, std::uint64_t>(1000000, reportTestCases),
	                                        "areal<64,11> u8 vs u64", test_tag);
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
} catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
} catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
