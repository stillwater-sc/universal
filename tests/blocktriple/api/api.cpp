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
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/number/bfloat/bfloat.hpp>
#include <universal/number/bfloat/manipulators.hpp>

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
		CONSTEXPRESSION blocktriple<10> a = 511.5f;
		cout << to_binary(a) << " : " << to_triple(a) << " : " << a << '\n';
	}
	{
		constexpr double d = 511.5;
		cout << to_binary(d, true) << '\n';
		CONSTEXPRESSION blocktriple<8> a = d;
		cout << to_binary(a) << " : " << to_triple(a) << " : " << a << '\n';
		CONSTEXPRESSION blocktriple<9> b = d;
		cout << to_binary(b) << " : " << to_triple(b) << " : " << b << '\n';
		CONSTEXPRESSION blocktriple<10> c = d;
		cout << to_binary(c) << " : " << to_triple(c) << " : " << c << '\n';
	}
	{
		constexpr size_t fbits = 7;
		constexpr size_t fhbits = fbits + 1;
		//constexpr size_t abits = fhbits + 3;
		//constexpr size_t sumbits = abits + 1;

		blocktriple<fhbits> a, b;
		//blocktriple<8, uint8_t> sum;
		using BlockType = typename blocktriple<fhbits>::bt;
		blockbinary<fhbits, BlockType> bba, bbb;
		bba.set_raw_bits(0xAAAAu);
		a.set(false, 7, bba);
		cout << to_triple(a) << " : " << a << '\n';
		b.set_raw_bits(0xAAAAu);
		b.set(false, 8, bbb);
		cout << to_triple(b) << " : " << b << '\n';
		//int aScale = a.scale();
		//int bScale = b.scale();
		//int maxScale = (aScale > bScale ? aScale : bScale);
		//blockbinary<sumbits, uint8_t> r1 = a.alignSignificant<sumbits>(aScale - maxScale + 3);
		cout << to_triple(a) << " : " << a << '\n';  // at this point the scale is off
	}
	{
		/*
		 * BlockTriple is the unifying compute engine for any of the 
		 * floating-point number systems, linear, tapered, compressed, etc.
		 * 
		 * The use case of BlockTriple is as a input/operator/round/output pipeline 
		 * from source number system, through BlockTriple, back to source, or a new target number system
		 * To make this fast, we need to avoid any unnecessary copies.
		 * This will be particularly important for precise numbers, that is,
		 * numbers with many fraction bits.
		 * 
		 * The input step is a normalization from number system to a (sign, scale, significant) triple.
		 * BlockTriple uses significant, that is, the fraction bits including the hidden bit.
		 * TODO: is there an optimization that can be applied that I am missing?
		 * 
		 * The significant is the input to the ALU operators. 
		 * For addition and subtraction the significant needs to be aligned,
		 * which involves a shift operation, which is expensive for multi-block
		 * representations.
		 */
#ifdef LATER
		bfloat<8, 2, uint8_t> a, b, c;
		a = 1.0f;
		b = -1.0f;
		constexpr size_t abits = a.fhbits; // a.abits;
		blocktriple<abits> aa, bb;
		blocktriple<abits + 1> sum;
		a.normalize(aa);  // decode of a bits into a triple form aa
		b.normalize(bb);  // decode of b bits into a triple form bb
		module_add(aa, bb, sum);  // ALU add operator
		convert(sum, c);
		cout << to_triple(sum) << " : " << sum << '\n';
		cout << color_print(c) << " : " << c << endl;
#endif
	}
	{
#ifdef LATER
		bfloat<8, 2, uint8_t> a, b, c;
		a = 1.0f;
		b = -1.0f;
		constexpr size_t mbits = a.fhbits; // a.abits;
		blocktriple<mbits> aa, bb;
		blocktriple<2*mbits> product;
		a.normalize(aa);  // decode of a bits into a triple form aa
		b.normalize(bb);  // decode of b bits into a triple form bb
		product.mul(aa, bb);  // ALU mule operator
		convert(product, c);
		cout << to_triple(product) << " : " << product << '\n';
		cout << color_print(c) << " : " << c << endl;
#endif
	}

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
