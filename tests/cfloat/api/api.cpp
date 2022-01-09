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
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "cfloat<> Application Programming Interface tests";
	int nrOfFailedTestCases = 0;

	// default behavior
	{
		std::cout << "Default cfloat has no subnormals, no supernormals and is not saturating\n";
		constexpr size_t nbits = 8;
		constexpr size_t es = 3;
		using Real = cfloat<nbits, es>;  // bt = uint8_t, hasSubnormals = false, hasSupernormals = false, isSaturating = false

		Real a(1.0f), b(0.5f), c(0.0);
		std::cout << type_tag(a) << '\n';
		c = a + b;
		std::cout << "c = " << c << '\n';
		c = c - a;
		std::cout << "c = " << c << '\n';
		c = c * b;
		std::cout << "c = " << c << '\n';
		std::cout << "---\n";
	}

	// explicit configuration
	{
		std::cout << "Explicit configuration of a cfloat\n";
		constexpr size_t nbits = 8;
		constexpr size_t es = 3;
		using bt = uint8_t;
		constexpr bool hasSubnormals   = true;
		constexpr bool hasSupernormals = true;
		constexpr bool isSaturating    = false;
		using Real = cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating>;

		Real a(1.0f), b(0.5f), c(0.0);
		std::cout << type_tag(a) << '\n';
		c = a + b;
		std::cout << "c = " << c << '\n';
		c = c - a;
		std::cout << "c = " << c << '\n';
		c = c * b;
		std::cout << "c = " << c << '\n';
		std::cout << "---\n";
	}

	// report on the dynamic range of some standard configurations
	{
		std::cout << "Dynamic ranges of some standard cfloat<> configurations\n";
		using quarter = cfloat<  8,  2, uint8_t, false, false, false>;
		using half    = cfloat< 16,  5, uint8_t, false, false, false>;
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

		std::cout << "---\n";

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

		std::cout << "---\n";
	}

	// constexpr and specific values
	{
		std::cout << "constexpr and specific values\n";
		constexpr size_t nbits = 10;
		constexpr size_t es = 3;
		using Real = cfloat<nbits, es>;  // bt = uint8_t, hasSubnormals = false, hasSupernormals = false, isSaturating = false

		CONSTEXPRESSION Real a; // zero constexpr
		std::cout << type_tag(a) << '\n';

		CONSTEXPRESSION Real b(1.0f);  // constexpr of a native type conversion
		std::cout << to_binary(b) << " : " << b << '\n';

		CONSTEXPRESSION Real c(SpecificValue::minpos);  // constexpr of a special value in the encoding
		std::cout << to_binary(c) << " : " << c << " == minpos" << '\n';

		CONSTEXPRESSION Real d(SpecificValue::maxpos);  // constexpr of a special value in the encoding
		std::cout << to_binary(d) << " : " << d << " == maxpos" << '\n';
	}

	// set bit patterns
	{
		std::cout << "set bit patterns API\n";
		constexpr size_t nbits = 16;
		constexpr size_t es = 5;
		using Real = cfloat<nbits, es>;  // bt = uint8_t, hasSubnormals = false, hasSupernormals = false, isSaturating = false

		Real a;
		std::cout << type_tag(a) << '\n';

		a.setbits(0x0000);
		std::cout << to_binary(a) << " : " << a << '\n';

		a.setbits(0xAAAA);
		std::cout << to_binary(a) << " : " << a << '\n';

		a.assign(std::string("0b1.01010.1010'1010'10"));
		std::cout << to_binary(a) << " : " << a << '\n';

		a.assign(std::string("0b1.01010.10'1010'1010"));
		std::cout << to_binary(a) << " : " << a << '\n';
	}

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

	{
		std::cout << "Subnormal exponent values\n";
		// we are not using element [0] as es = 0 is not supported in the cfloat spec
		int exponents[] = {
			0, 1, 0, -2, -6, -14, -30, -62, -126, -254, -510, -1022
		};
		for (int i = 1; i < 12; ++i) {
			std::cout << "es = " << i << " = " << exponents[i] << " " << std::setprecision(17) << subnormal_exponent[i] << std::endl;
		}
	}

	std::cout << "Number of failed test cases : " << nrOfFailedTestCases << std::endl;
	nrOfFailedTestCases = 0; // disregard any test failures in manual testing mode


	// construction
	{
		int start = nrOfFailedTestCases;
		cfloat<8, 2, uint8_t> zero, a(2.0), b(2.0), c(1.0), d(4.0);
		if (zero != (a - b)) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL : " << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		}
	}

	{
		cfloat<8, 2> a;
		std::cout << "maxpos : " << a.maxpos() << " : " << scale(a) << '\n';
		std::cout << "minpos : " << a.minpos() << " : " << scale(a) << '\n';
		std::cout << "zero   : " << a.zero() << " : " << scale(a) << '\n';
		std::cout << "minneg : " << a.minneg() << " : " << scale(a) << '\n';
		std::cout << "maxneg : " << a.maxneg() << " : " << scale(a) << '\n';
		std::cout << dynamic_range(a) << std::endl;
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