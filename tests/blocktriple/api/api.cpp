// api.cpp: class interface usage patterns for blocktriple
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <typeinfo>
// minimum set of include files to reflect source code dependencies
#define BLOCKTRIPLE_VERBOSE_OUTPUT
#define BLOCKTRIPLE_TRACE_ADD 1
#define BIT_CAST_SUPPORT 0
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/cfloat/manipulators.hpp>

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
 The blocktriple uses a 2's complement encoded significant, that is,
 the fraction bits including the hidden bit, and additionally extended
 for the specific use case, such as inputs to ALUs or Special Function Units (SFU).

 TODO: is there an optimization that can be applied that makes this
 even faster? What about moves? Need to ping Peter Gottschling.

 The significant is the input to the ALUs and SFUs.
 For addition and subtraction the significant needs to be aligned,
 which involves a shift operation, which is expensive for multi-block
 representations.
*/

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	std::cout << "blocktriple<> class interface tests" << std::endl;

	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	{
		cout << "\nFloat conversion use case and result\n";
		constexpr float f = 511.5f;
		cout << to_binary(f, true) << '\n';
		CONSTEXPRESSION blocktriple<8> a = f;
		cout << to_triple(a) << " : " << a << '\n';
		CONSTEXPRESSION blocktriple<9> b = f;
		cout << to_binary(b) << " : " << to_triple(b) << " : " << b << '\n';
		CONSTEXPRESSION blocktriple<10> c = f;
		cout << to_binary(c) << " : " << to_triple(c) << " : " << c << '\n';
	}

	{
		cout << "\nDouble conversion use case and result\n";
		constexpr double d = 511.5;
		cout << to_binary(d, true) << '\n';
		CONSTEXPRESSION blocktriple<8> a = d;
		cout << to_triple(a) << " : " << a << '\n';
		CONSTEXPRESSION blocktriple<9> b = d;
		cout << to_binary(b) << " : " << to_triple(b) << " : " << b << '\n';
		CONSTEXPRESSION blocktriple<10> c = d;
		cout << to_binary(c) << " : " << to_triple(c) << " : " << c << '\n';
	}

	{
		cout << "\nblocktriple add\n";
		constexpr size_t abits = 7;
		blocktriple<abits> a, b;
		blocktriple<abits + 1> c;
		a = 1.03125f;
		b = -1.03125f;
		cout << to_triple(a) << '\n' << to_triple(b) << '\n';
		c.add(a, b);   // ALU unrounded add operator
		cout << to_triple(c) << " : " << c << '\n';
	}

#ifdef BFLOAT
	// test the bfloat conversion 
	{
		cout << "\nbfloat conversion\n";
		using Real = bfloat<8, 2, uint8_t>;
		Real a;
		a = 1.875f;
		cout << color_print(a) << " : " << a << endl;
		constexpr size_t abits = Real::abits;
		blocktriple<abits> aa;
		a.normalize(aa);  // decode bfloat into a triple form
		cout << to_triple(aa) << " : " << a << '\n';
	}

	// test the bfloat addition 
	{
		cout << "\nbfloat addition\n";
		using Real = bfloat<8, 2, uint8_t>;
		Real a, b, c;
		a = 1.03125f;
		b = -1.03125f;
		constexpr size_t abits = Real::abits;
		blocktriple<abits> aa, bb;
		blocktriple<abits + 1> cc;
		a.normalize(aa);  // decode bfloat into a triple form aa ready for add/sub
		b.normalize(bb);  // decode bfloat into a triple form bb ready for add/sub
		cc.add(aa, bb);   // ALU unrounded add operator
		convert(cc, c);   // round and convert back to bfloat
		cout << to_triple(cc) << " : " << cc << '\n';
		cout << color_print(c) << " : " << c << endl;
	}
	{
		bfloat<8, 2, uint8_t> a, b, c;
		a = 1.0f;
		b = -1.0f;
		constexpr size_t mbits = a.fhbits; // a.abits;
		blocktriple<mbits> aa, bb;
		blocktriple<2*mbits> product;
		a.normalize(aa);  // decode of a bits into a triple form aa
		b.normalize(bb);  // decode of b bits into a triple form bb
		product.mul(aa, bb);  // ALU mul operator
		convert(product, c);
		cout << to_triple(product) << " : " << product << '\n';
		cout << color_print(c) << " : " << c << endl;

	}
#endif

#else // !MANUAL_TESTING



#endif // MANUAL_TESTING

	std::cout << "\nblocktriple API test suite           : " << (nrOfFailedTestCases == 0 ? "PASS\n" : "FAIL\n");

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
