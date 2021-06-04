// api.cpp: application programming interface tests for cfloat number system
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
// Configure the cfloat template environment
// first: enable general or specialized configurations
#define CFLOAT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable trace conversion
#define TRACE_CONVERSION 0

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/cfloat/manipulators.hpp>  // hex_print and the like
#include <universal/verification/test_suite_arithmetic.hpp>

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	int nrOfFailedTestCases = 0;

	std::cout << "cfloat<> Application Programming Interface tests" << std::endl;

#if MANUAL_TESTING

	{
		using bt = uint8_t;
		using Real = cfloat<8, 3, bt>;
		Real a(1.0f);
		Real b(1.0f);
		Real c{ 0 };
		// emulate addition
		constexpr size_t fbits = Real::fbits;
		blocktriple<fbits, bt> _a, _b;
		a.normalize(_a);
		b.normalize(_b);
		std::cout << to_binary(a) << " : " << to_triple(a) << std::endl;
	}

	{
		cfloat<16, 4, uint16_t> a(1.0);
		cfloat<16, 4, uint16_t> b;
		b = 1.5f;
		blocktriple<12> bt;
//		b.normalize(bt);
		std::cout << to_binary(b) << " : " << b << " : " << to_binary(bt) << std::endl;
		std::cout << color_print(b) << std::endl;
		cfloat<16, 4, uint16_t> c;
		c = a * b;
	}

	{
		using BlockType = uint32_t;
		float subnormal = std::nextafter(0.0f, 1.0f);
		cfloat<32, 8, BlockType> a;
		blockbinary<a.fhbits, BlockType> significant;
		std::cout << "   cfloat<32,8,uint32_t>         IEEE-754 float\n";
		uint32_t pattern = 0x00000001ul;
		for (unsigned i = 0; i < 24; ++i) {
			a.setbits(pattern);
			std::cout << to_binary(a, true) << " " << a << ": ";
			pattern <<= 1;
			std::cout << to_binary(subnormal, true) << " : " << subnormal << std::endl;
			subnormal *= 2.0f;

			size_t scale_offset = a.significant(significant);
			std::cout << to_binary(significant, true) << " : " << a.MIN_EXP_SUBNORMAL << " : " << a.MIN_EXP_NORMAL - scale_offset << " vs " << a.scale() << std::endl;
		}
	}

	int exponents[] = {
		0, 1, 0, -2, -6, -14, -30, -62, -126, -254, -510, -1022
	};
	for (int i = 1; i < 12; ++i) {
		std::cout << "es = " << exponents[i] << " " << std::setprecision(17) << subnormal_exponent[i] << std::endl;
	}

	std::cout << "Number of failed test cases : " << nrOfFailedTestCases << std::endl;
	nrOfFailedTestCases = 0; // disregard any test failures in manual testing mode

#else // !MANUAL_TESTING

	// construction
	{
		int start = nrOfFailedTestCases;
		cfloat<8, 2, uint8_t> zero, a(2.0), b(2.0), c(1.0), d(4.0);
		if (zero != (a - b)) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
			cout << "FAIL : " << a << ' ' << b << ' ' << c << ' ' << d << endl;
		}
	}

	{
		cfloat<8, 2> a;
		std::cout << "maxpos : " << maxpos(a) << " : " << scale(a) << '\n';
		std::cout << "minpos : " << minpos(a) << " : " << scale(a) << '\n';
		std::cout << "zero   : " << zero(a) << " : " << scale(a) << '\n';
		std::cout << "minneg : " << minneg(a) << " : " << scale(a) << '\n';
		std::cout << "maxneg : " << maxneg(a) << " : " << scale(a) << '\n';
		std::cout << dynamic_range(a) << std::endl;
	}

#endif // MANUAL_TESTING

	std::cout << "\nCFLOAT API test suite           : " << (nrOfFailedTestCases == 0 ? "PASS\n" : "FAIL\n");

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
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
