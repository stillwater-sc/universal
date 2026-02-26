// decimal32.cpp: verify dfloat<7, 6> matches IEEE 754-2008 decimal32 format
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/dfloat/dfloat.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "decimal32 (dfloat<7,6>) standard format validation";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;

	using decimal32_bid = dfloat<7, 6, DecimalEncoding::BID, uint32_t>;
	using decimal32_dpd = dfloat<7, 6, DecimalEncoding::DPD, uint32_t>;

	std::cout << test_suite << '\n';

	// Test 1: Verify field widths for decimal32
	std::cout << "+---------    Field width verification (BID)\n";
	{
		static_assert(decimal32_bid::ndigits == 7, "decimal32 precision must be 7 digits");
		static_assert(decimal32_bid::es == 6, "decimal32 exponent continuation must be 6 bits");
		static_assert(decimal32_bid::nbits == 32, "decimal32 must be 32 bits");
		static_assert(decimal32_bid::combBits == 5, "combination field must be 5 bits");
		std::cout << "  BID: nbits=" << decimal32_bid::nbits
		          << " ndigits=" << decimal32_bid::ndigits
		          << " es=" << decimal32_bid::es
		          << " t=" << decimal32_bid::t
		          << " bias=" << decimal32_bid::bias << '\n';
	}

	// Test 2: Verify DPD field widths
	std::cout << "+---------    Field width verification (DPD)\n";
	{
		static_assert(decimal32_dpd::ndigits == 7, "decimal32 DPD precision must be 7 digits");
		static_assert(decimal32_dpd::nbits == 32, "decimal32 DPD must be 32 bits");
		std::cout << "  DPD: nbits=" << decimal32_dpd::nbits
		          << " ndigits=" << decimal32_dpd::ndigits
		          << " es=" << decimal32_dpd::es
		          << " t=" << decimal32_dpd::t
		          << " bias=" << decimal32_dpd::bias << '\n';
	}

	// Test 3: Special values
	std::cout << "+---------    Special values\n";
	{
		decimal32_bid zero(0);
		if (!zero.iszero()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: zero not detected\n";
		}

		decimal32_bid inf(SpecificValue::infpos);
		if (!inf.isinf() || inf.sign()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: +inf not correctly set\n";
		}

		decimal32_bid ninf(SpecificValue::infneg);
		if (!ninf.isinf() || !ninf.sign()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: -inf not correctly set\n";
		}

		decimal32_bid nan(SpecificValue::qnan);
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
			decimal32_bid bid(v);
			decimal32_dpd dpd(v);
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
			decimal32_bid ba(tc.a), bb(tc.b);
			decimal32_dpd da(tc.a), db(tc.b);

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
		decimal32_bid a(42);
		std::cout << "  type_tag: " << type_tag(a) << '\n';
		std::cout << "  to_binary(42): " << to_binary(a) << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
