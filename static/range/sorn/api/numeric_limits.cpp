// numeric_limits.cpp: test suite runner for std::numeric_limits of SORNs
//
// Copyright (C) 2017-2026 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/sorn/sorn.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// The oracle reads the lattice, sorn<...>::datatype(); numeric_limits computes its constants
	// from the template parameters. Before #1986 every member was a placeholder and epsilon()
	// did not compile.
	template<typename SornType>
	int VerifyNumericLimits(bool reportTestCases, const std::string& label) {
		using Limits = std::numeric_limits<SornType>;
		using Interval = typename SornType::SORN_INTERVAL;
		const float inf = std::numeric_limits<float>::infinity();
		const auto& dt = SornType::datatype();
		int nrOfFailedTestCases = 0;
		auto fail = [&](const std::string& what, const auto& got, const auto& expected) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: " << label << ' ' << what << " = " << got << ", expected " << expected << '\n';
		};
		auto sornOf = [](const Interval& interval) { SornType s; s.sornIntVal = interval; return s; };

		// lattice points: the smallest positive and the largest finite upper bound
		float smallest = inf, largest = 0.0f;
		for (const auto& i : dt) {
			if (i.upperBound > 0 && std::isfinite(i.upperBound)) {
				smallest = std::min(smallest, i.upperBound);
				largest  = std::max(largest, i.upperBound);
			}
		}
		// the width of the interval holding the smallest float above 1
		const float above = std::nextafter(1.0f, inf);
		float width = inf;
		for (const auto& i : dt) {
			if (above > i.lowerBound && above <= i.upperBound) {
				width = i.upperBound - i.lowerBound;
				break;
			}
		}
		int digits = 0;
		if (std::isfinite(width)) {
			while (width <= std::ldexp(1.0f, 1 - (digits + 1))) ++digits;
		}
		// the lowest interval with a finite lower bound
		Interval lowest = dt.front();
		for (const auto& i : dt) {
			if (std::isfinite(i.lowerBound)) { lowest = i; break; }
		}

		if (Limits::min().sornIntVal.upperBound != smallest || Limits::min().sornIntVal.lowerBound < 0) fail("min()", Limits::min(), smallest);
		if (Limits::max().sornIntVal.upperBound != largest) fail("max()", Limits::max(), largest);
		if (Limits::lowest() != sornOf(lowest)) fail("lowest()", Limits::lowest(), sornOf(lowest));
		if (Limits::epsilon() != SornType(width)) fail("epsilon()", Limits::epsilon(), SornType(width));
		if (Limits::round_error() != SornType(width / 2.0f)) fail("round_error()", Limits::round_error(), SornType(width / 2.0f));
		if (Limits::denorm_min() != Limits::min()) fail("denorm_min()", Limits::denorm_min(), Limits::min());
		if (Limits::infinity() != sornOf(dt.back())) fail("infinity()", Limits::infinity(), sornOf(dt.back()));
		if (!Limits::quiet_NaN().isnan()) fail("quiet_NaN()", Limits::quiet_NaN(), "nan");
		if (!Limits::signaling_NaN().isnan()) fail("signaling_NaN()", Limits::signaling_NaN(), "nan");

		if (Limits::digits != digits) fail("digits", Limits::digits, digits);
		if (Limits::min_exponent != std::ilogb(smallest) + 1) fail("min_exponent", Limits::min_exponent, std::ilogb(smallest) + 1);
		if (Limits::max_exponent != std::ilogb(largest) + 1) fail("max_exponent", Limits::max_exponent, std::ilogb(largest) + 1);
		const int e10min = static_cast<int>(std::ceil(std::log10(double(smallest))));
		const int e10max = static_cast<int>(std::floor(std::log10(double(largest))));
		if (Limits::min_exponent10 != e10min) fail("min_exponent10", Limits::min_exponent10, e10min);
		if (Limits::max_exponent10 != e10max) fail("max_exponent10", Limits::max_exponent10, e10max);
		const int d10 = (digits > 1 ? static_cast<int>(std::floor((digits - 1) * std::log10(2.0))) : 0);
		const int md10 = static_cast<int>(std::ceil(1 + digits * std::log10(2.0)));
		if (Limits::digits10 != d10) fail("digits10", Limits::digits10, d10);
		if (Limits::max_digits10 != md10) fail("max_digits10", Limits::max_digits10, md10);

		if (Limits::has_infinity != std::isinf(dt.back().upperBound)) fail("has_infinity", Limits::has_infinity, std::isinf(dt.back().upperBound));
		if (Limits::is_signed != (dt.front().lowerBound < 0)) fail("is_signed", Limits::is_signed, dt.front().lowerBound < 0);
		if (!Limits::is_specialized || !Limits::has_quiet_NaN || Limits::has_signaling_NaN || Limits::is_integer ||
		    Limits::is_exact || Limits::is_iec559 || !Limits::is_bounded || Limits::is_modulo || Limits::traps ||
		    Limits::tinyness_before || Limits::has_denorm_loss || Limits::radix != 2 ||
		    Limits::has_denorm != std::denorm_absent || Limits::round_style != std::round_indeterminate) {
			fail("flags", "mismatch", "sorn flags");
		}
		return nrOfFailedTestCases;
	}

	// the values for sorn<0,4,8>, spelled out
	int VerifyLinearValues(bool reportTestCases) {
		using S = sorn<0, 4, 8>;   // [-inf,-4) ... [-0.5,0) 0 (0,0.5] ... (4,inf]
		using L = std::numeric_limits<S>;
		int nrOfFailedTestCases = 0;
		auto check = [&](const auto& got, const std::string& expected, const char* what) {
			std::stringstream s;
			s << got;
			if (s.str() != expected) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: sorn<0,4,8> " << what << " = " << s.str() << ", expected " << expected << '\n';
			}
		};
		check(L::min(),         "(0,0.5]",   "min()");
		check(L::max(),         "(3.5,4]",   "max()");
		check(L::lowest(),      "[-4,-3.5)", "lowest()");
		check(L::epsilon(),     "(0,0.5]",   "epsilon()");      // width of (1,1.5]
		check(L::round_error(), "(0,0.5]",   "round_error()");
		check(L::infinity(),    "(4,inf]",   "infinity()");
		check(L::digits,        "2",         "digits");          // 0.5 <= 2^(1-2)
		check(L::min_exponent,  "0",         "min_exponent");    // 0.5 = 2^(0-1)
		check(L::max_exponent,  "3",         "max_exponent");    // 4 < 2^3
		return nrOfFailedTestCases;
	}

}}  // namespace sw::universal

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

	std::string test_suite  = "sorn numeric_limits validation";
	std::string test_tag    = "numeric_limits";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using LinSorn     = sorn<0, 4, 8>;                       // linear with infinity
	using ThirdsSorn  = sorn<0, 3, 8>;                       // linear, step 0.375
	using LogSorn     = sorn<-2, 2, 1, 0, 1, 1, 1, 1>;       // logarithmic with infinity
	using HighLogSorn = sorn<1, 4, 1, 0, 1, 1, 1, 1>;        // logarithmic, first interval (0,2]
	using NoZeroSorn  = sorn<0, 4, 4, 1, 1, 1, 1, 0>;        // linear, no exact zero
	using PosSorn     = sorn<0, 2, 2, 1, 1, 0, 0, 1>;        // 0 (0,1] (1,2]: no negatives, no infinity
	using PosNoZero   = sorn<0, 2, 2, 1, 1, 0, 0, 0>;        // [0,1] (1,2]
	using UnitSorn    = sorn<0, 1, 4>;                       // nothing finite above 1
	using TinySorn    = sorn<-149, -148, 1, 0, 1, 0, 0, 1>;  // 0 (0,2^-149] (2^-149,2^-148]

