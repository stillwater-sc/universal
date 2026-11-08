// pow.cpp: test suite runner for power functions for ereal adaptive-precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <random>
#include <string>
#include <string_view>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/ereal_precision.hpp>
#include <math/constants/reference_constants.hpp>

namespace {
	using namespace sw::universal;

	// Precision (#2008, #2006). Every value is checked exactly, against a decimal reference
	// or another ereal, by the dyadic oracle in ereal_precision.hpp: nothing passes through a
	// double, so a result correct to only 53 bits fails. Each check asks for the bits the
	// configuration holds, ereal_bits_of<Real>::value, less kMarginBits.
	//
	// Measured (2026-11, gcc -O2) at ereal<8>/<16>/<19>, bits held 424/848/1007: the
	// integer powers are exact, and every other check reaches at least 431/876/1016 bits (a
	// result may carry up to twice its limbs, so it can exceed the bits held). The smallest
	// surplus is 7/28/9 bits, pow(e, 2) with e parsed from its decimal string (the parsed e
	// itself agrees to 431/876/1016). A margin of 16 bits keeps every check passing with room
	// for platform differences in the last limb, while asking for 408/832/991 bits: far
	// beyond the 53 a double-only result has. The arguments here are small (|y log x| < 25);
	// the error of exp(y log x) grows with that product, so a test of large exponents would
	// need a wider, stated margin.
	constexpr int kMarginBits = 16;

	template<typename Real>
	constexpr int wanted_bits() { return ereal_bits_of<Real>::value - kMarginBits; }

	// The widest double-limb configuration (maxlimbs <= 19), the reference for values that
	// have no stored constant.
	using WideReal = ereal<19, double>;

	// References computed with mpmath at 360 digits, printed to 330 significant digits.
	// The arguments are the doubles the tests pass, converted exactly (mpf(3.141592653589793)
	// is the double nearest pi, not pi).
	//   kExp2      = exp(2)                      (e^2)
	//   kPow2PiD   = power(2, mpf(3.141592653589793))
	//   kPow10_1p5 = power(10, mpf(1.5))         (10 sqrt(10))
	constexpr std::string_view kExp2 =
		"7.389056098930650227230427460575007813180315570551847324087127822522573796079057"
		"76338431248507912179477375316126547886612388460369278127337447839221339807777490"
		"01228956074107537023913309475506820865818202696478682084042209822552348757424625"
		"41414679928129331888070763301019337899740729986960095303307515320818823684694793"
		"02991355877";
	constexpr std::string_view kPow2PiD =
		"8.824977827076286874738941696579312098341913251408856142094696125424247390204572"
		"18168287439242013534420484369519650361914738890224893268498136277392972233760073"
		"28000367445055955983696415659781066306686397422905089473717236697741918345857256"
		"76622185696633649607135006166708849371720338468649992340713401104207230464496108"
		"46924613252";
	constexpr std::string_view kPow10_1p5 =
		"31.62277660168379331998893544432718533719555139325216826857504852792594438639238"
		"22134424810837930029518734728415284005514854885603045388001469051959670015390334"
		"49216571792599406591501534741133394841240853169295770904715764610443692578790620"
		"37808609941828371711548406328552999118596824564203326961604691314336128949791890"
		"26652954361";

	// Bits measured for a failure message; -1 where the oracle has no value to compare.
	template<typename Real>
	int measured_bits(const Real& got, std::string_view ref) {
		if (got.isnan() || got.isinf() || got.iszero()) return -1;
		return agreed_bits(got, ref);
	}
	template<typename Real, typename Ref>
	int measured_bits(const Real& got, const Ref& ref) {
		if (got.isnan() || got.isinf() || ref.isnan() || ref.isinf() || ref.iszero()) return -1;
		return agreed_bits(got, ref);
	}

	template<typename Real>
	int expect_bits(const Real& got, std::string_view ref, int bits, const std::string& what, bool reportTestCases) {
		if (agrees_to_bits(got, ref, bits)) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << ": " << measured_bits(got, ref) << " bits, wanted " << bits << '\n';
		return 1;
	}
	template<typename Real, typename Ref>
	int expect_bits(const Real& got, const Ref& ref, int bits, const std::string& what, bool reportTestCases) {
		if (agrees_to_bits(got, ref, bits)) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << ": " << measured_bits(got, ref) << " bits, wanted " << bits << '\n';
		return 1;
	}

