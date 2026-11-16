// logic.cpp: bfloat16 comparison operators, NaN ordering, signed zero and infinity (#2106)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// operator>= was !operator<, which is true when either operand is NaN: bfloat16(NaN) >= 1 was true,
// in the bfloat16 and in both float overloads. Every pair of a sample of encodings, NaNs, zeros and
// infinities included, is compared with all six operators against the same comparison of the
// floats, which hold every bfloat16 value exactly and follow IEEE 754.
#include <universal/utility/directives.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/verification/test_suite.hpp>

#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {

	using sw::universal::bfloat16;

	bfloat16 from_bits(unsigned b) {
		bfloat16 x;
		x.setbits(b);
		return x;
	}

	int VerifyComparisons(bool reportTestCases) {
		std::vector<unsigned> enc = { 0x0000u, 0x8000u, 0x0001u, 0x8001u, 0x007Fu, 0x0080u, 0x3F80u, 0xBF80u, 0x3F81u,
		                              0x7F7Fu, 0xFF7Fu, 0x7F80u, 0xFF80u, 0x7FC0u, 0xFFC0u, 0x7F81u, 0xFF81u, 0x4000u, 0xC000u };
		for (unsigned b = 0x0000u; b < 0x10000u; b += 0x0FBu) enc.push_back(b);   // a spread over every exponent
		int fails = 0;
		auto report = [&](const char* op, unsigned a, unsigned b) {
			++fails;
			if (reportTestCases && fails < 16) std::cerr << "FAIL 0x" << std::hex << a << ' ' << op << " 0x" << b << std::dec << '\n';
		};
		for (unsigned ea : enc) {
			for (unsigned eb : enc) {
				const bfloat16 a = from_bits(ea), b = from_bits(eb);
				const float fa = float(a), fb = float(b);
				if ((a == b) != (fa == fb)) report("==", ea, eb);
				if ((a != b) != (fa != fb)) report("!=", ea, eb);
				if ((a <  b) != (fa <  fb)) report("<",  ea, eb);
				if ((a >  b) != (fa >  fb)) report(">",  ea, eb);
				if ((a <= b) != (fa <= fb)) report("<=", ea, eb);
				if ((a >= b) != (fa >= fb)) report(">=", ea, eb);
				// the mixed overloads, with the float on either side
				if ((a >= fb) != (fa >= fb)) report(">= float", ea, eb);
				if ((fa >= b) != (fa >= fb)) report("float >=", ea, eb);
				if ((a <= fb) != (fa <= fb)) report("<= float", ea, eb);
				if ((fa <= b) != (fa <= fb)) report("float <=", ea, eb);
				if ((a < fb) != (fa < fb)) report("< float", ea, eb);
				if ((fa > b) != (fa > fb)) report("float >", ea, eb);
			}
		}
		return fails;
	}

	int VerifySpecialCases(bool reportTestCases) {
		int fails = 0;
		auto expect = [&](bool ok, const char* what) { if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << what << '\n'; } };
		const bfloat16 nan(sw::universal::SpecificValue::qnan), one(1.0f), zero(0.0f), nzero(-0.0f);
		const bfloat16 inf(sw::universal::SpecificValue::infpos), ninf(sw::universal::SpecificValue::infneg);
		expect(!(nan >= one) && !(one >= nan) && !(nan >= nan), "NaN >= is false");
		expect(!(nan >= 1.0f) && !(1.0f >= nan), "NaN >= float is false");
		expect(zero == nzero && zero >= nzero && nzero >= zero && !(zero < nzero), "+0 and -0 are equal");
		expect(inf > one && ninf < one && inf >= inf && !(inf > inf), "infinities order");
		return fails;
	}

}  // namespace

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

	std::string test_suite  = "bfloat16 logic (#2106)";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyComparisons(reportTestCases), "bfloat16", "six operators against float");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialCases(reportTestCases), "bfloat16", "NaN, signed zero, inf");
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
