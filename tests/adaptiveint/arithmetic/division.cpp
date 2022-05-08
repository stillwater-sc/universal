// division.cpp: test suite runner for division on adaptive precision binary integers
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <limits>

// minimum set of include files to reflect source code dependencies
#define ADAPTIVEINT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/adaptiveint/adaptiveint.hpp>
#include <universal/verification/test_reporters.hpp>

// generate specific test case that you can trace with the trace conditions in mpreal.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<typename Ty, typename BlockType = std::uint32_t>
void GenerateTestCase(Ty _a, Ty _b) {
	Ty ref;
	sw::universal::adaptiveint<BlockType> a, b, aref, aratio;
	a = _a;
	b = _b;
	aratio = a / b;
	ref = _a / _b;
	aref = ref;
	constexpr size_t ndigits = 30;
	std::cout << std::setw(ndigits) << _a << " / " << std::setw(ndigits) << _b << " = " << std::setw(ndigits) << ref << std::endl;
	std::cout << a << " / " << b << " = " << aratio << " (reference: " << aref << ")   " ;
	std::cout << (aref == aratio ? "PASS" : "FAIL") << std::endl << std::endl;
}

namespace sw { namespace universal {
	// enumerate all division cases for an integer<nbits, BlockType> configuration
	template<typename BlockType>
	int VerifyAdaptiveDivision(size_t nbits, bool reportTestCases) {
		using Integer = adaptiveint;
		size_t NR_INTEGERS = (size_t(1) << nbits);

		Integer ia, ib, iq, iref, ir;

		int nrOfFailedTests = 0;
		for (size_t i = 0; i < NR_INTEGERS; i++) {
			ia.setbits(i);
			int64_t i64a = int64_t(ia);
			for (size_t j = 0; j < NR_INTEGERS; j++) {
				ib.setbits(j);
				int64_t i64b = int64_t(ib);
#if ADAPTIVEINT_THROW_ARITHMETIC_EXCEPTION
				try {
					iq.reduce(ia, ib, ir);
				}
				catch (const adaptiveint_divide_by_zero& e) {
					if (ib.iszero()) {
						// correctly caught the exception
						continue;
					}
					else {
						std::cerr << "unexpected : " << e.what() << std::endl;
						nrOfFailedTests++;
					}
				}
				catch (...) {
					std::cerr << "unexpected exception" << std::endl;
					nrOfFailedTests++;
				}
#else
				iq.reduce(ia, ib, ir);
#endif
				if (j == 0) {
					iref = 0; // or maxneg?
				}
				else {
					iref = i64a / i64b;
				}
				if (iq != iref) {
					nrOfFailedTests++;
					if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", ia, ib, iref, iq);
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "/", ia, ib, iref, iq);
				}
				if (nrOfFailedTests > 100) return nrOfFailedTests;
			}
			if (reportTestCases) if (i % 1024 == 0) std::cout << '.';
		}
		if (reportTestCases) std::cout << std::endl;
		return nrOfFailedTests;
	}
} } // namespace sw::universal


// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
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
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "adaptive precision binary integer division";
	std::string test_tag = "adaptiveint division";
	bool reportTestCases = true;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING
//	bool bReportIndividualTestCases = false;

	// generate individual testcases to hand trace/debug
//	GenerateTestCase(1, 2);

	std::int32_t _a, _b, _q, _r;
	adaptiveint<uint8_t> a, b, q, r;
	_a = 0x08040201;
	_b = 0x0804;
	_q = _a / _b;
	_r = _a % _b;
	a = _a; 
	b = _b;
	q.reduce(a, b, r);
	std::cout << "a   : " << to_binary(a) << " : " << (long long)(a) << '\n';
	std::cout << "b   : " << to_binary(b) << " : " << (long long)(b) << '\n';
	std::cout << "q   : " << to_binary(q) << " : " << (long long)(q) << '\n';
	std::cout << "r   : " << to_binary(r) << " : " << (long long)(r) << '\n';

	std::cout << "_a  : " << to_binary(_a, 32, true) << " : " << _a << '\n';
	std::cout << "_b  : " << to_binary(_b, 32, true) << " : " << _b << '\n';
	std::cout << "_q  : " << to_binary(_q, 32, true) << " : " << _q << '\n';
	std::cout << "_r  : " << to_binary(_r, 32, true) << " : " << _r << '\n';

	nrOfFailedTestCases += ReportTestResult(VerifyAdaptiveDivision<uint8_t>(12, reportTestCases), "adaptiveint<uint8_t>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1

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
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
