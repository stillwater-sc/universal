// attributes.cpp: attribute tests for arbitrary configuration posit types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the posit template environment
// first: enable general or specialized configurations
#define POSIT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
// third: enable support for native literals in logic and arithmetic operations
#define POSIT_ENABLE_LITERALS 1
// fourth: enable/disable error-free serialization I/O
#define POSIT_ERROR_FREE_IO_FORMAT 0
// minimum set of include files to reflect source code dependencies
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/test_reporters.hpp>

template<unsigned nbits, unsigned es>
void PositComponents(const std::string& label, const sw::universal::posit<nbits, es>& p) {
	using namespace sw::universal;
	std::cout << "posit component values of a fully articulated standard posit\n";
	std::cout << label << '\n';
	bool s{ false };
	sw::universal::positRegime<nbits, es> r;
	sw::universal::positExponent<nbits, es> e;
	sw::universal::positFraction<nbits - 1ull - es> f;
	sw::universal::decode(p.get(), s, r, e, f);

	std::cout << "raw bits  : " << sw::universal::to_binary(p.bits(), true) << '\n';
	std::cout << "components: " << sw::universal::to_binary(p) << '\n';
	// posit component attribute functions and their equivalence to component value() functions
	std::cout << "sign      : " << (s ? "set" : "not set") << " : " << sw::universal::sign_value(p) << '\n';
	std::cout << "regime    : " << r << " : " << r.value() << " : " << sw::universal::regime_value(p) << '\n';
	std::cout << "exponent  : " << e << " : " << e.value() << " : " << sw::universal::exponent_value(p) << '\n';
	std::cout << "fraction  : " << f << " : " << f.value() << " : " << sw::universal::fraction_value(p) << '\n';
	std::cout << '\n';
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "generalized posit attribute functions";
	std::string test_tag    = "attributes";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	//// posit attribute functions

	// useed, minpos and maxpos scale
	// what is special with these functions is that they are independent of a posit
	// but associate through their template parameterizations
	{
		std::cout << "useed values for full articulated standard posits\n";
		std::cout << "es\tuseed scale\tuseed value\n";
		std::cout << ES_IS_0 << '\t' << std::setw(5) << useed_scale<ES_IS_0>() << '\t' << std::setw(15) << useed<ES_IS_0>() << '\n';
		std::cout << ES_IS_1 << '\t' << std::setw(5) << useed_scale<ES_IS_1>() << '\t' << std::setw(15) << useed<ES_IS_1>() << '\n';
		std::cout << ES_IS_2 << '\t' << std::setw(5) << useed_scale<ES_IS_2>() << '\t' << std::setw(15) << useed<ES_IS_2>() << '\n';
		std::cout << ES_IS_3 << '\t' << std::setw(5) << useed_scale<ES_IS_3>() << '\t' << std::setw(15) << useed<ES_IS_3>() << '\n';
		std::cout << ES_IS_4 << '\t' << std::setw(5) << useed_scale<ES_IS_4>() << '\t' << std::setw(15) << useed<ES_IS_4>() << '\n';
		std::cout << ES_IS_5 << '\t' << std::setw(5) << useed_scale<ES_IS_5>() << '\t' << std::setw(15) << useed<ES_IS_5>() << '\n';
		std::cout << '\n';
	}

	{
		constexpr size_t nbits = 16;
		constexpr size_t es = 2;
		//using BlockType = std::uint16_t;
		posit<nbits, es> maxpos(SpecificValue::maxpos);
		posit<nbits, es> minpos(SpecificValue::minpos);
		posit<nbits, es> zero(SpecificValue::zero);
		posit<nbits, es> minneg(SpecificValue::minneg);
		posit<nbits, es> maxneg(SpecificValue::maxneg);
		std::cout << "minpos patterns for full articulated standard posits\n";
		std::cout << "minpos : " << to_binary(minpos) << '\t' << minpos_scale<nbits, es>() << '\n';
		std::cout << "zero   : " << to_binary(zero) << '\t' << zero << '\n';
		std::cout << "minneg : " << to_binary(minneg) << '\t' << scale(minneg) << '\n';

		std::cout << "maxpos patterns for full articulated standard posits\n";
		std::cout << "maxpos : " << to_binary(maxpos) << '\t' << maxpos_scale<nbits, es>() << '\n';
		std::cout << "maxneg : " << to_binary(maxneg) << '\t' << scale(maxneg) << '\n';
		std::cout << '\n';
	}

	{
		constexpr unsigned nbits = 16;
		constexpr unsigned es = 2;
		//using BlockType = std::uint16_t;

		PositComponents(std::string("maxpos"), posit<nbits, es>(SpecificValue::maxpos));
		PositComponents(std::string("minpos"), posit<nbits, es>(SpecificValue::minpos));
		PositComponents(std::string("zero"), posit<nbits, es>(SpecificValue::zero));
		PositComponents(std::string("minneg"), posit<nbits, es>(SpecificValue::minneg));
		PositComponents(std::string("maxneg"), posit<nbits, es>(SpecificValue::maxneg));

		std::cout << components(posit<nbits, es>(SpecificValue::maxpos)) << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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


/*
Dynamic ranges of different specializations of an 8-bit generalized posit
sw::universal::posit<  8, 0, unsigned char> : minexp scale         -6     maxexp scale          6     minimum     0.015625     maximum           64
sw::universal::posit<  8, 1, unsigned char> : minexp scale        -12     maxexp scale         12     minimum  0.000244141     maximum         4096
sw::universal::posit<  8, 2, unsigned char> : minexp scale        -24     maxexp scale         24     minimum  5.96046e-08     maximum  1.67772e+07
sw::universal::posit<  8, 3, unsigned char> : minexp scale        -48     maxexp scale         48     minimum  3.55271e-15     maximum  2.81475e+14
sw::universal::posit<  8, 4, unsigned char> : minexp scale        -96     maxexp scale         96     minimum  1.26218e-29     maximum  7.92282e+28

Dynamic ranges of the standard posit configurations
sw::universal::posit<  8, 2, unsigned char> : min   5.96046e-08     max   1.67772e+07
sw::universal::posit< 16, 2, unsigned char> : min   1.38778e-17     max   7.20576e+16
sw::universal::posit< 32, 2, unsigned char> : min   7.52316e-37     max   1.32923e+36
sw::universal::posit< 64, 2, unsigned char> : min   2.21086e-75     max   4.52313e+74
sw::universal::posit<128, 2, unsigned char> : min  1.90934e-152     max  5.23742e+151
sw::universal::posit<256, 2, unsigned char> : min  1.42405e-306     max  7.02224e+305

Dynamic ranges of the standard posit configurations
sw::universal::posit<  8, 2, unsigned char> : [         -1.67772e+07,         -5.96046e-08       0           5.96046e-08,          1.67772e+07]
sw::universal::posit< 16, 2, unsigned char> : [         -7.20576e+16,         -1.38778e-17       0           1.38778e-17,          7.20576e+16]
sw::universal::posit< 32, 2, unsigned char> : [         -1.32923e+36,         -7.52316e-37       0           7.52316e-37,          1.32923e+36]
sw::universal::posit< 64, 2, unsigned char> : [         -4.52313e+74,         -2.21086e-75       0           2.21086e-75,          4.52313e+74]
sw::universal::posit<128, 2, unsigned char> : [        -5.23742e+151,        -1.90934e-152       0          1.90934e-152,         5.23742e+151]
sw::universal::posit<256, 2, unsigned char> : [        -7.02224e+305,        -1.42405e-306       0          1.42405e-306,         7.02224e+305]
 */
