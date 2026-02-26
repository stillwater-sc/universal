// short.cpp: verify hfloat<6, 7> matches IBM System/360 short precision format
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/hfloat/hfloat.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "hfloat_short (hfloat<6,7>) standard format validation";
	int nrOfFailedTestCases = 0;
	bool reportTestCases = true;

	using Short = hfloat<6, 7, uint32_t>;

	std::cout << test_suite << '\n';

	// Test 1: Field widths
	std::cout << "+---------    Field width verification\n";
	{
		static_assert(Short::ndigits == 6, "short HFP must have 6 hex digits");
		static_assert(Short::es == 7, "short HFP exponent must be 7 bits");
		static_assert(Short::fbits == 24, "short HFP fraction must be 24 bits");
		static_assert(Short::nbits == 32, "short HFP must be 32 bits total");
		static_assert(Short::bias == 64, "short HFP bias must be 64");
		std::cout << "  nbits=" << Short::nbits
		          << " ndigits=" << Short::ndigits
		          << " es=" << Short::es
		          << " fbits=" << Short::fbits
		          << " bias=" << Short::bias << '\n';
	}

	// Test 2: No NaN, no infinity
	std::cout << "+---------    No NaN, no infinity\n";
	{
		Short a(42);
		if (a.isnan()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: normal value reports as NaN\n";
		}
		if (a.isinf()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: normal value reports as inf\n";
		}
		// NaN request maps to zero
		Short nan_val(SpecificValue::qnan);
		if (!nan_val.iszero()) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: qnan request should map to zero\n";
		}
	}

	// Test 3: Trivially constructible
	std::cout << "+---------    Trivially constructible\n";
	{
		static_assert(std::is_trivially_constructible<Short>::value, "hfloat_short must be trivially constructible");
		static_assert(std::is_trivially_copyable<Short>::value, "hfloat_short must be trivially copyable");
		std::cout << "  trivially constructible: YES\n";
		std::cout << "  trivially copyable: YES\n";
	}

	// Test 4: Wobbling precision demonstration
	std::cout << "+---------    Wobbling precision\n";
	{
		// Value 1.0: binary = 0.0001 in hex → leading hex digit has 3 leading zero bits → less precision
		// Value 8.0: binary = 0.1000 in hex → leading hex digit fully utilized → more precision
		Short one(1.0), eight(8.0);
		std::cout << "  1.0: " << to_binary(one) << '\n';
		std::cout << "  8.0: " << to_binary(eight) << '\n';
		// both should convert correctly
		if (double(one) != 1.0) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 1.0 conversion\n";
		}
		if (double(eight) != 8.0) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: 8.0 conversion\n";
		}
	}

	// Test 5: Type tag
	std::cout << "+---------    Type identification\n";
	{
		Short a(42);
		std::cout << "  type_tag: " << type_tag(a) << '\n';
		std::cout << "  to_binary(42): " << to_binary(a) << '\n';
	}

	// Test 6: maxpos/maxneg range
	std::cout << "+---------    Dynamic range\n";
	{
		Short mp(SpecificValue::maxpos), mn(SpecificValue::maxneg);
		std::cout << "  maxpos: " << double(mp) << '\n';
		std::cout << "  maxneg: " << double(mn) << '\n';
		if (double(mp) <= 0) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: maxpos must be positive\n";
		}
		if (double(mn) >= 0) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: maxneg must be negative\n";
		}
		if (double(mp) != -double(mn)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: maxpos and maxneg should be symmetric\n";
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
