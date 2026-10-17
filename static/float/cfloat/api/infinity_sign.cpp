// infinity_sign.cpp: a positive infinity keeps its sign through normalization and printing (#1719)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// cfloat's normalize() family called blocktriple::setinf() without the sign, and that
// argument defaulted to negative.  Every infinity that went through normalize() became -inf,
// which operator<< then printed: fp32(1e300) printed "-inf" with sign bit 0.  The encoding,
// sign() and conversion to native types were always right; the arithmetic special-cases
// infinity before it normalizes.  This test pins the text and the normalized sign of both
// infinities, for cfloat and areal, whose normalize functions had the same call.
#include <universal/utility/directives.hpp>
#include <limits>
#include <sstream>
#include <string>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using namespace sw::universal;

template<typename T>
std::string text(const T& v, bool showpos = false) {
	std::stringstream s;
	if (showpos) s << std::showpos;
	s << v;
	return s.str();
}

template<typename Cfloat>
int VerifyCfloatInfinity(bool reportTestCases) {
	constexpr unsigned fbits = Cfloat::fbits;
	int fails = 0;
	auto check = [&](bool ok, const std::string& what) {
		if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL: " << type_tag(Cfloat()) << ' ' << what << '\n'; }
	};
	const Cfloat pinf(SpecificValue::infpos), ninf(SpecificValue::infneg);
	check(pinf.isinf() && !pinf.sign() && ninf.isinf() && ninf.sign(), "the encodings of +-inf");
	check(text(pinf) == "inf", "+inf prints as '" + text(pinf) + "'");
	check(text(ninf) == "-inf", "-inf prints as '" + text(ninf) + "'");
	check(text(pinf, true) == "+inf", "+inf with showpos prints as '" + text(pinf, true) + "'");
	// the overflow of a finite value is the same +inf; every configuration below is
	// non-saturating, so maxpos * 4 overflows (a saturating one would clamp to maxpos)
	static_assert(!Cfloat::isSaturating, "this check needs a non-saturating configuration");
	const Cfloat big = Cfloat(SpecificValue::maxpos) * Cfloat(4.0);
	check(big.isinf() && !big.sign(), "positive overflow produces +inf");
	check(text(big) == "inf", "positive overflow prints as '" + text(big) + "'");
	// every normalize variant keeps the sign
	for (const Cfloat& v : { pinf, ninf }) {
		blocktriple<fbits, BlockTripleOperator::REP, typename Cfloat::BlockType> r;
		blocktriple<fbits, BlockTripleOperator::ADD, typename Cfloat::BlockType> a;
		blocktriple<fbits, BlockTripleOperator::MUL, typename Cfloat::BlockType> m;
		blocktriple<fbits, BlockTripleOperator::DIV, typename Cfloat::BlockType> d;
		v.normalize(r);
		v.normalizeAddition(a);
		v.normalizeMultiplication(m);
		v.normalizeDivision(d);
		const bool want = v.sign();
		check(r.isinf() && r.sign() == want, "normalize keeps the sign");
		check(a.isinf() && a.sign() == want, "normalizeAddition keeps the sign");
		check(m.isinf() && m.sign() == want, "normalizeMultiplication keeps the sign");
		check(d.isinf() && d.sign() == want, "normalizeDivision keeps the sign");
	}
	return fails;
}

template<typename Areal>
int VerifyArealInfinity(bool reportTestCases) {
	constexpr unsigned fbits = Areal::fbits;
	int fails = 0;
	for (double inf : { std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity() }) {
		const Areal v(inf);
		blocktriple<fbits, BlockTripleOperator::ADD, typename Areal::BlockType> a;
		blocktriple<fbits, BlockTripleOperator::MUL, typename Areal::BlockType> m;
		blocktriple<fbits, BlockTripleOperator::DIV, typename Areal::BlockType> d;
		v.normalizeAddition(a);
		v.normalizeMultiplication(m);
		v.normalizeDivision(d);
		const bool want = (inf < 0);
		if (!(v.isinf() && v.sign() == want && a.sign() == want && m.sign() == want && d.sign() == want)) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL: " << type_tag(v) << " normalize of " << inf << " lost the sign\n";
		}
	}
	return fails;
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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "cfloat and areal infinity sign (#1719)";
	std::string test_tag    = "infinity sign";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatInfinity<fp32>(reportTestCases), "fp32", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatInfinity<fp16>(reportTestCases), "fp16", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatInfinity<fp32>(reportTestCases), "fp32", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatInfinity<fp64>(reportTestCases), "fp64", test_tag);
	// a non-IEEE configuration: no subnormals, no max-exponent values, a multi-block layout
	nrOfFailedTestCases += ReportTestResult((VerifyCfloatInfinity<cfloat<20, 6, std::uint8_t, false, false, false>>(reportTestCases)), "cfloat<20,6>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyArealInfinity<areal<16, 5>>(reportTestCases)), "areal<16,5>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyArealInfinity<areal<32, 8>>(reportTestCases)), "areal<32,8>", test_tag);
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
