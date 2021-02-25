// api.cpp: application programming interface tests for bfloat number system
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if defined(_MSC_VER)
#pragma warning(disable : 4514)  // unreferenced function is removed
#pragma warning(disable : 4710)  // function is not inlined
#endif
#include <iostream>
#include <iomanip>
#include <fstream>
#include <typeinfo>
// minimum set of include files to reflect source code dependencies
#include <universal/number/bfloat/bfloat.hpp>
#include <universal/number/bfloat/manipulators.hpp>  // hex_print and the like
#include <universal/verification/test_suite_arithmetic.hpp>



#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc > 0) std::cout << argv[0] << std::endl;

	int nrOfFailedTestCases = 0;

	std::cout << "bfloat<> Application Programming Interface tests" << std::endl;

#if MANUAL_TESTING

	// scales for gradual overflow range are incorrect
    // also scales for es = 1 are just underflow and overflow ranges, and currently incorrect

	/// TODO: subnormal numbers have a scale adjustment as 2^(2-2^(es - 1)).
	/// check if this is correct if es is > 2. In particular, bfloat<32,8> and bfloat<64,11> should write test suite for that

	{
		bfloat<16, 4, uint16_t> a(1.0);
		bfloat<16, 4, uint16_t> b;
		b = 1.5f;
		blocktriple<12, uint16_t> bt;
		b.normalize(bt);
		std::cout << to_binary(b) << " : " << b << " : " << to_binary(bt) << std::endl;
		std::cout << color_print(b) << std::endl;
		bfloat<16, 4, uint16_t> c;
		c = a * b;
	}

	{
		/*
		nbits             : 44
		es                : 9
		ALL_ONES          : b1111'1111'1111'1111
		BLOCK_MASK        : b1111'1111'1111'1111
		nrBlocks          : 3
		bits in MSU       : 12
		MSU               : 2
		MSU MASK          : b0000'1111'1111'1111
		SIGN_BIT_MASK     : b0000'1000'0000'0000
		LSB_BIT_MASK      : b0000'0000'0000'0001
		MSU CAPTURES E    : yes
		EXP_SHIFT         : 34
		MSU EXP MASK      : b0000'0000'0000'0000
		EXP_BIAS          : 255
		MAX_EXP           : 257
		MIN_EXP_NORMAL    : -254
		MIN_EXP_SUBNORMAL : -288
		*/
		bfloat<44, 9, uint16_t> a;
		a.constexprParameters();
	}

	{
		using BlockType = uint32_t;
		float subnormal = std::nextafter(0.0f, 1.0f);
		bfloat<32, 8, BlockType> a;
		blockbinary<a.fhbits, BlockType> significant;
		std::cout << "   bfloat<32,8,uint32_t>         IEEE-754 float\n";
		uint32_t pattern = 0x00000001ul;
		for (unsigned i = 0; i < 24; ++i) {
			a.set_raw_bits(pattern);
			std::cout << to_binary(a, true) << " " << a << ": ";
			pattern <<= 1;
			std::cout << to_binary(subnormal, true) << " : " << subnormal << std::endl;
			subnormal *= 2.0f;

			int scale_offset = a.significant(significant);
			std::cout << to_binary(significant, true) << " : " << a.MIN_EXP_SUBNORMAL << " : " << a.MIN_EXP_NORMAL - scale_offset << " vs " << a.scale() << std::endl;
		}
	}

	int exponents[] = {
		0, 1, 0, -2, -6, -14, -30, -62, -126, -254, -510, -1022
	};
	for (int i = 1; i < 12; ++i) {
		double e{ 0.0 };
		std::cout << "es = " << exponents[i] << " " << std::setprecision(17) << subnormal_exponent[i] << std::endl;
	}

#else // !MANUAL_TESTING

	// construction
	{
		int start = nrOfFailedTestCases;
		bfloat<8, 2, uint8_t> zero, a(2.0), b(2.0), c(1.0), d(4.0);
		if (zero != (a - b)) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
			cout << "FAIL : " << a << ' ' << b << ' ' << c << ' ' << d << endl;
		}
	}

	{
		bfloat<8, 2> a;
		std::cout << "maxpos : " << maxpos(a) << " : " << scale(a) << '\n';
		std::cout << "minpos : " << minpos(a) << " : " << scale(a) << '\n';
		std::cout << "zero   : " << zero(a) << " : " << scale(a) << '\n';
		std::cout << "minneg : " << minneg(a) << " : " << scale(a) << '\n';
		std::cout << "maxneg : " << maxneg(a) << " : " << scale(a) << '\n';
		std::cout << dynamic_range(a) << std::endl;
	}

#endif // MANUAL_TESTING

	std::cout << "\nBFLOAT API test suite           : " << (nrOfFailedTestCases == 0 ? "PASS\n" : "FAIL\n");

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
