// decimal128.cpp: verify dfloat<34, 12> matches IEEE 754-2008 decimal128 format
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/dfloat/dfloat.hpp>
#include <universal/verification/test_suite.hpp>

#ifdef __SIZEOF_INT128__

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "decimal128 (dfloat<34,12>) standard format validation";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;

	using decimal128_bid = dfloat<34, 12, DecimalEncoding::BID, uint32_t>;
	using decimal128_dpd = dfloat<34, 12, DecimalEncoding::DPD, uint32_t>;

	std::cout << test_suite << '\n';

	// Test 1: Verify field widths for decimal128
	std::cout << "+---------    Field width verification (BID)\n";
	{
		static_assert(decimal128_bid::ndigits == 34, "decimal128 precision must be 34 digits");
		static_assert(decimal128_bid::es == 12, "decimal128 exponent continuation must be 12 bits");
		static_assert(decimal128_bid::nbits == 128, "decimal128 must be 128 bits");
		static_assert(decimal128_bid::combBits == 5, "combination field must be 5 bits");
		static_assert(decimal128_bid::t == 110, "decimal128 BID trailing must be 110 bits");
		std::cout << "  BID: nbits=" << decimal128_bid::nbits
		          << " ndigits=" << decimal128_bid::ndigits
		          << " es=" << decimal128_bid::es
		          << " t=" << decimal128_bid::t
		          << " bias=" << decimal128_bid::bias << '\n';
	}

	// Test 2: Verify DPD field widths
	std::cout << "+---------    Field width verification (DPD)\n";
	{
		static_assert(decimal128_dpd::ndigits == 34, "decimal128 DPD precision must be 34 digits");
		static_assert(decimal128_dpd::nbits == 128, "decimal128 DPD must be 128 bits");
		// DPD: (34-1)/3 = 11 declets of 10 bits = 110 bits
		static_assert(decimal128_dpd::t == 110, "decimal128 DPD trailing must be 110 bits");
		std::cout << "  DPD: nbits=" << decimal128_dpd::nbits
		          << " ndigits=" << decimal128_dpd::ndigits
		          << " es=" << decimal128_dpd::es
		          << " t=" << decimal128_dpd::t
		          << " bias=" << decimal128_dpd::bias << '\n';
	}

	// Test 3: Special values
	std::cout << "+---------    Special values\n";
	{
		decimal128_bid zero(0);
		if (!zero.iszero()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: zero not detected\n";
		}

		decimal128_bid inf(SpecificValue::infpos);
		if (!inf.isinf() || inf.sign()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: +inf not correctly set\n";
		}

		decimal128_bid ninf(SpecificValue::infneg);
		if (!ninf.isinf() || !ninf.sign()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: -inf not correctly set\n";
		}

		decimal128_bid nan(SpecificValue::qnan);
		if (!nan.isnan()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: NaN not correctly set\n";
		}

		decimal128_bid maxp(SpecificValue::maxpos);
		if (maxp.iszero() || maxp.isinf() || maxp.isnan()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: maxpos is incorrectly special\n";
		}

		decimal128_bid minp(SpecificValue::minpos);
		if (minp.iszero() || minp.isinf() || minp.isnan()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: minpos is incorrectly special\n";
		}
	}

	// Test 4: Integer round-trip (small values)
	std::cout << "+---------    Integer round-trip (small values)\n";
	{
		int values[] = { 0, 1, -1, 42, -42, 100, 9999, -12345 };
		for (int v : values) {
			decimal128_bid a(v);
			double d = double(a);
			if (d != static_cast<double>(v)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: integer " << v << " round-trip: got " << d << '\n';
			}
		}
	}

	// Test 5: Decimal exactness - 10 * 0.1 == 1.0
	std::cout << "+---------    Decimal exactness\n";
	{
		decimal128_bid ten(10);
		decimal128_bid tenth(0.1);
		decimal128_bid product = ten * tenth;
		decimal128_bid one(1);
		if (!(product == one)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 10 * 0.1 != 1.0, got " << product << '\n';
		}
	}

	// Test 6: Basic arithmetic
	std::cout << "+---------    Basic arithmetic\n";
	{
		decimal128_bid a(100), b(42);

		// addition
		decimal128_bid sum = a + b;
		if (double(sum) != 142.0) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 100 + 42 = " << double(sum) << '\n';
		}

		// subtraction
		decimal128_bid diff = a - b;
		if (double(diff) != 58.0) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 100 - 42 = " << double(diff) << '\n';
		}

		// multiplication
		decimal128_bid prod = a * b;
		if (double(prod) != 4200.0) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 100 * 42 = " << double(prod) << '\n';
		}

		// division
		decimal128_bid quot = a / b;
		double expected_q = 100.0 / 42.0;
		double got_q = double(quot);
		// allow small tolerance due to double precision
		if (std::fabs(got_q - expected_q) > 1e-10) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 100 / 42 = " << got_q << " expected " << expected_q << '\n';
		}
	}

	// Test 7: Negation and sign
	std::cout << "+---------    Negation and sign\n";
	{
		decimal128_bid a(42);
		decimal128_bid neg = -a;
		if (!neg.sign() || double(neg) != -42.0) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: negation of 42: got " << double(neg) << '\n';
		}
	}

	// Test 8: BID and DPD encode same values
	std::cout << "+---------    BID and DPD encode same values\n";
	{
		double values[] = { 0.0, 1.0, -1.0, 42.0, 0.1, -0.5, 999.0, 1234567.0 };
		for (double v : values) {
			decimal128_bid bid(v);
			decimal128_dpd dpd(v);
			double dbid = double(bid);
			double ddpd = double(dpd);
			if (dbid != ddpd) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cerr << "FAIL: BID(" << v << ") = " << dbid
				                               << " but DPD(" << v << ") = " << ddpd << '\n';
			}
		}
	}

	// Test 9: BID and DPD arithmetic agreement
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
			decimal128_bid ba(tc.a), bb(tc.b);
			decimal128_dpd da(tc.a), db(tc.b);

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

	// Test 10: Type tag and to_binary
	std::cout << "+---------    Type identification\n";
	{
		decimal128_bid a(42);
		std::cout << "  type_tag: " << type_tag(a) << '\n';
		std::cout << "  type_field: " << type_field(a) << '\n';
		std::cout << "  to_binary(42): " << to_binary(a) << '\n';
		std::cout << "  components(42): " << components(a) << '\n';
	}

	// Test 11: isone
	std::cout << "+---------    isone test\n";
	{
		decimal128_bid one(1);
		if (!one.isone()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: isone(1) returned false\n";
		}
		decimal128_bid two(2);
		if (two.isone()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: isone(2) returned true\n";
		}
	}

	// Test 12: Comparison operators
	std::cout << "+---------    Comparison operators\n";
	{
		decimal128_bid a(10), b(20), c(10);
		if (!(a == c)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 10 == 10\n";
		}
		if (!(a < b)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 10 < 20\n";
		}
		if (!(b > a)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 20 > 10\n";
		}
		if (a != c) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 10 != 10\n";
		}

		// negative comparisons
		decimal128_bid neg5(-5), neg10(-10);
		if (!(neg10 < neg5)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: -10 < -5\n";
		}
		if (!(neg5 < a)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: -5 < 10\n";
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

#else
// No __uint128_t support: skip test
int main() {
	std::cout << "decimal128 test SKIPPED: __uint128_t not available on this platform\n";
	return EXIT_SUCCESS;
}
#endif
