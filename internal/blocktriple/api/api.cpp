// api.cpp: class interface usage patterns for blocktriple
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/utility/bit_cast.hpp>
#include <fstream>
#include <universal/verification/test_reporters.hpp>

// minimum set of include files to reflect source code dependencies
#define BLOCKTRIPLE_VERBOSE_OUTPUT
#define BLOCKTRIPLE_TRACE_ALL
#include <universal/internal/blocktriple/blocktriple.hpp>

/*
 BlockTriple is the unifying compute engine for any of the
 floating-point number systems, linear, tapered, compressed, etc.

 The use case of blocktriple is as an ephemeral input/operator/round/output
 data structure through the computational pipeline.
 The blocktriple enables a uniform machine to go from source number system,
 through different arithmetic operators, such as,
 add/sub/mul/div/sqrt/special function, back to the source number encoding,
 or a new target number system

 To make this fast, we need to avoid any unnecessary copies.
 This will be particularly important for precise numbers, that is,
 numbers with many fraction bits, as the cost of the copy grows
 linearly with the size of the fraction bits.

 The input step is a normalization from number system to a triple.
 A triple is (sign, scale, significant).
 The blocktriple uses a 2's complement encoded significant for addition and subtraction.
 The format is bit-extended so that it can capture the largest value,
 which leads to the format: 00h.ff...ff. We need to two extra positions
 to capture a negative overflow.

 For multiplication, the blocktriple is encoded as a signed magnitude number
 and the radix adapts after the multiply.

 TODO: is there an optimization that can be applied that makes this
 even faster? What about moves? Need to ping Peter Gottschling.

 The significant is the input to the ALUs and SFUs.
 For addition and subtraction the significant needs to be aligned,
 which involves a shift operation, which is expensive for multi-block
 representations.
*/

template<typename Real>
void TestConversionRounding()
{
	using namespace sw::universal;
	constexpr Real f = Real(511.5f);
	std::cout << "\n " << typeid(Real).name() << " conversion use case and result\n";
	std::cout << to_binary(f, true) << " : " << f << '\n';
	CONSTEXPRESSION blocktriple<6, BlockTripleOperator::ADD, uint8_t> a = f;
	std::cout << to_triple(a) << " : " << a << '\n';
	CONSTEXPRESSION blocktriple<7, BlockTripleOperator::ADD, uint8_t> b = f;
	std::cout << to_triple(b) << " : " << b << '\n';
	CONSTEXPRESSION blocktriple<8, BlockTripleOperator::ADD, uint8_t> c = f;
	std::cout << to_triple(c) << " : " << c << '\n';
	CONSTEXPRESSION blocktriple<9, BlockTripleOperator::ADD, uint8_t> d = f;
	std::cout << to_triple(d) << " : " << d << '\n';
	CONSTEXPRESSION blocktriple<10, BlockTripleOperator::ADD, uint8_t> e = f;
	std::cout << to_triple(e) << " : " << e << '\n';
}


int main()
try {
	using namespace sw::universal;

	std::string test_suite = "blocktriple<> class interface test suite";
	std::cout << test_suite << '\n';

	int nrOfFailedTestCases = 0;


	// relationship between native float/double and blocktriple
	{
		blocktriple<8, BlockTripleOperator::ADD, uint8_t> a;
		a = 1.5f;
		std::cout << "IEEE-754 float  : " << to_binary(1.5f, true) << '\n';
		std::cout << "IEEE-754 float  : " << to_triple(1.5f, true) << '\n';
		std::cout << "blocktriple<8>  : " << to_triple(a) << '\n';
		a = 1.5;
		std::cout << "IEEE-754 double : " << to_binary(1.5, true) << '\n';
		std::cout << "IEEE-754 double : " << to_triple(1.5, true) << '\n';
		std::cout << "blocktriple<8>  : " << to_triple(a) << '\n';
	}

	// pick a value that rounds up to even between 6 to 10 bits of fraction
	TestConversionRounding<float>();
	TestConversionRounding<double>();

	{
		std::cout << "\nblocktriple add\n";
		constexpr size_t fbits = 7;
		blocktriple<fbits, BlockTripleOperator::ADD, uint32_t> a, b, c;
		a = 1.03125f;
		b = -1.03125f;
		std::cout << to_triple(a) << '\n' << to_triple(b) << '\n';
		c.add(a, b);
		std::cout << to_triple(c) << " : " << c << '\n';
	}

	{
		std::cout << "\nblocktriple sub\n";
		constexpr size_t fbits = 7;
		blocktriple<fbits, BlockTripleOperator::ADD, uint32_t> a, b, c;
		a = 1.03125f;
		b = 1.03125f;
		std::cout << to_triple(a) << '\n' << to_triple(b) << '\n';
		c.sub(a, b);
		std::cout << to_triple(c) << " : " << c << '\n';
	}

	{
		std::cout << "\nblocktriple mul\n";
		constexpr size_t fbits = 8;
		blocktriple<fbits, BlockTripleOperator::MUL, uint32_t> a, b, c;
		a = 2.0f;
		b = -0.5f;
		std::cout << to_triple(a) << '\n' << to_triple(b) << '\n';
		c.mul(a, b);
		std::cout << to_triple(c) << " : " << c << '\n';
	}


	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