#if MANUAL_TESTING

	using L = std::numeric_limits<LinSorn>;
	std::cout << L::epsilon() << ' ' << L::digits << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<LinSorn>(reportTestCases, "sorn<0,4,8>"), "sorn<0,4,8>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<ThirdsSorn>(reportTestCases, "sorn<0,3,8>"), "sorn<0,3,8>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<LogSorn>(reportTestCases, "sorn<-2,2,1,log>"), "sorn<-2,2,1,log>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<HighLogSorn>(reportTestCases, "sorn<1,4,1,log>"), "sorn<1,4,1,log>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<NoZeroSorn>(reportTestCases, "sorn<0,4,4,nozero>"), "sorn<0,4,4,nozero>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<PosSorn>(reportTestCases, "sorn<0,2,2,pos>"), "sorn<0,2,2,pos>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<PosNoZero>(reportTestCases, "sorn<0,2,2,pos,nozero>"), "sorn<0,2,2,pos,nozero>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<UnitSorn>(reportTestCases, "sorn<0,1,4>"), "sorn<0,1,4>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits<TinySorn>(reportTestCases, "sorn<-149,-148,1,log>"), "sorn<-149,-148,1,log>", "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyLinearValues(reportTestCases), "sorn<0,4,8>", "numeric_limits values");
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
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
