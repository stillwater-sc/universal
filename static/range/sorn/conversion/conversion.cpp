// conversion.cpp: test suite runner for conversion to and from SORNs
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/sorn/sorn.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	template<typename SornType>
	SornType sornFromInterval(const typename SornType::SORN_INTERVAL& interval) {
		SornType s;
		s.sornIntVal = interval;
		return s;
	}

	// every lattice interval converts to a native value that converts back to the same interval;
	// before #1972 to_native returned 0 for everything
	template<typename SornType>
	int VerifyNativeRoundTrip(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		for (const auto& interval : SornType::datatype()) {
			const SornType s = sornFromInterval<SornType>(interval);
			const double  d = double(s);
			const float   f = float(s);
			const SornType fromDouble(d), fromFloat(f);
			if (fromDouble != s || fromFloat != s) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << s << " -> " << d << " -> " << fromDouble << '\n';
			}
		}
		return nrOfFailedTestCases;
	}

	// assigning a NaN replaces the previous value with NaN; it used to leave the old value in place
	template<typename SornType>
	int VerifyNaNAssignment(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		SornType s(1.25);
		s = std::numeric_limits<double>::quiet_NaN();
		if (!s.isnan()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: double NaN assignment left " << s << '\n';
		}
		s = 1.25f;
		s = std::numeric_limits<float>::quiet_NaN();
		if (!s.isnan() || !std::isnan(double(s))) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: float NaN assignment left " << s << '\n';
		}
		// a default-constructed sorn is NaN, and so is an empty bit pattern
		SornType d;
		if (!d.isnan()) ++nrOfFailedTestCases;
		s = 1.25;
		s.setbits(0);
		if (!s.isnan()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: setbits(0) left " << s << '\n';
		}
		return nrOfFailedTestCases;
	}

	// a value outside the lattice saturates to the outermost interval on its side
	template<typename SornType>
	int VerifySaturation(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const auto& dt = SornType::datatype();
		const SornType hi = sornFromInterval<SornType>(dt.back());
		const SornType lo = sornFromInterval<SornType>(dt.front());
		if (SornType(1.0e6) != hi || SornType(std::numeric_limits<double>::infinity()) != hi) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: large value did not saturate to " << hi << '\n';
		}
		if (SornType(-1.0e6) != lo || SornType(-std::numeric_limits<double>::infinity()) != lo) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: large negative value did not saturate to " << lo << '\n';
		}
		return nrOfFailedTestCases;
	}

	// sorn(SpecificValue) used to do nothing, leaving the default NaN
	int VerifySpecificValues(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		auto check = [&](const auto& got, const std::string& expected, const char* label) {
			std::stringstream s;
			s << got;
			if (s.str() != expected) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: " << label << " = " << s.str() << ", expected " << expected << '\n';
			}
		};
		using S = sorn<0, 4, 8>;   // [-inf,-4) ... [-0.5,0) 0 (0,0.5] ... (4,inf]
		check(S(SpecificValue::maxpos), "(3.5,4]",  "maxpos");
		check(S(SpecificValue::minpos), "(0,0.5]",  "minpos");
		check(S(SpecificValue::zero),   "0",        "zero");
		check(S(SpecificValue::minneg), "[-0.5,0)", "minneg");
		check(S(SpecificValue::maxneg), "[-4,-3.5)", "maxneg");
		check(S(SpecificValue::infpos), "(4,inf]",  "infpos");
		check(S(SpecificValue::infneg), "[-inf,-4)", "infneg");
		check(S(SpecificValue::qnan),   "nan",      "qnan");
		check(S(SpecificValue::snan),   "nan",      "snan");
		check(S(SpecificValue::nar),    "nan",      "nar");

		// no negatives and no infinity: minneg/maxneg do not exist, infinity saturates
		using P = sorn<0, 2, 2, 1, 1, 0, 0, 1>;   // 0 (0,1] (1,2]
		check(P(SpecificValue::maxpos), "(1,2]", "positive-only maxpos");
		check(P(SpecificValue::minneg), "nan",   "positive-only minneg");
		check(P(SpecificValue::maxneg), "nan",   "positive-only maxneg");
		check(P(SpecificValue::infpos), "(1,2]", "positive-only infpos");

		// to_binary: one bit per covered lattice interval, highest first; it used to print nothing
		if (to_binary(S(1.25)) != "0b0000001000000000000" || to_binary(S(SpecificValue::qnan)) != "0b0000000000000000000") {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: to_binary(" << S(1.25) << ") = " << to_binary(S(1.25)) << '\n';
		}
		return nrOfFailedTestCases;
	}

	// setbits/getBits round trip on every contiguous span; getBits used to mark only the two end
	// intervals of a span, and setbits did nothing
	template<typename SornType>
	int VerifyBitsRoundTrip(bool reportTestCases) {
		constexpr size_t N = SornType::sornBits;
		static_assert(N < 64, "span encodings must fit setbits");
		int nrOfFailedTestCases = 0;
		for (size_t i = 0; i < N; ++i) {
			for (size_t j = i; j < N; ++j) {
				std::uint64_t encoding = 0;
				for (size_t b = i; b <= j; ++b) encoding |= (std::uint64_t(1) << b);
				SornType s;
				s.setbits(encoding);
				if (s.getBits().to_ullong() != encoding) {
					++nrOfFailedTestCases;
					if (reportTestCases) std::cerr << "FAIL: setbits(" << encoding << ") = " << s << " reads back " << s.getBits() << '\n';
				}
			}
		}
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

	std::string test_suite  = "sorn conversion validation";
	std::string test_tag    = "conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using LinSorn    = sorn<0, 4, 8>;                   // linear with infinity
	using LogSorn    = sorn<-2, 2, 1, 0, 1, 1, 1, 1>;   // logarithmic with infinity
	using NoZeroSorn = sorn<0, 4, 4, 1, 1, 1, 1, 0>;    // linear, no exact zero
	using FiniteSorn = sorn<0, 4, 8, 1, 1, 1, 0, 1>;    // linear, no infinity
	using TinySorn   = sorn<-149, -148, 1, 0, 1, 0, 0, 1>;   // logarithmic at the bottom of binary32: 0 (0,2^-149] (2^-149,2^-148]

#if MANUAL_TESTING

	LinSorn s(1.25);
	std::cout << s << " -> " << double(s) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyNativeRoundTrip<LinSorn>(reportTestCases), "sorn<0,4,8>", "to_native round trip");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeRoundTrip<LogSorn>(reportTestCases), "sorn<-2,2,1,log>", "to_native round trip");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeRoundTrip<NoZeroSorn>(reportTestCases), "sorn<0,4,4,nozero>", "to_native round trip");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeRoundTrip<FiniteSorn>(reportTestCases), "sorn<0,4,8,noinf>", "to_native round trip");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeRoundTrip<TinySorn>(reportTestCases), "sorn<-149,-148,1,log>", "to_native round trip");
	nrOfFailedTestCases += ReportTestResult(VerifyNaNAssignment<LinSorn>(reportTestCases), "sorn<0,4,8>", "nan assignment");
	nrOfFailedTestCases += ReportTestResult(VerifySaturation<FiniteSorn>(reportTestCases), "sorn<0,4,8,noinf>", "saturation");
	nrOfFailedTestCases += ReportTestResult(VerifySaturation<LinSorn>(reportTestCases), "sorn<0,4,8>", "saturation");
	nrOfFailedTestCases += ReportTestResult(VerifySpecificValues(reportTestCases), "sorn", "SpecificValue");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsRoundTrip<LinSorn>(reportTestCases), "sorn<0,4,8>", "setbits/getBits");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsRoundTrip<LogSorn>(reportTestCases), "sorn<-2,2,1,log>", "setbits/getBits");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsRoundTrip<NoZeroSorn>(reportTestCases), "sorn<0,4,4,nozero>", "setbits/getBits");
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
