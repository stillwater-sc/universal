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

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	std::cout << "cfloat<> Application Programming Interface tests\n";

#if MANUAL_TESTING

	// default behavior
	{
		std::cout << "Default cfloat has subnormals, supernormals and is not saturating\n";
		constexpr size_t nbits = 8;
		constexpr size_t es = 3;
		using bt = uint8_t;
		constexpr bool hasSubnormals = true;
		constexpr bool hasSupernormals = true;
		constexpr bool isSaturating = false;
		using Real = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
		Real a(1.0f), b(0.5f), c(0.0);
		c = a + b;
		std::cout << "c = " << c << std::endl;
		c = c - a;
		std::cout << "c = " << c << std::endl;
		c = c * b;
		std::cout << "c = " << c << std::endl;
		std::cout << "---\n";
	}
	{
		constexpr size_t nbits = 8;
		constexpr size_t es = 3;
		using bt = uint8_t;
		constexpr bool hasSubnormals = true;
		constexpr bool hasSupernormals = true;
		constexpr bool isSaturating = false;
		using Real = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;
		CONSTEXPRESSION Real a(1.0f + 0.5f + 0.25f + 0.125f + 0.0625f);
		Real b(-1.0f - 0.5f - 0.25f - 0.125f - 0.0625f);
		constexpr size_t fbits = Real::fbits;
		//constexpr size_t abits = Real::abits;
		//constexpr size_t mbits = Real::mbits;
		//constexpr size_t divbits = Real::divbits;
		{
			// emulate conversion to blocktriple
			blocktriple<fbits, BlockTripleOperator::REPRESENTATION, bt> _a, _b;
			a.normalize(_a);
			b.normalize(_b);
			std::cout << to_binary(a) << " : " << to_triple(_a) << '\n';
			std::cout << to_binary(b) << " : " << to_triple(_b) << '\n';
			std::cout << hex_print(a) << '\n';
			std::cout << "========  end of representation  =========\n\n";
		}

		{
			Real c = a + b;
			std::cout << "Result of addition       : " << color_print(c) << '\n';

			// emulate addition
			blocktriple<fbits, BlockTripleOperator::ADD, bt> _a, _b, _c;
			a.normalizeAddition(_a);
			b.normalizeAddition(_b);
			_c.add(_a, _b);
			std::cout << to_binary(a) << " : " << to_triple(_a) << std::endl;
			std::cout << to_binary(b) << " : " << to_triple(_b) << std::endl;
			std::cout << to_binary(c) << " : " << to_triple(_c) << std::endl;
			std::cout << "+++++++++    end of addition    ++++++++++\n\n";
		}

		{
			Real c = a * b;
			std::cout << "result of multiplication : " << color_print(c) << '\n';

			// emulate multiplication
			blocktriple<fbits, BlockTripleOperator::MUL, bt> _a, _b, _c;
			a.normalizeMultiplication(_a);
			b.normalizeMultiplication(_b);
			_c.mul(_a, _b);
			std::cout << to_binary(a) << " : " << to_triple(_a) << std::endl;
			std::cout << to_binary(b) << " : " << to_triple(_b) << std::endl;
			std::cout << to_binary(c) << " : " << to_triple(_c) << std::endl;
			std::cout << "********* end of multiplication **********\n\n";
		}

		{
			Real c = a / b;
			std::cout << "Result of division       : " << color_print(c) << '\n';

			// emulate division
			blocktriple<fbits, BlockTripleOperator::DIV, bt> _a, _b, _c;
			a.normalizeDivision(_a);
			b.normalizeDivision(_b);
			_c.div(_a, _b);
			std::cout << to_binary(a) << " : " << to_triple(_a) << std::endl;
			std::cout << to_binary(b) << " : " << to_triple(_b) << std::endl;
			std::cout << to_binary(c) << " : " << to_triple(_c) << std::endl;
			std::cout << "/////////    end of division    //////////\n\n";
		}
	}

	{
		// report on the dynamic range of some standard configurations
		using quarter = cfloat<  8,  2, uint8_t, true, true, false>;
		using half    = cfloat< 16,  5, uint8_t, true, true, false>;
		using single  = cfloat< 32,  8, uint8_t, true, false, false>;
		using dual    = cfloat< 64, 11, uint8_t, true, false, false>;
		using quad    = cfloat<128, 15, uint8_t, true, false, false>;
		using octo    = cfloat<256, 18, uint8_t, true, false, false>;
		report_range<quarter>(std::cout);
		report_range<half>(std::cout);
		report_range<single>(std::cout);
		report_range<dual>(std::cout);
		report_range<quad>(std::cout);
		report_range<octo>(std::cout);

		quarter q;
		q.setbits(0x01);  // smallest subnormal
		std::cout << "minpos  cfloat<8,2> : " << to_binary(q) << " : " << q << '\n';
		q.setbits(0x5f);  // max normal
		std::cout << "maxnorm cfloat<8,2> : " << to_binary(q) << " : " << q << '\n';
		q.setbits(0x7d);  // max supernormal
		std::cout << "maxpos  cfloat<8,2> : " << to_binary(q) << " : " << q << '\n';

		half h;
		h.setbits(0x0001); // smallest subnormal
		std::cout << "minpos  cfloat<16,5>: " << to_binary(h) << " : " << h << '\n';
		h.setbits(0x7bff);  // max normal
		std::cout << "maxnorm cfloat<16,5>: " << to_binary(h) << " : " << h << '\n';
		h.setbits(0x7ffd);  // max supernormal
		std::cout << "maxpos  cfloat<16,5>: " << to_binary(h) << " : " << h << '\n';

		using QuarterNormal = cfloat<  8, 2, uint8_t, false, false, false>; // no sub or supernormals
		QuarterNormal qn;
		qn.minpos();
		std::cout << "minpos quarterNormal: " << to_binary(qn) << " : " << qn << '\n';
		qn.maxpos();
		std::cout << "maxpos quarterNormal: " << to_binary(qn) << " : " << qn << '\n';

		using halfNormal = cfloat< 16, 5, uint16_t, false, false, false>; // no sub or supernormals
		halfNormal hn;
		hn.minpos();
		std::cout << "minpos halfNormal   : " << to_binary(hn) << " : " << hn << '\n';
		hn.maxpos();
		std::cout << "maxpos halfNormal   : " << to_binary(hn) << " : " << hn << '\n';
	}

	return 0;
	{
		using BlockType = uint32_t;
		float subnormal = std::nextafter(0.0f, 1.0f);
		cfloat<32, 8, BlockType> a;
		blockbinary<a.fhbits, BlockType> significant;
		std::cout << "   cfloat<32,8,uint32_t>         IEEE-754 float subnormals\n";
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

	std::cout << "Subnormal exponent values\n";
	// we are not using element [0] as es = 0 is not supported in the cfloat spec
	int exponents[] = {
		0, 1, 0, -2, -6, -14, -30, -62, -126, -254, -510, -1022
	};
	for (int i = 1; i < 12; ++i) {
		std::cout << "es = " << i << " = " << exponents[i] << " " << std::setprecision(17) << subnormal_exponent[i] << std::endl;
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
