// saturating_overflow.cpp: saturating cfloat arithmetic at the top of the range
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// A saturating cfloat clamps an overflow to maxpos or maxneg. The conversion from the
// arithmetic's blocktriple clamped only a result that came out as the NaN pattern: one
// that rounded onto the inf pattern slipped through, so 1.5 * 3 in cfloat<4,2> gave inf,
// and every saturating configuration without max-exponent values mis-rounded products,
// sums, differences and quotients near maxpos (#1396, found through ldexp).
//
// The saturating/ directories do not catch it: most of their suites set isSaturating to
// false, and the one that does not runs in manual mode. This suite sweeps every pair of
// encodings through the four operations, against the IEEE double reference of
// cfloat_test_suite.hpp.
//
// A saturating cfloat with max-exponent values has no inf: the pattern a nonsaturating
// cfloat uses for +-inf, 0b0.11..11.11..110, is maxpos. isinf() still recognized it, so
// the largest finite value behaved as inf: maxpos / 2 was inf, double(maxpos) was inf,
// and frexp returned it unchanged (#1865). The sweep above does not see it, because it
// compares encodings and the double reference inf converts back to the same bits.
#include <universal/utility/directives.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

namespace sw { namespace universal {

template<unsigned nbits, unsigned es, bool hasSubnormals, bool hasMaxExpValues>
int VerifySaturatingArithmetic(bool reportTestCases) {
	using Cfloat = cfloat<nbits, es, uint8_t, hasSubnormals, hasMaxExpValues, true>;
	int fails = 0;
	fails += VerifyCfloatAddition<Cfloat>(reportTestCases);
	fails += VerifyCfloatSubtraction<Cfloat>(reportTestCases);
	fails += VerifyCfloatMultiplication<Cfloat>(reportTestCases);
	fails += VerifyCfloatDivision<Cfloat>(reportTestCases);
	return fails;
}

// maxpos and maxneg of a saturating cfloat with max-exponent values are finite values (#1865)
template<unsigned nbits, unsigned es, bool hasSubnormals>
int VerifyMaxposIsFinite(bool reportTestCases) {
	using Cfloat = cfloat<nbits, es, uint8_t, hasSubnormals, true, true>;
	constexpr unsigned fbits = nbits - 1u - es;
	// all-ones exponent field is the scale 2^(es-1), fraction 11..110 is the significand 2 - 2^(1-fbits)
	const double expected = std::ldexp(2.0 - std::ldexp(1.0, 1 - static_cast<int>(fbits)), 1 << (es - 1u));
	int fails = 0;
	auto check = [&](bool ok, const char* what) {
		if (!ok) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL " << type_tag(Cfloat()) << " : " << what << '\n';
		}
	};
	Cfloat maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
	check(!maxpos.isinf() && !maxneg.isinf(), "maxpos/maxneg isinf()");
	check(!maxpos.isnan() && !maxneg.isnan(), "maxpos/maxneg isnan()");
	check(double(maxpos) == expected && double(maxneg) == -expected, "double(maxpos/maxneg)");
	check(double(maxpos / Cfloat(2)) == expected / 2.0, "maxpos / 2");
	check(double(maxneg / Cfloat(2)) == -expected / 2.0, "maxneg / 2");
	Cfloat below(maxpos);
	--below;
	check(double(maxpos - below) == std::ldexp(1.0, (1 << (es - 1u)) - static_cast<int>(fbits)), "maxpos - (maxpos - 1 ulp)");
	check((maxpos - maxpos).iszero(), "maxpos - maxpos");
	int e = 0;
	Cfloat fr = frexp(maxpos, &e);
	// the fraction is in [0.5, 1), or in [1, 2) where the format cannot represent 0.5 as a normal (es <= 2)
	const int expectedExp = (1 << (es - 1u)) + (std::numeric_limits<Cfloat>::min_exponent <= 0 ? 1 : 0);
	check(double(fr) == std::ldexp(expected, -expectedExp) && e == expectedExp, "frexp(maxpos)");
	check(ldexp(fr, e) == maxpos, "ldexp(frexp(maxpos))");
	// an infinity saturates to maxpos/maxneg
	check(Cfloat(SpecificValue::infpos) == maxpos && Cfloat(SpecificValue::infneg) == maxneg, "SpecificValue::infpos/infneg");
	check(Cfloat(std::numeric_limits<double>::infinity()) == maxpos, "Cfloat(+inf)");
	check(Cfloat(-std::numeric_limits<double>::infinity()) == maxneg, "Cfloat(-inf)");
	check(!(maxpos * Cfloat(2)).isinf() && (maxpos * Cfloat(2)) == maxpos, "maxpos * 2 saturates");
	check(!std::numeric_limits<Cfloat>::has_infinity, "numeric_limits::has_infinity");
	check(double(std::numeric_limits<Cfloat>::max()) == expected, "numeric_limits::max()");
	return fails;
}

}} // namespace sw::universal

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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "saturating cfloat arithmetic at the top of the range";
	std::string test_tag    = "saturating arithmetic";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		using Cfloat = cfloat<4, 2, uint8_t, false, false, true>;
		Cfloat a(1.5f), b(3.0f);
		std::cout << a << " * " << b << " = " << (a * b) << " (expected maxpos " << Cfloat(SpecificValue::maxpos) << ")\n";
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	// every pair of encodings, +, -, *, /
	nrOfFailedTestCases += ReportTestResult(VerifySaturatingArithmetic<4, 2, false, false>(reportTestCases), "cfloat<4,2,f,f,t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturatingArithmetic<6, 2, false, false>(reportTestCases), "cfloat<6,2,f,f,t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturatingArithmetic<8, 2, false, false>(reportTestCases), "cfloat<8,2,f,f,t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturatingArithmetic<8, 3, false, false>(reportTestCases), "cfloat<8,3,f,f,t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturatingArithmetic<8, 4, false, false>(reportTestCases), "cfloat<8,4,f,f,t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturatingArithmetic<8, 5, false, false>(reportTestCases), "cfloat<8,5,f,f,t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturatingArithmetic<4, 2, true,  false>(reportTestCases), "cfloat<4,2,t,f,t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturatingArithmetic<6, 2, true,  false>(reportTestCases), "cfloat<6,2,t,f,t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturatingArithmetic<8, 2, true,  false>(reportTestCases), "cfloat<8,2,t,f,t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturatingArithmetic<8, 3, true,  false>(reportTestCases), "cfloat<8,3,t,f,t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturatingArithmetic<8, 4, true,  false>(reportTestCases), "cfloat<8,4,t,f,t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturatingArithmetic<8, 5, true,  false>(reportTestCases), "cfloat<8,5,t,f,t>", test_tag);
	// with max-exponent values the overflow never reached the inf pattern; kept as a guard
	nrOfFailedTestCases += ReportTestResult(VerifySaturatingArithmetic<8, 3, false, true >(reportTestCases), "cfloat<8,3,f,t,t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturatingArithmetic<8, 3, true,  true >(reportTestCases), "cfloat<8,3,t,t,t>", test_tag);

	// with max-exponent values, maxpos is the inf pattern of a nonsaturating cfloat and is finite
	nrOfFailedTestCases += ReportTestResult(VerifyMaxposIsFinite< 8, 3, false>(reportTestCases), "cfloat<8,3,f,t,t>", "maxpos is finite");
	nrOfFailedTestCases += ReportTestResult(VerifyMaxposIsFinite< 8, 2, true >(reportTestCases), "cfloat<8,2,t,t,t>", "maxpos is finite");
	nrOfFailedTestCases += ReportTestResult(VerifyMaxposIsFinite< 8, 3, true >(reportTestCases), "cfloat<8,3,t,t,t>", "maxpos is finite");
	nrOfFailedTestCases += ReportTestResult(VerifyMaxposIsFinite<16, 5, true >(reportTestCases), "cfloat<16,5,t,t,t>", "maxpos is finite");
	nrOfFailedTestCases += ReportTestResult(VerifyMaxposIsFinite<32, 8, false>(reportTestCases), "cfloat<32,8,f,t,t>", "maxpos is finite");
	nrOfFailedTestCases += ReportTestResult(VerifyMaxposIsFinite<32, 8, true >(reportTestCases), "cfloat<32,8,t,t,t>", "maxpos is finite");
	nrOfFailedTestCases += ReportTestResult(VerifyMaxposIsFinite<40, 8, true >(reportTestCases), "cfloat<40,8,t,t,t>", "maxpos is finite");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifySaturatingArithmetic<9, 3, false, false>(reportTestCases), "cfloat<9,3,f,f,t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturatingArithmetic<9, 3, true,  false>(reportTestCases), "cfloat<9,3,t,f,t>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifySaturatingArithmetic<10, 4, false, false>(reportTestCases), "cfloat<10,4,f,f,t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturatingArithmetic<10, 4, true,  false>(reportTestCases), "cfloat<10,4,t,f,t>", test_tag);
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
