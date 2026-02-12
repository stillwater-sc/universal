// arithmetic.cpp: functional tests for quire arithmetic operations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the posit arithmetic
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	////////////////////////////////////////////////////////////////////////
	// Quire assignment tests

	template<unsigned nbits, unsigned es>
	int VerifyQuireAssignment(bool reportTestCases) {
		int nrOfFailedTests = 0;
		using Posit = posit<nbits, es>;
		using Quire = quire<nbits, es>;

		Quire q;
		Posit result;

		// Test assignment from zero
		{
			q = 0;
			if (!q.iszero()) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: q = 0 should be zero\n";
			}
		}

		// Test assignment from positive integer (use power of 2 for exact representation)
		{
			q = 8;
			result = q.template convert_to<Posit>();
			if (double(result) != 8.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: q = 8: got " << result << '\n';
			}
		}

		// Test assignment from negative integer (use power of 2 for exact representation)
		{
			q = -4;
			result = q.template convert_to<Posit>();
			if (double(result) != -4.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: q = -4: got " << result << '\n';
			}
		}

		// Test assignment from float (use power of 2 for exact representation)
		{
			q = 0.5f;
			result = q.template convert_to<Posit>();
			if (std::abs(double(result) - 0.5) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: q = 0.5f: got " << result << '\n';
			}
		}

		// Test assignment from double (use power of 2 for exact representation)
		{
			q = 0.25;
			result = q.template convert_to<Posit>();
			if (std::abs(double(result) - 0.25) > 0.001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: q = 0.25: got " << result << '\n';
			}
		}

		// Test assignment from posit (use power of 2 for exact representation)
		{
			Posit p(2.0);
			q = p;
			result = q.template convert_to<Posit>();
			if (double(result) != double(p)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: q = posit(2.0): got " << result << '\n';
			}
		}

		// Test clear/reset
		{
			q = 16;
			q.clear();
			if (!q.iszero()) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: q.clear() should be zero\n";
			}

			q = 32;
			q.reset();
			if (!q.iszero()) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: q.reset() should be zero\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Quire addition tests

	template<unsigned nbits, unsigned es>
	int VerifyQuireAddition(bool reportTestCases) {
		int nrOfFailedTests = 0;
		using Posit = posit<nbits, es>;
		using Quire = quire<nbits, es>;

		Quire q;
		Posit result;

		// Test: 0 + x = x (use power of 2)
		{
			q.clear();
			q += Posit(4.0);
			result = q.template convert_to<Posit>();
			if (double(result) != 4.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: 0 + 4 = " << result << '\n';
			}
		}

		// Test: x + 0 = x (use power of 2)
		{
			q = 8;
			q += Posit(0.0);
			result = q.template convert_to<Posit>();
			if (double(result) != 8.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: 8 + 0 = " << result << '\n';
			}
		}

		// Test: positive + positive (use powers of 2: 8 + 4 = 12, but 12 may not be exact)
		// Instead use: 4 + 4 = 8
		{
			q = 4;
			q += Posit(4.0);
			result = q.template convert_to<Posit>();
			if (double(result) != 8.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: 4 + 4 = " << result << '\n';
			}
		}

		// Test: positive + negative (result positive): 8 + (-4) = 4
		{
			q = 8;
			q += Posit(-4.0);
			result = q.template convert_to<Posit>();
			if (double(result) != 4.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: 8 + (-4) = " << result << '\n';
			}
		}

		// Test: positive + negative (result negative): 4 + (-8) = -4
		{
			q = 4;
			q += Posit(-8.0);
			result = q.template convert_to<Posit>();
			if (double(result) != -4.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: 4 + (-8) = " << result << '\n';
			}
		}

		// Test: negative + negative: -4 + (-4) = -8
		{
			q = -4;
			q += Posit(-4.0);
			result = q.template convert_to<Posit>();
			if (double(result) != -8.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: -4 + (-4) = " << result << '\n';
			}
		}

		// Test: multiple accumulations (use powers of 2: 1+2+4+8 = 15, but not exact)
		// Instead: 1+1+1+1 = 4
		{
			q.clear();
			for (int i = 0; i < 4; ++i) {
				q += Posit(1.0);
			}
			result = q.template convert_to<Posit>();
			if (double(result) != 4.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: sum(1,1,1,1) = " << result << " (expected 4)\n";
			}
		}

		// Test: adding quires: 8 + 8 = 16
		{
			Quire q1, q2;
			q1 = 8;
			q2 = 8;
			q1 += q2;
			result = q1.template convert_to<Posit>();
			if (double(result) != 16.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: quire(8) + quire(8) = " << result << '\n';
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Quire subtraction tests

	template<unsigned nbits, unsigned es>
	int VerifyQuireSubtraction(bool reportTestCases) {
		int nrOfFailedTests = 0;
		using Posit = posit<nbits, es>;
		using Quire = quire<nbits, es>;

		Quire q;
		Posit result;

		// Test: x - 0 = x (use power of 2)
		{
			q = 8;
			q -= Posit(0.0);
			result = q.template convert_to<Posit>();
			if (double(result) != 8.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: 8 - 0 = " << result << '\n';
			}
		}

		// Test: positive - positive (result positive): 8 - 4 = 4
		{
			q = 8;
			q -= Posit(4.0);
			result = q.template convert_to<Posit>();
			if (double(result) != 4.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: 8 - 4 = " << result << '\n';
			}
		}

		// Test: positive - positive (result negative): 4 - 8 = -4
		{
			q = 4;
			q -= Posit(8.0);
			result = q.template convert_to<Posit>();
			if (double(result) != -4.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: 4 - 8 = " << result << '\n';
			}
		}

		// Test: positive - negative = positive + positive: 4 - (-4) = 8
		{
			q = 4;
			q -= Posit(-4.0);
			result = q.template convert_to<Posit>();
			if (double(result) != 8.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: 4 - (-4) = " << result << '\n';
			}
		}

		// Test: negative - positive: -4 - 4 = -8
		{
			q = -4;
			q -= Posit(4.0);
			result = q.template convert_to<Posit>();
			if (double(result) != -8.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: -4 - 4 = " << result << '\n';
			}
		}

		// Test: x - x = 0 (use power of 2)
		{
			q = 16;
			q -= Posit(16.0);
			if (!q.iszero()) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: 16 - 16 should be zero\n";
			}
		}

		// Test: subtracting quires: 16 - 8 = 8
		{
			Quire q1, q2;
			q1 = 16;
			q2 = 8;
			q1 -= q2;
			result = q1.template convert_to<Posit>();
			if (double(result) != 8.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: quire(16) - quire(8) = " << result << '\n';
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Quire conversion tests

	template<unsigned nbits, unsigned es>
	int VerifyQuireConversion(bool reportTestCases) {
		int nrOfFailedTests = 0;
		using Posit = posit<nbits, es>;
		using Quire = quire<nbits, es>;

		Quire q;
		Posit result;

		// Test powers of 2
		for (int exp = 0; exp <= 4; ++exp) {
			double val = std::pow(2.0, exp);
			q = val;
			result = q.template convert_to<Posit>();
			if (double(result) != val) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert 2^" << exp << " = " << result << '\n';
			}
		}

		// Test negative powers of 2
		for (int exp = -1; exp >= -4; --exp) {
			double val = std::pow(2.0, exp);
			q = val;
			result = q.template convert_to<Posit>();
			if (std::abs(double(result) - val) > 0.0001) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: convert 2^" << exp << " = " << result << '\n';
			}
		}

		// Test round-trip: posit -> quire -> posit (use power of 2 for exact representation)
		{
			Posit original(4.0);
			q = original;
			result = q.template convert_to<Posit>();
			if (result != original) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: round-trip 4.0: got " << result << '\n';
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// Quire exception tests

	template<unsigned nbits, unsigned es>
	int VerifyQuireExceptions(bool reportTestCases) {
		int nrOfFailedTests = 0;
		using Quire = quire<nbits, es>;

		// Test operand_too_large_for_quire
		{
			Quire q;
			bool exceptionCaught = false;
			try {
				// Create a value larger than the quire can hold
				// For posit<8,0>, half_range is small, so a large integer should overflow
				internal::value<64> largeValue;
				largeValue.set(false, q.max_scale() + 10, internal::bitblock<64>(), false, false);
				q = largeValue;
			}
			catch (const operand_too_large_for_quire&) {
				exceptionCaught = true;
			}
			catch (...) {
				// Some other exception
			}
			if (!exceptionCaught) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: operand_too_large_for_quire not thrown\n";
			}
		}

		// Test operand_too_small_for_quire
		{
			Quire q;
			bool exceptionCaught = false;
			try {
				// Create a value smaller than the quire can hold
				internal::value<64> smallValue;
				smallValue.set(false, q.min_scale() - 10, internal::bitblock<64>(), false, false);
				q = smallValue;
			}
			catch (const operand_too_small_for_quire&) {
				exceptionCaught = true;
			}
			catch (...) {
				// Some other exception
			}
			if (!exceptionCaught) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: operand_too_small_for_quire not thrown\n";
			}
		}

		return nrOfFailedTests;
	}

	////////////////////////////////////////////////////////////////////////
	// FDP (Fused Dot Product) tests

	template<unsigned nbits, unsigned es>
	int VerifyFusedDotProduct(bool reportTestCases) {
		int nrOfFailedTests = 0;
		using Posit = posit<nbits, es>;
		using Quire = quire<nbits, es>;

		// Test simple FDP
		{
			Posit a[] = { Posit(1.0), Posit(2.0), Posit(3.0) };
			Posit b[] = { Posit(4.0), Posit(5.0), Posit(6.0) };
			// Expected: 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32

			Quire q;
			for (int i = 0; i < 3; ++i) {
				q += quire_mul(a[i], b[i]);
			}
			Posit result = q.template convert_to<Posit>();
			if (double(result) != 32.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: FDP [1,2,3].[4,5,6] = " << result << " (expected 32)\n";
			}
		}

		// Test FDP with negative values
		{
			Posit a[] = { Posit(1.0), Posit(-2.0), Posit(3.0) };
			Posit b[] = { Posit(4.0), Posit(5.0), Posit(-6.0) };
			// Expected: 1*4 + (-2)*5 + 3*(-6) = 4 - 10 - 18 = -24

			Quire q;
			for (int i = 0; i < 3; ++i) {
				q += quire_mul(a[i], b[i]);
			}
			Posit result = q.template convert_to<Posit>();
			if (double(result) != -24.0) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: FDP with negatives = " << result << " (expected -24)\n";
			}
		}

		return nrOfFailedTests;
	}

}} // namespace sw::universal

