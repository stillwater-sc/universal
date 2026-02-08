// api.cpp: application programming interface tests for e8m0 scale type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define E8M0_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/e8m0/e8m0.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "e8m0 API tests";
	int nrOfFailedTestCases = 0;

	// type tag
	std::cout << "+---------    e8m0 type tag   --------+\n";
	{
		e8m0 a;
		std::cout << type_tag(a) << '\n';
	}

	// verify 1.0 encoding
	std::cout << "+---------    e8m0 value 1.0   --------+\n";
	{
		e8m0 a(1.0f);
		std::cout << to_binary(a) << " : " << a << '\n';
		if (!a.isone()) {
			std::cout << "e8m0 isone() test FAILED\n";
			++nrOfFailedTestCases;
		}
		if (a.bits() != 127u) {
			std::cout << "e8m0 encoding for 1.0 should be 127, got " << unsigned(a.bits()) << "\n";
			++nrOfFailedTestCases;
		}
	}

	// triviality
	std::cout << "+---------    Triviality   --------+\n";
	{
		ReportTrivialityOfType<e8m0>();
	}

	// dynamic range
	std::cout << "+---------    Dynamic range   --------+\n";
	{
		e8m0 v;

		v.maxpos();
		std::cout << "maxpos  e8m0 : " << to_binary(v) << " : " << v << " (2^127)\n";

		v.setbits(127); // 2^0 = 1.0
		std::cout << "one     e8m0 : " << to_binary(v) << " : " << v << " (2^0)\n";

		v.minpos();
		std::cout << "minpos  e8m0 : " << to_binary(v) << " : " << v << " (2^-127)\n";

		v.setnan();
		std::cout << "NaN     e8m0 : " << to_binary(v) << " : " << v << '\n';
		if (!v.isnan()) {
			std::cout << "e8m0 NaN test FAILED\n";
			++nrOfFailedTestCases;
		}
	}

	// specific power-of-2 values
	std::cout << "+---------    Power-of-2 values   --------+\n";
	{
		float test_values[] = { 1.0f, 2.0f, 4.0f, 0.5f, 0.25f, 8.0f, 16.0f, 0.125f };
		for (float fv : test_values) {
			e8m0 a(fv);
			float roundtrip = float(a);
			std::cout << to_binary(a) << " : " << a << " (input: " << fv << ", roundtrip: " << roundtrip << ")\n";
			if (roundtrip != fv) {
				std::cout << "FAIL: round-trip for " << fv << " gave " << roundtrip << "\n";
				++nrOfFailedTestCases;
			}
		}
	}

	// non power-of-2 values get rounded to nearest power of 2
	std::cout << "+---------    Non power-of-2 rounding   --------+\n";
	{
		e8m0 a(3.0f);  // should round to 2^2 = 4.0 or 2^1 = 2.0
		std::cout << "e8m0(3.0) : " << to_binary(a) << " : " << a << '\n';

		e8m0 b(5.0f);  // should round to 2^2 = 4.0
		std::cout << "e8m0(5.0) : " << to_binary(b) << " : " << b << '\n';

		e8m0 c(6.0f);  // should round to 2^3 = 8.0
		std::cout << "e8m0(6.0) : " << to_binary(c) << " : " << c << '\n';
	}

	// setbit API
	std::cout << "+---------    setbit API   --------+\n";
	{
		e8m0 a;
		a.setbits(0x00);
		std::cout << to_binary(a) << " : " << a << '\n';

		a.setbits(0x7F); // 127 = 2^0 = 1.0
		std::cout << to_binary(a) << " : " << a << '\n';

		a.setbits(0xFE); // 254 = 2^127
		std::cout << to_binary(a) << " : " << a << '\n';

		a.setbits(0xFF); // NaN
		std::cout << to_binary(a) << " : " << a << " (NaN)\n";
	}

	// comparison operators
	std::cout << "+---------    Comparison operators   --------+\n";
	{
		e8m0 a(1.0f), b(2.0f), c(1.0f);
		if (!(a == c)) { ++nrOfFailedTestCases; std::cerr << "FAIL: 1.0 == 1.0\n"; }
		if (a == b) { ++nrOfFailedTestCases; std::cerr << "FAIL: 1.0 != 2.0\n"; }
		if (!(a < b)) { ++nrOfFailedTestCases; std::cerr << "FAIL: 1.0 < 2.0\n"; }
		if (!(b > a)) { ++nrOfFailedTestCases; std::cerr << "FAIL: 2.0 > 1.0\n"; }
	}

	// NaN behavior
	std::cout << "+---------    NaN behavior   --------+\n";
	{
		e8m0 nan_val;
		nan_val.setnan();
		e8m0 a(1.0f);
		if (nan_val == nan_val) { ++nrOfFailedTestCases; std::cerr << "FAIL: NaN == NaN should be false\n"; }
		if (nan_val == a) { ++nrOfFailedTestCases; std::cerr << "FAIL: NaN == 1.0 should be false\n"; }
	}

	// increment/decrement
	std::cout << "+---------    Increment/decrement   --------+\n";
	{
		e8m0 a(1.0f); // encoding 127
		std::cout << "a = " << a << " : " << to_binary(a) << '\n';
		++a; // should be 2^1 = 2.0
		std::cout << "++a = " << a << " : " << to_binary(a) << '\n';
		if (float(a) != 2.0f) {
			std::cout << "FAIL: ++e8m0(1.0) should be 2.0\n";
			++nrOfFailedTestCases;
		}
		--a; // back to 1.0
		std::cout << "--a = " << a << " : " << to_binary(a) << '\n';
		if (float(a) != 1.0f) {
			std::cout << "FAIL: --e8m0(2.0) should be 1.0\n";
			++nrOfFailedTestCases;
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
