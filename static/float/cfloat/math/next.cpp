// next.cpp: test suite runner for ULP functions nextafter, nextforward
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default number system library configuration
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_reporters.hpp>
#include <universal/verification/test_suite_mathlib.hpp>
#include <cerrno>

namespace sw { namespace universal {

// the special cases of #1901: NaN operands, stepping off either zero, overflow, and nexttoward
// stepping by x's own ulp
template<typename Cfloat>
int VerifyNextafterSpecialCases(bool reportTestCases) {
	using Wide = cfloat<128, 15, typename Cfloat::BlockType, Cfloat::hasSubnormals, Cfloat::hasMaxExpValues, Cfloat::isSaturating>;
	int nrOfFailedTests = 0;
	auto check = [&](bool ok, const char* label, const Cfloat& result) {
		if (!ok) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << " : " << to_binary(result) << '\n';
		}
	};
	auto isquiet = [](const Cfloat& v) { return v.isnan(NAN_TYPE_QUIET); };

	Cfloat one(1), pzero(0), nzero(0), qnan, snan, mp, mn, maxp, pinf;
	nzero.setsign(true);
	qnan.setnan(NAN_TYPE_QUIET);
	snan.setnan(NAN_TYPE_SIGNALLING);
	mp.minpos();
	mn.minneg();
	maxp.maxpos();
	pinf.setinf(false);
	Cfloat onepp(one), onemm(one);
	++onepp;
	--onemm;

	// a NaN operand gives a quiet NaN, in either position
	check(isquiet(nextafter(one, qnan)),  "nextafter(1, qNaN)", nextafter(one, qnan));
	check(isquiet(nextafter(one, snan)),  "nextafter(1, sNaN)", nextafter(one, snan));
	check(isquiet(nextafter(qnan, one)),  "nextafter(qNaN, 1)", nextafter(qnan, one));
	check(isquiet(nextafter(snan, one)),  "nextafter(sNaN, 1)", nextafter(snan, one));
	check(isquiet(nextafter(qnan, qnan)), "nextafter(qNaN, qNaN)", nextafter(qnan, qnan));

	// stepping off either zero, and the increment operators that do it
	Cfloat r;
	r = nextafter(nzero, one); check(r == mp && !r.sign(), "nextafter(-0, 1)", r);
	r = nextafter(pzero, one); check(r == mp && !r.sign(), "nextafter(+0, 1)", r);
	r = nextafter(pzero, -one); check(r == mn, "nextafter(+0, -1)", r);
	r = nextafter(nzero, -one); check(r == mn, "nextafter(-0, -1)", r);
	r = nzero; ++r; check(r == mp && !r.sign(), "++(-0)", r);
	r = pzero; ++r; check(r == mp && !r.sign(), "++(+0)", r);
	r = nzero; --r; check(r == mn, "--(-0)", r);
	r = pzero; --r; check(r == mn, "--(+0)", r);
	r = nextafter(pzero, nzero); check(r.iszero() && r.sign(), "nextafter(+0, -0) is -0", r);

	// overflow, and back from infinity
	if (pinf.isinf()) {
		errno = 0;
		r = nextafter(maxp, pinf); check(r.isinf() && !r.sign(), "nextafter(maxpos, inf)", r);
		check(errno == ERANGE, "nextafter(maxpos, inf) sets ERANGE", r);
		errno = 0;
		r = nextafter(-maxp, -pinf); check(r.isinf() && r.sign(), "nextafter(maxneg, -inf)", r);
		check(errno == ERANGE, "nextafter(maxneg, -inf) sets ERANGE", r);
		errno = 0;
		r = nextafter(pinf, one); check(r == maxp, "nextafter(inf, 1)", r);
		check(errno == 0, "nextafter(inf, 1) leaves errno", r);
	}

