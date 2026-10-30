// exceptions.cpp: test suite runner for exceptions on adaptive precision binary integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//  SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>

// throw, so that the division by zero is observable
#define EINTEGER_THROW_ARITHMETIC_EXCEPTION 1
// minimum set of include files to reflect source code dependencies
#include <universal/number/einteger/einteger.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// op must throw einteger_divide_by_zero, which is an einteger_arithmetic_exception
	// and a std::runtime_error, so each catch level reaches it
	inline bool ThrowsDivideByZero(const std::function<void()>& op) {
		int levels = 0;
		try { op(); } catch (const einteger_divide_by_zero& err) { if (std::string(err.what()).size() > 0) ++levels; } catch (...) {}
		try { op(); } catch (const einteger_arithmetic_exception&) { ++levels; } catch (...) {}
		try { op(); } catch (const std::runtime_error&) { ++levels; } catch (...) {}
		return levels == 3;
	}

	// every division and remainder form with a zero divisor: einteger and literal operands, the
	// binary and the compound operators, reduce, and a zero divisor that is a computed result
	template<typename BlockType>
	int VerifyDivideByZero(bool reportTestCases) {
		using EInteger = einteger<BlockType>;
		int nrOfFailedTests = 0;
		const EInteger zero(0), empty, computedZero = EInteger(12345) - EInteger(12345);
		for (long long v : { 0ll, 1ll, -1ll, 255ll, -65536ll, 4294967297ll }) {
			const EInteger a(v);
			for (const EInteger& z : { zero, empty, computedZero }) {
				const std::pair<const char*, std::function<void()>> cases[] = {
					{ "a / 0",      [&] { (void)(a / z); } },
					{ "a % 0",      [&] { (void)(a % z); } },
					{ "a /= 0",     [&] { EInteger t(a); t /= z; } },
					{ "a %= 0",     [&] { EInteger t(a); t %= z; } },
					{ "a / 0ll",    [&] { (void)(a / 0ll); } },
					{ "a % 0ll",    [&] { (void)(a % 0ll); } },
					{ "a /= 0ll",   [&] { EInteger t(a); t /= 0ll; } },
					{ "a %= 0ll",   [&] { EInteger t(a); t %= 0ll; } },
					{ "v / 0",      [&] { (void)(v / z); } },
					{ "v % 0",      [&] { (void)(v % z); } },
					{ "reduce",     [&] { EInteger q, r; q.reduce(a, z, r); } },
				};
				for (const auto& c : cases) {
					if (!ThrowsDivideByZero(c.second)) {
						++nrOfFailedTests;
						if (reportTestCases) std::cerr << "FAIL: " << c.first << " with a = " << v << " did not throw einteger_divide_by_zero\n";
					}
				}
			}
		}
		return nrOfFailedTests;
	}

	// a non-zero divisor must not throw, including the divisors that are easiest to mistake for zero
	template<typename BlockType>
	int VerifyNoSpuriousThrow(bool reportTestCases) {
		using EInteger = einteger<BlockType>;
		int nrOfFailedTests = 0;
		for (long long d : { 1ll, -1ll, 256ll, -256ll, 65536ll, 4294967296ll }) {
			try {
				const EInteger a(1000003), b(d);
				EInteger q = a / b, r = a % b;
				q /= b;
				r %= b;
				(void)(a / d);
				(void)(1000003ll % b);
			}
			catch (const einteger_arithmetic_exception& err) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: divisor " << d << " threw " << err.what() << '\n';
			}
		}
		return nrOfFailedTests;
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

	std::string test_suite  = "adaptive precision binary integer exception validation";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	try {
		einteger<std::uint32_t> a(1), b(0);
		std::cout << a / b << '\n';
	}
	catch (const einteger_divide_by_zero& err) {
		std::cout << "caught: " << err.what() << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZero<std::uint8_t>(reportTestCases), "einteger<uint8_t>", "divide by zero");
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZero<std::uint16_t>(reportTestCases), "einteger<uint16_t>", "divide by zero");
	nrOfFailedTestCases += ReportTestResult(VerifyDivideByZero<std::uint32_t>(reportTestCases), "einteger<uint32_t>", "divide by zero");
	nrOfFailedTestCases += ReportTestResult(VerifyNoSpuriousThrow<std::uint8_t>(reportTestCases), "einteger<uint8_t>", "no spurious throw");
	nrOfFailedTestCases += ReportTestResult(VerifyNoSpuriousThrow<std::uint32_t>(reportTestCases), "einteger<uint32_t>", "no spurious throw");
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
catch (const sw::universal::einteger_arithmetic_exception& err) {
	std::cerr << "Uncaught einteger arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::einteger_internal_exception& err) {
	std::cerr << "Uncaught einteger internal exception: " << err.what() << std::endl;
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
