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

	for (int i = 0; i < argc; ++i) std::cout << argv[0] << ' ';
	std::cout << std::endl;

	std::cout << "blocktriple<> class interface tests" << std::endl;

	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING


	{
		CONSTEXPRESSION blocktriple<10, uint32_t> a = 511.5f;
		cout << to_binary(a) << " : " << to_triple(a) << " : " << a << '\n';
	}
	{
		constexpr double d = 511.5;
		cout << to_binary(d, true) << '\n';
		CONSTEXPRESSION blocktriple<8, uint64_t> a = d;
		cout << to_binary(a) << " : " << to_triple(a) << " : " << a << '\n';
		CONSTEXPRESSION blocktriple<9, uint64_t> b = d;
		cout << to_binary(b) << " : " << to_triple(b) << " : " << b << '\n';
		CONSTEXPRESSION blocktriple<10, uint64_t> c = d;
		cout << to_binary(c) << " : " << to_triple(c) << " : " << c << '\n';
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

		bfloat<8, 2, uint8_t> a, b, c;
		a = 1.0f;
		b = -1.0f;
		constexpr size_t abits = a.fhbits; // a.abits;
		blocktriple<abits, uint8_t> aa, bb;
		blocktriple<abits + 1, uint8_t> sum;
		a.normalize(aa);  // decode of a bits into a triple form aa
		b.normalize(bb);  // decode of b bits into a triple form bb
		sum.add(aa, bb);  // ALU add operator
		convert(sum, c);
		cout << to_triple(sum) << " : " << sum << '\n';
		cout << color_print(c) << " : " << c << endl;

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
