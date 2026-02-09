// api.cpp: application programming interface tests for microfloat number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the microfloat template environment
#define MICROFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/microfloat/microfloat.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "microfloat API tests";
	int nrOfFailedTestCases = 0;

	// demonstrate all 5 microfloat type aliases
	std::cout << "+---------    microfloat type aliases   --------+\n";
	{
		e2m1 a(1.0f);
		std::cout << "e2m1 : " << type_tag(a) << " : " << to_binary(a) << " : " << a << '\n';

		e2m3 b(1.0f);
		std::cout << "e2m3 : " << type_tag(b) << " : " << to_binary(b) << " : " << b << '\n';

		e3m2 c(1.0f);
		std::cout << "e3m2 : " << type_tag(c) << " : " << to_binary(c) << " : " << c << '\n';

		e4m3 d(1.0f);
		std::cout << "e4m3 : " << type_tag(d) << " : " << to_binary(d) << " : " << d << '\n';

		e5m2 e(1.0f);
		std::cout << "e5m2 : " << type_tag(e) << " : " << to_binary(e) << " : " << e << '\n';
	}

	// verify isone()
	std::cout << "+---------    isone() tests   --------+\n";
	{
		e4m3 a(1.0f);
		if (a.isone()) {
			std::cout << "e4m3 isone() test passed\n";
		}
		else {
			std::cout << "e4m3 isone() test failed\n";
			++nrOfFailedTestCases;
		}
	}

	// important behavioral traits
	std::cout << "+---------    Triviality of types   --------+\n";
	{
		ReportTrivialityOfType<e2m1>();
		ReportTrivialityOfType<e2m3>();
		ReportTrivialityOfType<e3m2>();
		ReportTrivialityOfType<e4m3>();
		ReportTrivialityOfType<e5m2>();
	}

	// arithmetic operators
	std::cout << "+---------    Arithmetic operators   --------+\n";
	{
		e4m3 a(2.0f), b(0.5f);
		ArithmeticOperators(a, b);
	}

	// logical operators
	std::cout << "+---------    Logical operators   --------+\n";
	{
		e4m3 a(1.0f), b(0.5f);
		LogicalOperators(a, b);
	}

	// explicit conversions
	std::cout << "+---------    Explicit conversions   --------+\n";
	{
		e4m3 a(1.0f);
		ExplicitConversions(a);
	}

	// dynamic ranges of all microfloat configurations
	std::cout << "+---------    Dynamic ranges   --------+\n";
	{
		auto show_range = [](auto mf) {
			using MF = decltype(mf);
			MF v;
			std::cout << type_tag(v) << '\n';
			std::cout << "  maxpos  : " << to_binary(v.maxpos()) << " : " << v << '\n';
			v.minpos();
			std::cout << "  minpos  : " << to_binary(v) << " : " << v << '\n';
			v.setzero();
			std::cout << "  zero    : " << to_binary(v) << " : " << v << '\n';
			v.minneg();
			std::cout << "  minneg  : " << to_binary(v) << " : " << v << '\n';
			v.maxneg();
			std::cout << "  maxneg  : " << to_binary(v) << " : " << v << '\n';
		};

		show_range(e2m1{});
		show_range(e2m3{});
		show_range(e3m2{});
		show_range(e4m3{});
		show_range(e5m2{});
	}

	// constexpr and specific values
	std::cout << "+---------    constexpr and specific values   --------+\n";
	{
		using Real = e4m3;

		CONSTEXPRESSION Real a{};
		std::cout << type_tag(a) << '\n';

		Real b(1.0f);
		std::cout << to_binary(b) << " : " << b << '\n';

		CONSTEXPRESSION Real c(SpecificValue::minpos);
		std::cout << to_binary(c) << " : " << c << " == minpos" << '\n';

		CONSTEXPRESSION Real d(SpecificValue::maxpos);
		std::cout << to_binary(d) << " : " << d << " == maxpos" << '\n';
	}

	// set bit patterns
	std::cout << "+---------    set bit patterns API   --------+\n";
	{
		using Real = e4m3;

		Real a;
		a.setbits(0x00);
		std::cout << to_binary(a) << " : " << a << '\n';

		a.setbit(3);
		std::cout << to_binary(a) << " : " << a << " : set bit 3" << '\n';

		a.setbits(0xFF);
		a.setbit(3, false);
		std::cout << to_binary(a) << " : " << a << " : reset bit 3" << '\n';
	}

	// e4m3 specific: NaN encoding
	std::cout << "+---------    e4m3 NaN   --------+\n";
	{
		e4m3 a;
		a.setbits(0x7F); // positive NaN for e4m3
		std::cout << "0x7F isnan: " << a.isnan() << " value: " << a << '\n';
		if (!a.isnan()) {
			std::cout << "e4m3 NaN test FAILED\n";
			++nrOfFailedTestCases;
		}
		a.setbits(0xFF); // negative NaN for e4m3
		std::cout << "0xFF isnan: " << a.isnan() << " value: " << a << '\n';
		if (!a.isnan()) {
			std::cout << "e4m3 NaN test FAILED\n";
			++nrOfFailedTestCases;
		}
		// e4m3 has no infinity
		a.setbits(0x7E);
		std::cout << "0x7E isinf: " << a.isinf() << " value: " << a << '\n';
		if (a.isinf()) {
			std::cout << "e4m3 should not have inf, test FAILED\n";
			++nrOfFailedTestCases;
		}
	}

	// e5m2 specific: IEEE-like Inf/NaN
	std::cout << "+---------    e5m2 Inf/NaN   --------+\n";
	{
		e5m2 a;
		a.setinf(false);
		std::cout << "positive inf: " << to_binary(a) << " isinf: " << a.isinf() << '\n';
		if (!a.isinf()) {
			std::cout << "e5m2 inf test FAILED\n";
			++nrOfFailedTestCases;
		}
		a.setnan(NAN_TYPE_QUIET);
		std::cout << "quiet NaN: " << to_binary(a) << " isnan: " << a.isnan() << '\n';
		if (!a.isnan()) {
			std::cout << "e5m2 NaN test FAILED\n";
			++nrOfFailedTestCases;
		}
	}

	// e2m1: no Inf, no NaN
	std::cout << "+---------    e2m1: no Inf, no NaN   --------+\n";
	{
		e2m1 a;
		// All 16 encodings are valid numbers
		for (unsigned i = 0; i < 16; ++i) {
			a.setbits(i);
			std::cout << to_binary(a) << " : " << a << '\n';
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
