// cross_type_conversion.cpp: test suite for cross-type conversions between Universal number types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// number system includes
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/number/lns/lns.hpp>
// cross-type conversion infrastructure
#include <universal/number/convert/convert.hpp>
// test infrastructure
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

// Verify that the UniversalNumber concept correctly identifies types
template<typename T>
bool VerifyConcept(const std::string& label, bool expected) {
	bool detected = UniversalNumber<T>;
	if (detected != expected) {
		std::cerr << "FAIL: " << label << " concept detection: expected "
		          << (expected ? "true" : "false") << ", got "
		          << (detected ? "true" : "false") << '\n';
		return false;
	}
	return true;
}

// Verify round-trip conversion: Source -> Target -> Source
template<typename Source, typename Target>
int VerifyRoundTrip(double testValue, bool reportTestCases) {
	int nrOfFailedTests = 0;
	Source src(testValue);
	Target tgt = universal_cast<Target>(src);
	Source roundTrip = universal_cast<Source>(tgt);

	double srcVal = static_cast<double>(src);
	double rtVal = static_cast<double>(roundTrip);

	// Allow for representation loss in the intermediate type
	double tgtVal = static_cast<double>(tgt);
	Source expected(tgtVal);  // what we expect after round-trip through Target

	if (static_cast<double>(roundTrip) != static_cast<double>(expected)) {
		++nrOfFailedTests;
		if (reportTestCases) {
			std::cerr << "FAIL: round-trip " << typeid(Source).name()
			          << " -> " << typeid(Target).name()
			          << " -> " << typeid(Source).name()
			          << ": " << srcVal << " -> " << tgtVal
			          << " -> " << rtVal << '\n';
		}
	}
	return nrOfFailedTests;
}

// Verify mixed-type arithmetic
template<typename T1, typename T2>
int VerifyMixedArithmetic(double a, double b, bool reportTestCases) {
	int nrOfFailedTests = 0;
	T1 lhs(a);
	T2 rhs(b);

	double lhsVal = static_cast<double>(lhs);
	double rhsVal = static_cast<double>(rhs);

	using Result = promote_t<T1, T2>;

	// Test addition
	{
		auto result = lhs + rhs;
		double expected = static_cast<double>(Result(lhsVal + rhsVal));
		double got = static_cast<double>(result);
		if (got != expected) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cerr << "FAIL: mixed add: " << lhsVal << " + " << rhsVal
				          << " = " << got << " (expected " << expected << ")\n";
			}
		}
	}

	// Test subtraction
	{
		auto result = lhs - rhs;
		double expected = static_cast<double>(Result(lhsVal - rhsVal));
		double got = static_cast<double>(result);
		if (got != expected) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cerr << "FAIL: mixed sub: " << lhsVal << " - " << rhsVal
				          << " = " << got << " (expected " << expected << ")\n";
			}
		}
	}

	// Test multiplication
	{
		auto result = lhs * rhs;
		double expected = static_cast<double>(Result(lhsVal * rhsVal));
		double got = static_cast<double>(result);
		if (got != expected) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cerr << "FAIL: mixed mul: " << lhsVal << " * " << rhsVal
				          << " = " << got << " (expected " << expected << ")\n";
			}
		}
	}

	// Test division
	if (rhsVal != 0.0) {
		auto result = lhs / rhs;
		double expected = static_cast<double>(Result(lhsVal / rhsVal));
		double got = static_cast<double>(result);
		if (got != expected) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cerr << "FAIL: mixed div: " << lhsVal << " / " << rhsVal
				          << " = " << got << " (expected " << expected << ")\n";
			}
		}
	}

	return nrOfFailedTests;
}

// Verify mixed-type comparisons
template<typename T1, typename T2>
int VerifyMixedComparisons(double a, double b, bool reportTestCases) {
	int nrOfFailedTests = 0;
	T1 lhs(a);
	T2 rhs(b);

	double lhsVal = static_cast<double>(lhs);
	double rhsVal = static_cast<double>(rhs);

	if ((lhs == rhs) != (lhsVal == rhsVal)) {
		++nrOfFailedTests;
		if (reportTestCases) {
			std::cerr << "FAIL: mixed ==: " << lhsVal << " == " << rhsVal << '\n';
		}
	}
	if ((lhs != rhs) != (lhsVal != rhsVal)) {
		++nrOfFailedTests;
		if (reportTestCases) {
			std::cerr << "FAIL: mixed !=: " << lhsVal << " != " << rhsVal << '\n';
		}
	}
	if ((lhs < rhs) != (lhsVal < rhsVal)) {
		++nrOfFailedTests;
		if (reportTestCases) {
			std::cerr << "FAIL: mixed <: " << lhsVal << " < " << rhsVal << '\n';
		}
	}
	if ((lhs <= rhs) != (lhsVal <= rhsVal)) {
		++nrOfFailedTests;
		if (reportTestCases) {
			std::cerr << "FAIL: mixed <=: " << lhsVal << " <= " << rhsVal << '\n';
		}
	}
	if ((lhs > rhs) != (lhsVal > rhsVal)) {
		++nrOfFailedTests;
		if (reportTestCases) {
			std::cerr << "FAIL: mixed >: " << lhsVal << " > " << rhsVal << '\n';
		}
	}
	if ((lhs >= rhs) != (lhsVal >= rhsVal)) {
		++nrOfFailedTests;
		if (reportTestCases) {
			std::cerr << "FAIL: mixed >=: " << lhsVal << " >= " << rhsVal << '\n';
		}
	}

	return nrOfFailedTests;
}

}} // namespace sw::universal

