// minmax.cpp: test suite runner for min, max, fmin and fmax of cfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// use default number system library configuration
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

// min/max are IEEE-754-2019 minimum/maximum, fmin/fmax are C fmin/fmax (minimumNumber); all four
// order -0 below +0 and never return a signalling NaN (#1901)

// the reference operand for minimum (wantMax false) or maximum (wantMax true) of two non-NaN
// values, decided by the double values and the sign bits, independent of the cfloat compares
template<typename Cfloat>
Cfloat ReferenceMinMax(const Cfloat& x, const Cfloat& y, bool wantMax) {
	double dx = double(x), dy = double(y);
	if (dx == 0.0 && dy == 0.0) {
		// -0 < +0
		if (wantMax) return x.sign() ? y : x;
		return x.sign() ? x : y;
	}
	if (wantMax) return (dx < dy) ? y : x;
	return (dy < dx) ? y : x;
}

template<typename Cfloat>
bool SameEncoding(const Cfloat& a, const Cfloat& b) {
	return to_binary(a) == to_binary(b);
}

// every pair of encodings of a small configuration
template<typename Cfloat>
int VerifyMinMaxExhaustive(bool reportTestCases) {
	constexpr unsigned nbits = Cfloat::nbits;
	constexpr size_t NR_ENCODINGS = (size_t(1) << nbits);
	int nrOfFailedTests = 0;
	auto report = [&](const char* op, const Cfloat& x, const Cfloat& y, const Cfloat& result, const char* ref) {
		++nrOfFailedTests;
		if (reportTestCases) std::cerr << "FAIL: " << op << '(' << to_binary(x) << ", " << to_binary(y) << ") = " << to_binary(result) << " expected " << ref << '\n';
	};
	Cfloat x, y;
	for (size_t i = 0; i < NR_ENCODINGS; ++i) {
		x.setbits(i);
		for (size_t j = 0; j < NR_ENCODINGS; ++j) {
			y.setbits(j);
			Cfloat mn = min(x, y), mx = max(x, y), fmn = fmin(x, y), fmx = fmax(x, y);
			if (x.isnan() || y.isnan()) {
				// minimum/maximum propagate a quiet NaN
				if (!mn.isnan(NAN_TYPE_QUIET)) report("min", x, y, mn, "qNaN");
				if (!mx.isnan(NAN_TYPE_QUIET)) report("max", x, y, mx, "qNaN");
				// fmin/fmax return the number, or a quiet NaN when both are NaN
				if (x.isnan() && y.isnan()) {
					if (!fmn.isnan(NAN_TYPE_QUIET)) report("fmin", x, y, fmn, "qNaN");
					if (!fmx.isnan(NAN_TYPE_QUIET)) report("fmax", x, y, fmx, "qNaN");
				}
				else {
					const Cfloat& number = x.isnan() ? y : x;
					if (!SameEncoding(fmn, number)) report("fmin", x, y, fmn, "the number");
					if (!SameEncoding(fmx, number)) report("fmax", x, y, fmx, "the number");
				}
				continue;
			}
			Cfloat refMin = ReferenceMinMax(x, y, false), refMax = ReferenceMinMax(x, y, true);
			if (!SameEncoding(mn, refMin))  report("min", x, y, mn, to_binary(refMin).c_str());
			if (!SameEncoding(mx, refMax))  report("max", x, y, mx, to_binary(refMax).c_str());
			if (!SameEncoding(fmn, refMin)) report("fmin", x, y, fmn, to_binary(refMin).c_str());
			if (!SameEncoding(fmx, refMax)) report("fmax", x, y, fmx, to_binary(refMax).c_str());
		}
	}
	return nrOfFailedTests;
}