		// Verify pow function - special cases
		template<typename Real>
		int VerifyPowSpecialCases(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			constexpr int bits = wanted_bits<Real>();

			// x^0 = 1, x^1 = x, 1^y = 1: exact results, checked to the configuration
			nrOfFailedTestCases += expect_bits(pow(Real(5.0), Real(0.0)), Real(1.0), bits, "pow(5, 0) vs 1", reportTestCases);
			nrOfFailedTestCases += expect_bits(pow(Real(5.0), Real(1.0)), Real(5.0), bits, "pow(5, 1) vs 5", reportTestCases);
			nrOfFailedTestCases += expect_bits(pow(Real(1.0), Real(42.0)), Real(1.0), bits, "pow(1, 42) vs 1", reportTestCases);

			// Test: 0^y = +0 for y > 0
			Real result = pow(Real(0.0), Real(2.0));
			if (!result.iszero() || std::signbit(double(result))) {
				if (reportTestCases) std::cout << "    FAIL pow(0, 2) != +0\n";
				++nrOfFailedTestCases;
			}

			return nrOfFailedTestCases;
		}

		// Verify pow function - integer powers
		template<typename Real>
		int VerifyPowIntegerPowers(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			constexpr int bits = wanted_bits<Real>();

			nrOfFailedTestCases += expect_bits(pow(Real(2.0), Real(3.0)), Real(8.0), bits, "pow(2, 3) vs 8", reportTestCases);
			nrOfFailedTestCases += expect_bits(pow(Real(10.0), Real(2.0)), Real(100.0), bits, "pow(10, 2) vs 100", reportTestCases);
			nrOfFailedTestCases += expect_bits(pow(Real(3.0), Real(4.0)), Real(81.0), bits, "pow(3, 4) vs 81", reportTestCases);
			nrOfFailedTestCases += expect_bits(pow(Real(2.0), Real(-1.0)), Real(0.5), bits, "pow(2, -1) vs 0.5", reportTestCases);
			// 0.01 has no finite binary value: compare against the decimal itself, not double(0.01)
			nrOfFailedTestCases += expect_bits(pow(Real(10.0), Real(-2.0)), std::string_view("0.01"), bits, "pow(10, -2) vs 0.01", reportTestCases);

			return nrOfFailedTestCases;
		}

		// Verify pow function - large integer exponents and negative bases
		template<typename Real>
		int VerifyPowLargeIntegerAndNegativeBases(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			constexpr int bits = wanted_bits<Real>();

			// Every expected value below is an integer or a power of two, exact in a double, so
			// Real(expected) is the exact reference.

			// Test 1: Large positive integer exponent with negative base: (-2)^15 = -32768
			nrOfFailedTestCases += expect_bits(pow(Real(-2.0), Real(15.0)), Real(-32768.0), bits, "pow(-2, 15) vs -32768", reportTestCases);

			// Test 2: Large negative integer exponent with negative base: (-2)^(-10) = 1/1024
			nrOfFailedTestCases += expect_bits(pow(Real(-2.0), Real(-10.0)), Real(1.0 / 1024.0), bits, "pow(-2, -10) vs 1/1024", reportTestCases);

			// Test 3: Even integer exponent with negative base (result positive): (-3)^20 = 3486784401
			nrOfFailedTestCases += expect_bits(pow(Real(-3.0), Real(20.0)), Real(3486784401.0), bits, "pow(-3, 20) vs 3486784401", reportTestCases);

			// Test 4: Odd integer exponent with negative base (result negative): (-3)^21 = -10460353203
			nrOfFailedTestCases += expect_bits(pow(Real(-3.0), Real(21.0)), Real(-10460353203.0), bits, "pow(-3, 21) vs -10460353203", reportTestCases);

			// Test 5: Non-integer exponent with negative base should return NaN
			// (-2)^2.5 should be NaN (complex result)
			{
				Real x(-2.0), y(2.5);
				Real result = pow(x, y);
				double result_double = double(result);
				if (!std::isnan(result_double)) {
					if (reportTestCases) std::cout << "    FAIL pow(-2, 2.5) = "
						<< result_double << ", expected NaN\n";
					++nrOfFailedTestCases;
				}
			}

			// Test 6: Very large integer exponent (within int range): 2^30 = 1073741824
			nrOfFailedTestCases += expect_bits(pow(Real(2.0), Real(30.0)), Real(1073741824.0), bits, "pow(2, 30) vs 2^30", reportTestCases);

			// Test 7: Exponent just outside old [-10, 10] limit
			// (-2)^11 = -2048 (previously would have returned NaN)
			nrOfFailedTestCases += expect_bits(pow(Real(-2.0), Real(11.0)), Real(-2048.0), bits, "pow(-2, 11) vs -2048", reportTestCases);

			// Test 8: Negative base with integer exponent = 0 (should be 1)
			nrOfFailedTestCases += expect_bits(pow(Real(-5.0), Real(0.0)), Real(1.0), bits, "pow(-5, 0) vs 1", reportTestCases);

			return nrOfFailedTestCases;
		}