// Regression testing guards: typically set by the cmake configuration, but MANUAL://://://://://
#define MANUAL_TESTING 0
// REGRESSION://://://://://
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "cross-type conversion validation";
	std::string test_tag = "cross_type";
	bool reportTestCases = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	////////////////////////////////////////////////////////////////////
	// Section 1: Concept detection
	{
		// Universal number types should satisfy UniversalNumber
		nrOfFailedTestCases += !VerifyConcept<posit<32,2>>("posit<32,2>", true);
		nrOfFailedTestCases += !VerifyConcept<cfloat<32,8,uint32_t>>("cfloat<32,8>", true);
		nrOfFailedTestCases += !VerifyConcept<fixpnt<32,16>>("fixpnt<32,16>", true);
		nrOfFailedTestCases += !VerifyConcept<integer<32>>("integer<32>", true);
		nrOfFailedTestCases += !VerifyConcept<lns<32,16>>("lns<32,16>", true);

		// Native arithmetic types should NOT satisfy UniversalNumber
		nrOfFailedTestCases += !VerifyConcept<float>("float", false);
		nrOfFailedTestCases += !VerifyConcept<double>("double", false);
		nrOfFailedTestCases += !VerifyConcept<int>("int", false);
		nrOfFailedTestCases += !VerifyConcept<unsigned>("unsigned", false);

		if (nrOfFailedTestCases == 0) {
			std::cout << "PASS: concept detection\n";
		}
	}

	////////////////////////////////////////////////////////////////////
	// Section 2: universal_cast conversions
	{
		int sectionFails = 0;
		double testValues[] = { 0.0, 1.0, -1.0, 0.5, -0.5, 3.14159, -2.71828, 100.0, -100.0, 0.001 };

		// posit <-> cfloat
		for (double v : testValues) {
			sectionFails += VerifyRoundTrip<posit<32,2>, cfloat<32,8,uint32_t>>(v, reportTestCases);
		}

		// posit <-> fixpnt
		for (double v : testValues) {
			sectionFails += VerifyRoundTrip<posit<32,2>, fixpnt<32,16>>(v, reportTestCases);
		}

		// cfloat <-> fixpnt
		for (double v : testValues) {
			sectionFails += VerifyRoundTrip<cfloat<32,8,uint32_t>, fixpnt<32,16>>(v, reportTestCases);
		}

		// posit <-> lns
		for (double v : testValues) {
			if (v == 0.0) continue; // lns cannot represent exact zero in all configs
			sectionFails += VerifyRoundTrip<posit<32,2>, lns<32,16>>(v, reportTestCases);
		}

		// integer -> posit (integer has no fractions)
		{
			integer<32> i(42);
			posit<32,2> p = universal_cast<posit<32,2>>(i);
			if (static_cast<double>(p) != 42.0) {
				++sectionFails;
				if (reportTestCases) std::cerr << "FAIL: integer(42) -> posit\n";
			}
		}

		// integer -> cfloat
		{
			integer<32> i(-17);
			cfloat<32,8,uint32_t> c = universal_cast<cfloat<32,8,uint32_t>>(i);
			if (static_cast<double>(c) != -17.0) {
				++sectionFails;
				if (reportTestCases) std::cerr << "FAIL: integer(-17) -> cfloat\n";
			}
		}

		nrOfFailedTestCases += sectionFails;
		if (sectionFails == 0) {
			std::cout << "PASS: universal_cast conversions\n";
		}
	}

	////////////////////////////////////////////////////////////////////
	// Section 3: Mixed-type arithmetic
	{
		int sectionFails = 0;

		// posit + cfloat
		sectionFails += VerifyMixedArithmetic<posit<32,2>, cfloat<32,8,uint32_t>>(3.14, 2.71, reportTestCases);

		// cfloat + fixpnt
		sectionFails += VerifyMixedArithmetic<cfloat<32,8,uint32_t>, fixpnt<32,16>>(1.5, 2.5, reportTestCases);

		// posit + fixpnt
		sectionFails += VerifyMixedArithmetic<posit<32,2>, fixpnt<32,16>>(10.0, 3.0, reportTestCases);

		// Test with negative values
		sectionFails += VerifyMixedArithmetic<posit<32,2>, cfloat<32,8,uint32_t>>(-5.0, 3.0, reportTestCases);

		// Test with very small values
		sectionFails += VerifyMixedArithmetic<posit<32,2>, cfloat<32,8,uint32_t>>(0.001, 0.002, reportTestCases);

		nrOfFailedTestCases += sectionFails;
		if (sectionFails == 0) {
			std::cout << "PASS: mixed-type arithmetic\n";
		}
	}

	////////////////////////////////////////////////////////////////////
	// Section 4: Mixed-type comparisons
	{
		int sectionFails = 0;

		sectionFails += VerifyMixedComparisons<posit<32,2>, cfloat<32,8,uint32_t>>(3.14, 2.71, reportTestCases);
		sectionFails += VerifyMixedComparisons<posit<32,2>, cfloat<32,8,uint32_t>>(1.0, 1.0, reportTestCases);
		sectionFails += VerifyMixedComparisons<cfloat<32,8,uint32_t>, fixpnt<32,16>>(5.0, 3.0, reportTestCases);
		sectionFails += VerifyMixedComparisons<posit<32,2>, fixpnt<32,16>>(-1.0, 1.0, reportTestCases);

		nrOfFailedTestCases += sectionFails;
		if (sectionFails == 0) {
			std::cout << "PASS: mixed-type comparisons\n";
		}
	}

	////////////////////////////////////////////////////////////////////
	// Section 5: Compound assignment operators
	{
		int sectionFails = 0;

		// posit += cfloat
		{
			posit<32,2> p(3.0);
			cfloat<32,8,uint32_t> c(2.0);
			p += c;
			double expected = static_cast<double>(posit<32,2>(3.0 + 2.0));
			if (static_cast<double>(p) != expected) {
				++sectionFails;
				if (reportTestCases) std::cerr << "FAIL: posit += cfloat\n";
			}
		}

		// cfloat *= posit
		{
			cfloat<32,8,uint32_t> c(4.0);
			posit<32,2> p(3.0);
			c *= p;
			double expected = static_cast<double>(cfloat<32,8,uint32_t>(4.0 * 3.0));
			if (static_cast<double>(c) != expected) {
				++sectionFails;
				if (reportTestCases) std::cerr << "FAIL: cfloat *= posit\n";
			}
		}

		nrOfFailedTestCases += sectionFails;
		if (sectionFails == 0) {
			std::cout << "PASS: compound assignment operators\n";
		}
	}

	////////////////////////////////////////////////////////////////////
	// Section 6: Promotion trait verification
	{
		int sectionFails = 0;

		// Verify that promote_t picks the wider type
		using P32 = posit<32,2>;
		using C16 = cfloat<16,5,uint16_t>;
		using C32 = cfloat<32,8,uint32_t>;

		static_assert(std::is_same_v<promote_t<P32, C16>, P32>,
			"posit<32,2> should be promoted over cfloat<16,5>");

		// posit<32,2> has fbits=27, cfloat<32,8> has fbits=23
		// so posit<32,2> has higher precision and always wins
		static_assert(std::is_same_v<promote_t<P32, C32>, P32>,
			"posit<32,2> has more fraction bits than cfloat<32,8>");
		static_assert(std::is_same_v<promote_t<C32, P32>, P32>,
			"posit<32,2> has more fraction bits than cfloat<32,8>");

		if (sectionFails == 0) {
			std::cout << "PASS: promotion traits\n";
		}
	}

	////////////////////////////////////////////////////////////////////
	// Section 7: Use case from issue #197 -- integer sequences to real
	{
		int sectionFails = 0;

		// Generate Fibonacci-like sequence with integers, convert to posit
		integer<32> fib_prev(1);
		integer<32> fib_curr(1);
		for (int i = 0; i < 10; ++i) {
			integer<32> next;
			next = fib_prev + fib_curr;
			fib_prev = fib_curr;
			fib_curr = next;
		}
		// fib_curr should be 144 (12th Fibonacci number)

		// Convert to posit for golden ratio approximation
		posit<32,2> ratio = universal_cast<posit<32,2>>(fib_curr);
		posit<32,2> prev = universal_cast<posit<32,2>>(fib_prev);
		posit<32,2> golden = ratio / prev;

		double goldenVal = static_cast<double>(golden);
		// Golden ratio is ~1.618, Fibonacci ratio at F(12)/F(11) = 144/89 ~= 1.6179
		if (goldenVal < 1.61 || goldenVal > 1.62) {
			++sectionFails;
			if (reportTestCases) {
				std::cerr << "FAIL: golden ratio approximation: " << goldenVal << '\n';
			}
		}

		// Convert golden ratio to cfloat for further computation
		cfloat<32,8,uint32_t> golden_cf = universal_cast<cfloat<32,8,uint32_t>>(golden);
		double cfVal = static_cast<double>(golden_cf);
		if (cfVal < 1.61 || cfVal > 1.62) {
			++sectionFails;
			if (reportTestCases) {
				std::cerr << "FAIL: golden ratio cfloat conversion: " << cfVal << '\n';
			}
		}

		nrOfFailedTestCases += sectionFails;
		if (sectionFails == 0) {
			std::cout << "PASS: issue #197 use case (integer -> posit -> cfloat)\n";
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