// the cases of #1901 by name, for any configuration
template<typename Cfloat>
int VerifyMinMaxSpecialCases(bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto check = [&](bool ok, const char* label, const Cfloat& result) {
		if (!ok) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: " << label << " : " << to_binary(result) << '\n';
		}
	};
	auto isquiet = [](const Cfloat& v) { return v.isnan(NAN_TYPE_QUIET); };
	Cfloat one(1), two(2), pzero(0), nzero(0), qnan, snan;
	nzero.setsign(true);
	qnan.setnan(NAN_TYPE_QUIET);
	snan.setnan(NAN_TYPE_SIGNALLING);

	// NaN handling is symmetric, and a signalling NaN never comes back
	check(isquiet(min(qnan, one)), "min(qNaN, 1)", min(qnan, one));
	check(isquiet(min(one, qnan)), "min(1, qNaN)", min(one, qnan));
	check(isquiet(max(qnan, one)), "max(qNaN, 1)", max(qnan, one));
	check(isquiet(max(one, qnan)), "max(1, qNaN)", max(one, qnan));
	check(isquiet(min(snan, one)), "min(sNaN, 1)", min(snan, one));
	check(isquiet(max(one, snan)), "max(1, sNaN)", max(one, snan));
	check(fmin(qnan, one) == one,  "fmin(qNaN, 1)", fmin(qnan, one));
	check(fmin(one, qnan) == one,  "fmin(1, qNaN)", fmin(one, qnan));
	check(fmax(snan, one) == one,  "fmax(sNaN, 1)", fmax(snan, one));
	check(fmax(one, snan) == one,  "fmax(1, sNaN)", fmax(one, snan));
	check(isquiet(fmin(snan, snan)), "fmin(sNaN, sNaN)", fmin(snan, snan));
	check(isquiet(fmax(qnan, snan)), "fmax(qNaN, sNaN)", fmax(qnan, snan));

	// -0 orders below +0, in either argument order
	Cfloat r;
	r = min(pzero, nzero); check(r.iszero() && r.sign(), "min(+0, -0)", r);
	r = min(nzero, pzero); check(r.iszero() && r.sign(), "min(-0, +0)", r);
	r = max(pzero, nzero); check(r.iszero() && !r.sign(), "max(+0, -0)", r);
	r = max(nzero, pzero); check(r.iszero() && !r.sign(), "max(-0, +0)", r);
	r = fmin(pzero, nzero); check(r.iszero() && r.sign(), "fmin(+0, -0)", r);
	r = fmax(nzero, pzero); check(r.iszero() && !r.sign(), "fmax(-0, +0)", r);

	// ordinary values, and the operand itself comes back: no rounding through double
	check(min(one, two) == one && min(two, one) == one, "min(1, 2)", min(one, two));
	check(max(one, two) == two && max(two, one) == two, "max(1, 2)", max(one, two));
	Cfloat onepp(one);
	++onepp;  // one ulp above 1
	check(min(onepp, two) == onepp, "min(1 + ulp, 2)", min(onepp, two));
	check(max(onepp, one) == onepp, "max(1 + ulp, 1)", max(onepp, one));
	check(fmin(onepp, two) == onepp, "fmin(1 + ulp, 2)", fmin(onepp, two));
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

	std::string test_suite  = "cfloat<> min/max/fmin/fmax validation";
	std::string test_tag    = "min/max";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	nrOfFailedTestCases += ReportTestResult(VerifyMinMaxSpecialCases< half >(true), "half", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyMinMaxSpecialCases< half >(reportTestCases), "half", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMinMaxSpecialCases< single >(reportTestCases), "single", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMinMaxSpecialCases< duble >(reportTestCases), "double", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMinMaxSpecialCases< quad >(reportTestCases), "quad", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMinMaxSpecialCases< cfloat< 8, 4, std::uint8_t, true, true, false> >(reportTestCases), "cfloat<8,4,sub,max>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMinMaxSpecialCases< cfloat<16, 5, std::uint8_t, true, true, false> >(reportTestCases), "cfloat<16,5,uint8_t,sub,max>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyMinMaxExhaustive< cfloat< 8, 4, std::uint8_t, true, false, false> >(reportTestCases), "cfloat<8,4,sub>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMinMaxExhaustive< cfloat< 8, 4, std::uint8_t, true, true, false> >(reportTestCases), "cfloat<8,4,sub,max>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMinMaxExhaustive< cfloat< 8, 2, std::uint8_t, true, false, false> >(reportTestCases), "cfloat<8,2,sub>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyMinMaxExhaustive< cfloat<10, 5, std::uint8_t, true, false, false> >(reportTestCases), "cfloat<10,5,uint8_t,sub>", test_tag);
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
