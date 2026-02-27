// decimal64.cpp: verify dfloat<16, 8> matches IEEE 754-2008 decimal64 format
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/dfloat/dfloat.hpp>
#include <universal/verification/test_suite.hpp>

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

	std::string test_suite  = "decimal64 (dfloat<16,8>) standard format validation";
	std::string test_tag    = "decimal64";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using decimal64_bid = dfloat<16, 8, DecimalEncoding::BID, uint32_t>;
	using decimal64_dpd = dfloat<16, 8, DecimalEncoding::DPD, uint32_t>;

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1

	// Test 1: Verify field widths for decimal64
	std::cout << "+---------    Field width verification (BID)\n";
	{
		static_assert(decimal64_bid::ndigits == 16, "decimal64 precision must be 16 digits");
		static_assert(decimal64_bid::es == 8, "decimal64 exponent continuation must be 8 bits");
		static_assert(decimal64_bid::nbits == 64, "decimal64 must be 64 bits");
		static_assert(decimal64_bid::combBits == 5, "combination field must be 5 bits");
		std::cout << "  BID: nbits=" << decimal64_bid::nbits
		          << " ndigits=" << decimal64_bid::ndigits
		          << " es=" << decimal64_bid::es
		          << " t=" << decimal64_bid::t
		          << " bias=" << decimal64_bid::bias << '\n';
	}

	// Test 2: Verify DPD field widths
	std::cout << "+---------    Field width verification (DPD)\n";
	{
		static_assert(decimal64_dpd::ndigits == 16, "decimal64 DPD precision must be 16 digits");
		static_assert(decimal64_dpd::nbits == 64, "decimal64 DPD must be 64 bits");
		std::cout << "  DPD: nbits=" << decimal64_dpd::nbits
		          << " ndigits=" << decimal64_dpd::ndigits
		          << " es=" << decimal64_dpd::es
		          << " t=" << decimal64_dpd::t
		          << " bias=" << decimal64_dpd::bias << '\n';
	}

	// Test 3: Special values
	std::cout << "+---------    Special values\n";
	{
		decimal64_bid zero(0);
		if (!zero.iszero()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: zero not detected\n";
		}

		decimal64_bid inf(SpecificValue::infpos);
		if (!inf.isinf() || inf.sign()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: +inf not correctly set\n";
		}

		decimal64_bid ninf(SpecificValue::infneg);
		if (!ninf.isinf() || !ninf.sign()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: -inf not correctly set\n";
		}

		decimal64_bid nan(SpecificValue::qnan);
		if (!nan.isnan()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: NaN not correctly set\n";
		}
	}

	// Test 4: BID and DPD agree on values
	std::cout << "+---------    BID and DPD encode same values\n";
	{
		double values[] = { 0, 1, -1, 42, 0.1, -0.5, 999, 1234567, -9999999 };
		for (double v : values) {
			decimal64_bid bid(v);
			decimal64_dpd dpd(v);
			double dbid = double(bid);
			double ddpd = double(dpd);
			if (dbid != ddpd) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: BID(" << v << ") = " << dbid
				                               << " but DPD(" << v << ") = " << ddpd << '\n';
			}
		}
	}

	// Test 5: BID and DPD arithmetic agreement
	std::cout << "+---------    BID and DPD arithmetic agreement\n";
	{
		struct TestCase { double a; double b; };
		TestCase cases[] = {
			{ 42, 7 },
			{ 100, 0.1 },
			{ -5, 3 },
			{ 999, 1 },
		};
		for (const auto& tc : cases) {
			decimal64_bid ba(tc.a), bb(tc.b);
			decimal64_dpd da(tc.a), db(tc.b);

			double bid_sum = double(ba + bb);
			double dpd_sum = double(da + db);
			if (bid_sum != dpd_sum) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: BID " << tc.a << " + " << tc.b
				                               << " = " << bid_sum << " but DPD = " << dpd_sum << '\n';
			}

			double bid_prod = double(ba * bb);
			double dpd_prod = double(da * db);
			if (bid_prod != dpd_prod) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: BID " << tc.a << " * " << tc.b
				                               << " = " << bid_prod << " but DPD = " << dpd_prod << '\n';
			}
		}
	}

	// Test 6: Type tag and to_binary
	std::cout << "+---------    Type identification\n";
	{
		decimal64_bid a(42);
		std::cout << "  type_tag: " << type_tag(a) << '\n';
		std::cout << "  to_binary(42): " << to_binary(a) << '\n';
	}

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
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
