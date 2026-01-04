// algo.cpp: tests to explore different implementations of the arithmetic operators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/utility/bit_cast.hpp>
#include <iostream>
#include <iomanip>

// minimum set of include files to reflect source code dependencies
#include <universal/native/ieee754.hpp>
#define VALUE_THROW_ARITHMETIC_EXCEPTION 0
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/internal/value/value.hpp>
#define BLOCKTRIPLE_VERBOSE_OUTPUT 1
#define BLOCKTRIPLE_TRACE_ALL
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
// #include <universal/verification/test_reporters.hpp>

int main()
try {
	using namespace sw::universal;
	
//	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	// generate individual testcases to hand trace/debug

	{
		constexpr size_t fbits = 7;
		constexpr size_t fhbits = fbits + 1;
		constexpr size_t abits = fhbits + 3;
		constexpr size_t sumbits = abits + 1;
		internal::value<fbits> a,b;
		a = 1.0f;
		b = 1.0f;
		std::cout << to_triple(a) << " : " << a << '\n';
		std::cout << to_triple(b) << " : " << b << '\n';
		// add is adding 3 bits to the mantissa to 
		// have all rounding bits available after alignment
		internal::value<sumbits> c;
		internal::module_add<fbits, abits>(a, b, c);  // this API with the <abits + 1> argument is too confusing 
		std::cout << to_triple(c) << " : " << c << '\n';
	}

	// blocktriple stores the significant as you need the hidden bit in any
	// arithmetic operators.

	// to support the quire (Kulisch superaccumulator):
	// - operators add/sub/mul need to produce unrounded results
	// - operators div/sqrt are rounded as part of the conversion iteration
	
	// for a significant of nbits, the add/sub input size is nbits + 3
	// The extra 3 bits, are the guard, round, and stick bits that need
	// to come into play to correctly round add/sub as operand alignment
	// shifts information into these bits.
	// The output of the add/sub is nbits + 3 + 1 representing the unrounded result.
	{
		constexpr size_t fbits = 4;  // the number of fraction bits in the representation

		blocktriple<fbits, BlockTripleOperator::ADD, uint32_t> a, b, c;
		a.constexprClassParameters();

		std::cout << "-----------  1 + 1 = 2 -----------\n";
		// we have fbits fraction bits
		// an ADD needs 2*(fbits + 1) fraction bits to accomodate correct rounding on argument alignment
		// an ADD needs 3 extra bits to capture the integer bits cases of overflow and 2's complement
		a.setbits(1ull << (a.abits));
		b.setbits(1ull << (b.abits));
		c.add(a, b);
		std::cout << to_triple(a) << " : " << a << '\n';
		std::cout << to_triple(b) << " : " << b << '\n';
		std::cout << to_triple(c) << " : " << c << '\n';

		std::cout << "-----------  1 - 1 = 0 -----------\n";
//		a = 1.0f;
//		b = -1.0f;
		a.setbits(1ull << (a.abits));
		b.setbits(1ull << (b.abits));
		b.setsign(true);
		c.add(a, b);
		std::cout << to_triple(a) << " : " << a << '\n';
		std::cout << to_triple(b) << " : " << b << '\n';
		std::cout << to_triple(c) << " : " << c << '\n';

		std::cout << "-----------  0 - 1 = -1 -----------\n";
//		a = 0.0f;
//		b = -1.0f;
		a.setbits(0ull);
		b.setbits(1ull << (b.abits));
		b.setsign(true);
		c.add(a, b);
		std::cout << to_triple(a) << " : " << a << '\n';
		std::cout << to_triple(b) << " : " << b << '\n';
		std::cout << to_triple(c) << " : " << c << '\n';
	}


	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