	// nexttoward steps by x's own ulp
	r = nexttoward(one, Wide(2));   check(r == onepp, "nexttoward(1, 2)", r);
	r = nexttoward(one, Wide(0.5)); check(r == onemm, "nexttoward(1, 0.5)", r);
	r = nexttoward(one, Wide(1));   check(r == one, "nexttoward(1, 1)", r);
	r = nexttoward(nzero, Wide(1)); check(r == mp && !r.sign(), "nexttoward(-0, 1)", r);
	Wide wnan; wnan.setnan(NAN_TYPE_QUIET);
	r = nexttoward(one, wnan);      check(isquiet(r), "nexttoward(1, qNaN)", r);
	r = nexttoward(snan, Wide(1));  check(isquiet(r), "nexttoward(sNaN, 1)", r);

	// an x wider than the cfloat<128,15> target: 1 + 2 ulp(x) rounds to 1 in 128 bits, so a
	// compare in 128 bits would call it equal to the target 1 and return 1 instead of stepping
	if constexpr (Cfloat::fbits > Wide::fbits) {
		Cfloat up1(one), up2(one);
		++up1;
		++up2; ++up2;
		r = nexttoward(up2, Wide(1)); check(r == up1, "nexttoward(1 + 2ulp, 1) for wide x", r);
		r = nexttoward(up1, Wide(2)); check(r == up2, "nexttoward(1 + ulp, 2) for wide x", r);
	}
	return nrOfFailedTests;
}

} } // namespace sw::universal

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

	std::string test_suite  = "cfloat<> nextafter/toward validation";
	std::string test_tag    = "nextafter/toward";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	nrOfFailedTestCases += ReportTestResult(VerifyNextafter< cfloat< 16, 5, std::uint16_t, true, true, false> >(reportTestCases), "cfloat< 16, 5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNextafterSpecialCases< half >(true), "half", "special cases");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyNextafter< cfloat<  8, 2, std::uint8_t , true, true, false> >(reportTestCases), "cfloat<  8, 2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNextafter< cfloat< 16, 5, std::uint16_t, true, true, false> >(reportTestCases), "cfloat< 16, 5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNextafter< cfloat< 32, 8, std::uint32_t, true, true, false> >(reportTestCases), "cfloat< 32, 8>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNextafter< cfloat< 64,11, std::uint64_t, true, true, false> >(reportTestCases), "cfloat< 64,11>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNextafter< cfloat< 80,11, std::uint32_t, true, true, false> >(reportTestCases), "cfloat< 80,11>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNextafter< cfloat<128,15, std::uint32_t, true, true, false> >(reportTestCases), "cfloat<128,15>", test_tag);

	// NaN operands, zeros, overflow and nexttoward (#1901)
	nrOfFailedTestCases += ReportTestResult(VerifyNextafterSpecialCases< half >(reportTestCases), "half", "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifyNextafterSpecialCases< cfloat< 16, 5, std::uint8_t , true, false, false> >(reportTestCases), "cfloat< 16, 5, uint8_t>", "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifyNextafterSpecialCases< cfloat<  8, 4, std::uint8_t , true, true, false> >(reportTestCases), "cfloat<  8, 4, sub, max>", "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifyNextafterSpecialCases< cfloat<  8, 4, std::uint8_t , false, false, false> >(reportTestCases), "cfloat<  8, 4, nosub>", "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifyNextafterSpecialCases< cfloat< 16, 5, std::uint8_t , false, false, false> >(reportTestCases), "cfloat< 16, 5, uint8_t, nosub>", "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifyNextafterSpecialCases< single >(reportTestCases), "single", "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifyNextafterSpecialCases< quad >(reportTestCases), "quad", "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifyNextafterSpecialCases< cfloat< 80,11, std::uint8_t , false, false, false> >(reportTestCases), "cfloat< 80,11, uint8_t, nosub>", "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifyNextafterSpecialCases< cfloat<128,15, std::uint8_t , false, false, false> >(reportTestCases), "cfloat<128,15, uint8_t, nosub>", "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifyNextafterSpecialCases< cfloat<128,15, std::uint64_t, false, false, false> >(reportTestCases), "cfloat<128,15, uint64_t, nosub>", "special cases");
	nrOfFailedTestCases += ReportTestResult(VerifyNextafterSpecialCases< cfloat<256,19, std::uint64_t, true, false, false> >(reportTestCases), "cfloat<256,19>", "special cases");
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