// Regression testing guards
#define MANUAL_TESTING 0
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

	std::string test_suite = "quire arithmetic verification";
	std::string test_tag = "arithmetic";
	bool reportTestCases = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	reportTestCases = true;
	nrOfFailedTestCases += ReportTestResult(VerifyQuireAssignment<16, 1>(reportTestCases), "quire<16,1>", "assignment");
	nrOfFailedTestCases += ReportTestResult(VerifyQuireAddition<16, 1>(reportTestCases), "quire<16,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyQuireSubtraction<16, 1>(reportTestCases), "quire<16,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyQuireConversion<16, 1>(reportTestCases), "quire<16,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyQuireExceptions<8, 0>(reportTestCases), "quire<8,0>", "exceptions");
	nrOfFailedTestCases += ReportTestResult(VerifyFusedDotProduct<16, 1>(reportTestCases), "quire<16,1>", "FDP");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	// posit<8,0> tests
	nrOfFailedTestCases += ReportTestResult(VerifyQuireAssignment<8, 0>(reportTestCases), "quire<8,0>", "assignment");
	nrOfFailedTestCases += ReportTestResult(VerifyQuireAddition<8, 0>(reportTestCases), "quire<8,0>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyQuireSubtraction<8, 0>(reportTestCases), "quire<8,0>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyQuireConversion<8, 0>(reportTestCases), "quire<8,0>", "conversion");

	// posit<16,1> tests
	nrOfFailedTestCases += ReportTestResult(VerifyQuireAssignment<16, 1>(reportTestCases), "quire<16,1>", "assignment");
	nrOfFailedTestCases += ReportTestResult(VerifyQuireAddition<16, 1>(reportTestCases), "quire<16,1>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyQuireSubtraction<16, 1>(reportTestCases), "quire<16,1>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyQuireConversion<16, 1>(reportTestCases), "quire<16,1>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyFusedDotProduct<16, 1>(reportTestCases), "quire<16,1>", "FDP");

	// Exception tests
	nrOfFailedTestCases += ReportTestResult(VerifyQuireExceptions<8, 0>(reportTestCases), "quire<8,0>", "exceptions");
	nrOfFailedTestCases += ReportTestResult(VerifyQuireExceptions<16, 1>(reportTestCases), "quire<16,1>", "exceptions");
#endif

#if REGRESSION_LEVEL_2
	// posit<32,2> tests
	nrOfFailedTestCases += ReportTestResult(VerifyQuireAssignment<32, 2>(reportTestCases), "quire<32,2>", "assignment");
	nrOfFailedTestCases += ReportTestResult(VerifyQuireAddition<32, 2>(reportTestCases), "quire<32,2>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyQuireSubtraction<32, 2>(reportTestCases), "quire<32,2>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyQuireConversion<32, 2>(reportTestCases), "quire<32,2>", "conversion");
	nrOfFailedTestCases += ReportTestResult(VerifyFusedDotProduct<32, 2>(reportTestCases), "quire<32,2>", "FDP");
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Unexpected quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Caught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