		// Verify pow function - fractional powers
		template<typename Real>
		int VerifyPowFractionalPowers(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			constexpr int bits = wanted_bits<Real>();

			// Test: 4^0.5 = 2 (square root)
			nrOfFailedTestCases += expect_bits(pow(Real(4.0), Real(0.5)), Real(2.0), bits, "pow(4, 0.5) vs 2", reportTestCases);

			// Test: 8^(1/3) ~= 2 (cube root). The exponent is 1/3 to the configuration, so the
			// result is 2 to the configuration (d(8^y)/dy = 8^y ln8 is small).
			nrOfFailedTestCases += expect_bits(pow(Real(8.0), Real(1.0) / Real(3.0)), Real(2.0), bits, "pow(8, 1/3) vs 2", reportTestCases);

			// Test: 2^0.5 = sqrt(2): stored reference
			nrOfFailedTestCases += expect_bits(pow(Real(2.0), Real(0.5)), s_sqrt2, bits, "pow(2, 0.5) vs sqrt2", reportTestCases);

			return nrOfFailedTestCases;
		}

		// Verify pow function - general powers
		template<typename Real>
		int VerifyPowGeneralPowers(bool reportTestCases) {
			int nrOfFailedTestCases = 0;
			constexpr int bits = wanted_bits<Real>();

			// Test: 2^pi, with pi the double 3.141592653589793 (mpmath reference for that double)
			nrOfFailedTestCases += expect_bits(pow(Real(2.0), Real(3.141592653589793)), kPow2PiD, bits, "pow(2, double(pi))", reportTestCases);

			// Test: e^2 = exp(2). The base must be e to the configuration's precision: the
			// double std::exp(1.0) is e only to 53 bits, and so would be its square.
			Real e;
			e.parse(std::string(s_e));
			nrOfFailedTestCases += expect_bits(pow(e, Real(2.0)), kExp2, bits, "pow(e, 2) vs e^2", reportTestCases);

			// Test: 10^1.5 = 10 sqrt(10)
			nrOfFailedTestCases += expect_bits(pow(Real(10.0), Real(1.5)), kPow10_1p5, bits, "pow(10, 1.5)", reportTestCases);

			return nrOfFailedTestCases;
		}

