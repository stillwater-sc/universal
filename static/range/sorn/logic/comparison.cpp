// comparison.cpp: test suite runner for the comparison operators of SORNs
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/sorn/sorn.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// every contiguous span [i, j] of the lattice equals itself and differs from every other span;
	// before #1972 operator!= was always false and there was no operator==
	template<typename SornType>
	int VerifySpanEquality(bool reportTestCases) {
		constexpr size_t N = SornType::sornBits;
		auto span = [](size_t i, size_t j) {
			SornType s;
			std::bitset<N> bits;
			for (size_t b = i; b <= j; ++b) bits[b] = true;
			s.setBits(bits);
			return s;
		};
		int nrOfFailedTestCases = 0;
		for (size_t i = 0; i < N; ++i) {
			for (size_t j = i; j < N; ++j) {
				const SornType a = span(i, j);
				for (size_t k = 0; k < N; ++k) {
					for (size_t l = k; l < N; ++l) {
						const SornType b = span(k, l);
						const bool same = (i == k && j == l);
						if ((a == b) != same || (a != b) == same) {
							++nrOfFailedTestCases;
							if (reportTestCases) std::cerr << "FAIL: " << a << (same ? " == " : " != ") << b << '\n';
						}
					}
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// NaN, the empty set, compares unequal to everything, itself included
	template<typename SornType>
	int VerifyNaNComparison(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const SornType nan(SpecificValue::qnan);
		const SornType one(1.0);
		if (nan == nan || !(nan != nan)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: nan compares equal to itself\n";
		}
		if (nan == one || one == nan || !(nan != one)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: nan compares equal to " << one << '\n';
		}
		// a negative zero and a positive zero are the same exact zero
		if (SornType(-0.0) != SornType(0.0)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: -0 != +0\n";
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

	std::string test_suite  = "sorn comparison validation";
	std::string test_tag    = "comparison";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using LinSorn   = sorn<0, 4, 8>;                   // linear, 19 intervals
	using SmallSorn = sorn<0, 2, 2>;                   // linear, 7 intervals
	using LogSorn   = sorn<-2, 2, 1, 0, 1, 1, 1, 1>;   // logarithmic

#if MANUAL_TESTING

	LinSorn a(1.25), b(1.25), c(2.25);
	std::cout << a << " == " << b << " : " << (a == b) << '\n';
	std::cout << a << " != " << c << " : " << (a != c) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySpanEquality<SmallSorn>(reportTestCases), "sorn<0,2,2>", "== and != on spans");
	nrOfFailedTestCases += ReportTestResult(VerifySpanEquality<LogSorn>(reportTestCases), "sorn<-2,2,1,log>", "== and != on spans");
	nrOfFailedTestCases += ReportTestResult(VerifyNaNComparison<LinSorn>(reportTestCases), "sorn<0,4,8>", "nan comparison");
	nrOfFailedTestCases += ReportTestResult(VerifyNaNComparison<LogSorn>(reportTestCases), "sorn<-2,2,1,log>", "nan comparison");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifySpanEquality<LinSorn>(reportTestCases), "sorn<0,4,8>", "== and != on spans");
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