		// Property fuzzer over the positive domain: pow(x,2)==x*x, pow(x,0.5)==
		// sqrt(x), the pow(x,a)*pow(x,b)==pow(x,a+b) exponent law, and the
		// pow(x,0)==1 / pow(x,1)==x fixed points, checked in ereal itself. Since a
		// self-consistent loss of precision would satisfy them, pow(x,1.5) is also checked
		// against pow at the widest configuration, on every fourth sample (the ereal<19> pow
		// dominates an iteration's cost).
		template<typename Real>
		int VerifyPowFuzz(bool reportTestCases, unsigned nrIterations) {
			int nrOfFailedTestCases = 0;
			constexpr int bits = wanted_bits<Real>();
			std::mt19937_64 rng(0xC1A55'1FFEULL);
			std::uniform_real_distribution<double> dist(0.1, 50.0);
			constexpr bool hasWiderReference = ereal_bits_of<Real>::value < ereal_bits_of<WideReal>::value;
			Real one(1.0), two(2.0), half(0.5), threeHalves(1.5);
			for (unsigned i = 0; i < nrIterations; ++i) {
				double dx = dist(rng);
				Real x(dx);
				const std::string at = " at x=" + std::to_string(dx);
				Real p15 = pow(x, threeHalves);
				nrOfFailedTestCases += expect_bits(pow(x, two), x * x, bits, "pow(x,2)==x*x" + at, reportTestCases);
				nrOfFailedTestCases += expect_bits(pow(x, half), sqrt(x), bits, "pow(x,0.5)==sqrt(x)" + at, reportTestCases);
				nrOfFailedTestCases += expect_bits(p15 * pow(x, half), pow(x, two), bits, "pow exponent law" + at, reportTestCases);
				if constexpr (hasWiderReference) {
					if (i % 4 == 0) nrOfFailedTestCases += expect_bits(p15, pow(WideReal(dx), WideReal(1.5)), bits, "pow(x,1.5) vs ereal<19>" + at, reportTestCases);
				}
				// fixed points: checked to the configuration -- the general pow path
				// need not return exactly 1 / x on every platform.
				nrOfFailedTestCases += expect_bits(pow(x, Real(0.0)), one, bits, "pow(x,0)==1" + at, reportTestCases);
				nrOfFailedTestCases += expect_bits(pow(x, one), x, bits, "pow(x,1)==x" + at, reportTestCases);
			}
			return nrOfFailedTestCases;
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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "ereal mathlib power function validation";
	std::string test_tag    = "power";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Manual test cases for visual verification
	std::cout << "Manual testing of power functions:\n";
	std::cout << "pow(2, 3) = " << double(pow(ereal<>(2.0), ereal<>(3.0))) << " (expected: 8)\n";
	std::cout << "pow(4, 0.5) = " << double(pow(ereal<>(4.0), ereal<>(0.5))) << " (expected: 2)\n";
	std::cout << "pow(e, 2) = " << double(pow(ereal<>(std::exp(1.0)), ereal<>(2.0))) << " (expected: " << std::exp(2.0) << ")\n";

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	// Phase 4b function: pow
	test_tag = "pow special cases";
	nrOfFailedTestCases += ReportTestResult(VerifyPowSpecialCases<ereal<>>(reportTestCases), "pow(ereal) special", test_tag);

	test_tag = "pow integer powers";
	nrOfFailedTestCases += ReportTestResult(VerifyPowIntegerPowers<ereal<>>(reportTestCases), "pow(ereal) integer", test_tag);

	test_tag = "pow large integer and negative bases";
	nrOfFailedTestCases += ReportTestResult(VerifyPowLargeIntegerAndNegativeBases<ereal<>>(reportTestCases), "pow(ereal) large int", test_tag);

	test_tag = "pow fractional powers";
	nrOfFailedTestCases += ReportTestResult(VerifyPowFractionalPowers<ereal<>>(reportTestCases), "pow(ereal) fractional", test_tag);

	test_tag = "pow general powers";
	nrOfFailedTestCases += ReportTestResult(VerifyPowGeneralPowers<ereal<>>(reportTestCases), "pow(ereal) general", test_tag);

	test_tag = "pow fuzz";
	// L1 (sanity tier; run by CI's Debug-instrumented ASan/UBSan/coverage jobs) takes
	// only a small fuzz smoke sample. The count scales up for the on-demand stress
	// tiers L2-L4 so QA depth is preserved where runtime is not a concern (#1007).
	unsigned powFuzzCount = 15;
#if REGRESSION_LEVEL_4
	powFuzzCount = 2000;
#elif REGRESSION_LEVEL_3
	powFuzzCount = 500;
#elif REGRESSION_LEVEL_2
	powFuzzCount = 100;
#endif
	nrOfFailedTestCases += ReportTestResult(VerifyPowFuzz<ereal<>>(reportTestCases, powFuzzCount), "pow property fuzz", test_tag);
#endif

#if REGRESSION_LEVEL_2
	// Extended precision tests at 512 bits (~=154 decimal digits)
	test_tag = "pow special cases high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyPowSpecialCases<ereal<8>>(reportTestCases), "pow(ereal<8>) special", test_tag);

	test_tag = "pow integer powers high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyPowIntegerPowers<ereal<8>>(reportTestCases), "pow(ereal<8>) integer", test_tag);

	test_tag = "pow large integer and negative bases high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyPowLargeIntegerAndNegativeBases<ereal<8>>(reportTestCases), "pow(ereal<8>) large int", test_tag);

	test_tag = "pow fractional powers high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyPowFractionalPowers<ereal<8>>(reportTestCases), "pow(ereal<8>) fractional", test_tag);

	test_tag = "pow general powers high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyPowGeneralPowers<ereal<8>>(reportTestCases), "pow(ereal<8>) general", test_tag);
#endif

#if REGRESSION_LEVEL_3
	// High precision tests at 1024 bits (~=308 decimal digits)
	test_tag = "pow special cases very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyPowSpecialCases<ereal<16>>(reportTestCases), "pow(ereal<16>) special", test_tag);

	test_tag = "pow integer powers very high precision";
	nrOfFailedTestCases += ReportTestResult(VerifyPowIntegerPowers<ereal<16>>(reportTestCases), "pow(ereal<16>) integer", test_tag);
#endif

#if REGRESSION_LEVEL_4
	// Extreme precision tests at max 1216 bits (~=304 decimal digits, ereal<19> is max)
	test_tag = "pow special cases extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifyPowSpecialCases<ereal<19>>(reportTestCases), "pow(ereal<19>) special", test_tag);

	test_tag = "pow integer powers extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifyPowIntegerPowers<ereal<19>>(reportTestCases), "pow(ereal<19>) integer", test_tag);

	test_tag = "pow large integer and negative bases extreme precision";
	nrOfFailedTestCases += ReportTestResult(VerifyPowLargeIntegerAndNegativeBases<ereal<19>>(reportTestCases), "pow(ereal<19>) large int", test_tag);
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
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
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
