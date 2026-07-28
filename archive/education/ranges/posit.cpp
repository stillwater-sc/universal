// posit.cpp : report dynamic range of posit configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/posit1/posit1.hpp>

#ifdef UNIVERSAL_ORACLE_ENABLED
// TODO: this needs to be done with an adaptive precision floating-point as these scale factors grow very large
constexpr size_t MAX_ES = 5;
constexpr size_t MAX_K = 10;
uint64_t GENERATED_SCALE_FACTORS[MAX_ES][MAX_K];

void generateScaleFactorLookupTable() {
	uint64_t useed, useed_power_k;
	for (size_t es = 0; es < MAX_ES; es++) {
		useed = two_to_the_power(two_to_the_power(es));
		useed_power_k = useed; 
		GENERATED_SCALE_FACTORS[es][0] = 1; // for k = 0
		for (size_t k = 1; k < MAX_K; k++) {
			useed_power_k *= useed;
			GENERATED_SCALE_FACTORS[es][k] = useed_power_k;
		}
	}
}

void printScaleFactors(uint64_t scale_factors[MAX_ES][MAX_K]) {
	cout << "      ";
	for (size_t k = 0; k < MAX_K; k++) {
		cout << "     k = " << k << "   ";
	}
	cout << endl;
	for (size_t es = 0; es < MAX_ES; es++) {
		cout << "es = " << es << " ";
		for (size_t k = 0; k < MAX_K; k++) {
			cout << setw(12) << scale_factors[es][k] << " ";
		}
		cout << endl;
	}
	cout << endl;
}
#endif

// print scales for small posit configurations
void ReportSmallPositScales() {
	using namespace sw::universal;

	std::cout << std::endl;
	std::cout << "Scales are represented as the binary scale of the number: i.e. 2^scale" << std::endl << std::endl;
	std::cout << "Small, specialized posit configurations" << std::endl;

	std::cout << "nbits = 2" << std::endl;
	std::cout << posit_range<2, 0>() << std::endl;
	std::cout << posit_range<2, 1>() << std::endl;
	std::cout << posit_range<2, 2>() << std::endl;
	std::cout << posit_range<2, 3>() << std::endl;
	std::cout << "nbits = 3" << std::endl;
	std::cout << posit_range<3, 0>() << std::endl;
	std::cout << posit_range<3, 1>() << std::endl;
	std::cout << posit_range<3, 2>() << std::endl;
	std::cout << posit_range<3, 3>() << std::endl;
	std::cout << "nbits = 4" << std::endl;
	std::cout << posit_range<4, 0>() << std::endl;
	std::cout << posit_range<4, 1>() << std::endl;
	std::cout << posit_range<4, 2>() << std::endl;
	std::cout << posit_range<4, 3>() << std::endl;
	std::cout << "nbits = 5" << std::endl;
	std::cout << posit_range<5, 0>() << std::endl;
	std::cout << posit_range<5, 1>() << std::endl;
	std::cout << posit_range<5, 2>() << std::endl;
	std::cout << posit_range<5, 3>() << std::endl;
}

// print scales of different posit configurations
void ReportStandardPositScales() {
	using namespace sw::universal;

	std::cout << "es = 0" << std::endl;
	std::cout << posit_range<8, 0>() << std::endl;
	std::cout << posit_range<16, 0>() << std::endl;
	std::cout << posit_range<32, 0>() << std::endl;
	std::cout << posit_range<64, 0>() << std::endl;
	std::cout << posit_range<128, 0>() << std::endl;
	std::cout << posit_range<256, 0>() << std::endl;

	std::cout << "es = 1" << std::endl;
	std::cout << posit_range<8, 1>() << std::endl;
	std::cout << posit_range<16, 1>() << std::endl;
	std::cout << posit_range<32, 1>() << std::endl;
	std::cout << posit_range<64, 1>() << std::endl;
	std::cout << posit_range<128, 1>() << std::endl;
	std::cout << posit_range<256, 1>() << std::endl;

	std::cout << "es = 2" << std::endl;
	std::cout << posit_range<8, 2>() << std::endl;
	std::cout << posit_range<16, 2>() << std::endl;
	std::cout << posit_range<32, 2>() << std::endl;
	std::cout << posit_range<64, 2>() << std::endl;
	std::cout << posit_range<128, 2>() << std::endl;
	std::cout << posit_range<256, 2>() << std::endl;

	std::cout << "es = 3" << std::endl;
	std::cout << posit_range<8, 3>() << std::endl;
	std::cout << posit_range<16, 3>() << std::endl;
	std::cout << posit_range<32, 3>() << std::endl;
	std::cout << posit_range<64, 3>() << std::endl;
	std::cout << posit_range<128, 3>() << std::endl;
	std::cout << posit_range<256, 3>() << std::endl;

	std::cout << "es = 4" << std::endl;
	std::cout << posit_range<8, 4>() << std::endl;
	std::cout << posit_range<16, 4>() << std::endl;
	std::cout << posit_range<32, 4>() << std::endl;
	std::cout << posit_range<64, 4>() << std::endl;
	std::cout << posit_range<128, 4>() << std::endl;
	std::cout << posit_range<256, 4>() << std::endl;

	std::cout << "es = 5" << std::endl;
	std::cout << posit_range<8, 5>() << std::endl;
	std::cout << posit_range<16, 5>() << std::endl;
	std::cout << posit_range<32, 5>() << std::endl;
	std::cout << posit_range<64, 5>() << std::endl;
	std::cout << posit_range<128, 5>() << std::endl;
	std::cout << posit_range<256, 5>() << std::endl;
}

// print scales of different posit configurations
// useed = 2^(2^es) and thus is just a function of the exponent configuration
// maxpos = useed^(nbits-2)
// minpos = useed^(2-nbits)
void ReportPositScales() {
	using namespace sw::universal;
	posit<3, 0> p3_0;
	posit<3, 1> p3_1;
	posit<3, 2> p3_2;
	posit<3, 3> p3_3;
	posit<3, 4> p3_4;
	posit<4, 0> p4_0;
	posit<4, 1> p4_1;
	posit<4, 2> p4_2;
	posit<4, 3> p4_3;
	posit<4, 4> p4_4;
	posit<5, 0> p5_0;
	posit<5, 1> p5_1;
	posit<5, 2> p5_2;
	posit<5, 3> p5_3;
	posit<5, 4> p5_4;
	posit<6, 0> p6_0;
	posit<6, 1> p6_1;
	posit<6, 2> p6_2;
	posit<6, 3> p6_3;
	posit<6, 4> p6_4;
	posit<7, 0> p7_0;
	posit<7, 1> p7_1;
	posit<7, 2> p7_2;
	posit<7, 3> p7_3;
	posit<7, 4> p7_4;
	posit<8, 0> p8_0;
	posit<8, 1> p8_1;
	posit<8, 2> p8_2;
	posit<8, 3> p8_3;
	posit<8, 4> p8_4;
	posit<9, 0> p9_0;
	posit<9, 1> p9_1;
	posit<9, 2> p9_2;
	posit<9, 3> p9_3;
	posit<9, 4> p9_4;
	posit<10, 0> p10_0;
	posit<10, 1> p10_1;
	posit<10, 2> p10_2;
	posit<10, 3> p10_3;
	posit<10, 4> p10_4;
	posit<11, 0> p11_0;
	posit<11, 1> p11_1;
	posit<11, 2> p11_2;
	posit<11, 3> p11_3;
	posit<11, 4> p11_4;
	posit<12, 0> p12_0;
	posit<12, 1> p12_1;
	posit<12, 2> p12_2;
	posit<12, 3> p12_3;
	posit<12, 4> p12_4;


	posit<16, 0> p16_0;
	posit<16, 1> p16_1;
	posit<16, 2> p16_2;
	posit<16, 3> p16_3;
	posit<16, 4> p16_4;
	posit<32, 0> p32_0;
	posit<32, 1> p32_1;
	posit<32, 2> p32_2;
	posit<32, 3> p32_3;
	posit<32, 4> p32_4;
	posit<64, 0> p64_0;
	posit<64, 1> p64_1;
	posit<64, 2> p64_2;
	posit<64, 3> p64_3;
	posit<64, 4> p64_4;

	posit<20, 1> p20_1;
	posit<24, 1> p24_1;
	posit<28, 1> p28_1;
	posit<40, 2> p40_2;
	posit<48, 2> p48_2;
	posit<56, 2> p56_2;

	posit<80, 2> p80_2;
	posit<80, 3> p80_3;
	posit<80, 4> p80_4;
	posit<96, 2> p96_2;
	posit<96, 3> p96_3;
	posit<96, 4> p96_4;
	posit<112, 2> p112_2;
	posit<112, 3> p112_3;
	posit<112, 4> p112_4;
	posit<128, 2> p128_2;
	posit<128, 3> p128_3;
	posit<128, 4> p128_4;

	std::cout << "Posit specificiation examples and their ranges:" << std::endl;
	std::cout << "Scales are represented as the binary scale of the number: i.e. 2^scale" << std::endl << std::endl;
	std::cout << "Small, specialized posit configurations" << std::endl;
	std::cout << "nbits = 3" << std::endl;
	std::cout << dynamic_range(p3_0) << std::endl;
	std::cout << dynamic_range(p3_1) << std::endl;
	std::cout << dynamic_range(p3_2) << std::endl;
	std::cout << dynamic_range(p3_3) << std::endl;
	std::cout << dynamic_range(p3_4) << std::endl;
	std::cout << "nbits = 4" << std::endl;
	std::cout << dynamic_range(p4_0) << std::endl;
	std::cout << dynamic_range(p4_1) << std::endl;
	std::cout << dynamic_range(p4_2) << std::endl;
	std::cout << dynamic_range(p4_3) << std::endl;
	std::cout << dynamic_range(p4_4) << std::endl;
	std::cout << "nbits = 5" << std::endl;
	std::cout << dynamic_range(p5_0) << std::endl;
	std::cout << dynamic_range(p5_1) << std::endl;
	std::cout << dynamic_range(p5_2) << std::endl;
	std::cout << dynamic_range(p5_3) << std::endl;
	std::cout << dynamic_range(p5_4) << std::endl;
	std::cout << "nbits = 6" << std::endl;
	std::cout << dynamic_range(p6_0) << std::endl;
	std::cout << dynamic_range(p6_1) << std::endl;
	std::cout << dynamic_range(p6_2) << std::endl;
	std::cout << dynamic_range(p6_3) << std::endl;
	std::cout << dynamic_range(p6_4) << std::endl;
	std::cout << "nbits = 7" << std::endl;
	std::cout << dynamic_range(p7_0) << std::endl;
	std::cout << dynamic_range(p7_1) << std::endl;
	std::cout << dynamic_range(p7_2) << std::endl;
	std::cout << dynamic_range(p7_3) << std::endl;
	std::cout << dynamic_range(p7_4) << std::endl;
	std::cout << "nbits = 8" << std::endl;
	std::cout << dynamic_range(p8_0) << std::endl;
	std::cout << dynamic_range(p8_1) << std::endl;
	std::cout << dynamic_range(p8_2) << std::endl;
	std::cout << dynamic_range(p8_3) << std::endl;
	std::cout << dynamic_range(p8_4) << std::endl;
	std::cout << "nbits = 9" << std::endl;
	std::cout << dynamic_range(p9_0) << std::endl;
	std::cout << dynamic_range(p9_1) << std::endl;
	std::cout << dynamic_range(p9_2) << std::endl;
	std::cout << dynamic_range(p9_3) << std::endl;
	std::cout << dynamic_range(p9_4) << std::endl;
	std::cout << "nbits = 10" << std::endl;
	std::cout << dynamic_range(p10_0) << std::endl;
	std::cout << dynamic_range(p10_1) << std::endl;
	std::cout << dynamic_range(p10_2) << std::endl;
	std::cout << dynamic_range(p10_3) << std::endl;
	std::cout << dynamic_range(p10_4) << std::endl;
	std::cout << "nbits = 11" << std::endl;
	std::cout << dynamic_range(p11_0) << std::endl;
	std::cout << dynamic_range(p11_1) << std::endl;
	std::cout << dynamic_range(p11_2) << std::endl;
	std::cout << dynamic_range(p11_3) << std::endl;
	std::cout << dynamic_range(p11_4) << std::endl;
	std::cout << "nbits = 12" << std::endl;
	std::cout << dynamic_range(p12_0) << std::endl;
	std::cout << dynamic_range(p12_1) << std::endl;
	std::cout << dynamic_range(p12_2) << std::endl;
	std::cout << dynamic_range(p12_3) << std::endl;
	std::cout << dynamic_range(p12_4) << std::endl;
	std::cout << "Standard posit configurations" << std::endl;
	std::cout << dynamic_range(p8_0) << std::endl;
	std::cout << dynamic_range(p16_1) << std::endl;
	std::cout << dynamic_range(p32_2) << std::endl;
	std::cout << dynamic_range(p64_3) << std::endl;
	std::cout << "Extended Standard posit configurations" << std::endl;
	std::cout << dynamic_range(p4_0) << std::endl;	
	std::cout << dynamic_range(p8_0) << std::endl;
	std::cout << dynamic_range(p12_0) << std::endl;
	std::cout << dynamic_range(p16_1) << std::endl;
	std::cout << dynamic_range(p20_1) << std::endl;
	std::cout << dynamic_range(p24_1) << std::endl;
	std::cout << dynamic_range(p28_1) << std::endl;
	std::cout << dynamic_range(p32_2) << std::endl;
	std::cout << dynamic_range(p40_2) << std::endl;
	std::cout << dynamic_range(p48_2) << std::endl;
	std::cout << dynamic_range(p56_2) << std::endl;
	std::cout << dynamic_range(p64_3) << std::endl;
	std::cout << "Extended Modified Standard posit configurations" << std::endl;
	std::cout << dynamic_range(p4_0) << std::endl;
	std::cout << dynamic_range(p4_1) << std::endl;
	std::cout << dynamic_range(p8_0) << std::endl;
	std::cout << dynamic_range(p8_1) << std::endl;
	std::cout << dynamic_range(p8_2) << std::endl;
	std::cout << dynamic_range(p8_3) << std::endl;
	std::cout << dynamic_range(p8_4) << std::endl;
	std::cout << dynamic_range(p16_0) << std::endl;
	std::cout << dynamic_range(p16_1) << std::endl;
	std::cout << dynamic_range(p16_2) << std::endl;
	std::cout << dynamic_range(p16_3) << std::endl;
	std::cout << dynamic_range(p16_4) << std::endl;
	std::cout << dynamic_range(p32_0) << std::endl;
	std::cout << dynamic_range(p32_1) << std::endl;
	std::cout << dynamic_range(p32_2) << std::endl;
	std::cout << dynamic_range(p32_3) << std::endl;
	std::cout << dynamic_range(p32_4) << std::endl;
	std::cout << dynamic_range(p64_0) << std::endl;
	std::cout << dynamic_range(p64_1) << std::endl;
	std::cout << dynamic_range(p64_2) << std::endl;
	std::cout << dynamic_range(p64_3) << std::endl;
	std::cout << dynamic_range(p64_4) << std::endl;
	std::cout << "Ginormous posit configurations" << std::endl;
	std::cout << dynamic_range(p80_2) << std::endl;
	std::cout << dynamic_range(p80_3) << std::endl;
	std::cout << dynamic_range(p80_4) << std::endl;
	std::cout << dynamic_range(p96_2) << std::endl;
	std::cout << dynamic_range(p96_3) << std::endl;
	std::cout << dynamic_range(p96_4) << std::endl;
	std::cout << dynamic_range(p112_2) << std::endl;
	std::cout << dynamic_range(p112_3) << std::endl;
	std::cout << dynamic_range(p112_4) << std::endl;
	std::cout << dynamic_range(p128_2) << std::endl;
	std::cout << dynamic_range(p128_3) << std::endl;
	std::cout << dynamic_range(p128_4) << std::endl;
	std::cout << std::endl;

	std::cout << "IEEE float configurations from numeric_limits<Ty>" << std::endl;
	std::cout << dynamic_range<float>() << std::endl;
	std::cout << dynamic_range<double>() << std::endl;
	std::cout << dynamic_range<long double>() << std::endl;
}

// enumerate and validate scales
template<size_t nbits, size_t es>
int ValidateScales(std::string& str, bool bReportIndividualTestCases) {
	using namespace sw::universal;
	int nrOfTestFailures = 0;
	constexpr size_t NR_OF_TESTCASES = (size_t(1) << nbits);

	sw::universal::posit<nbits, es> p;
	for (size_t i = 0; i < NR_OF_TESTCASES; ++i) {
		p.set_raw_bits(i);
		constexpr size_t fbits = nbits - 3 - es;
		bool                      _sign;
		positRegime<nbits, es>    _regime;
		positExponent<nbits, es>  _exponent;
		positFraction<fbits>      _fraction;
		decode(p.get(), _sign, _regime, _exponent, _fraction);
		std::cout << _regime << " " << _exponent << " " << _fraction << " regime scale: " << std::setw(3) << _regime.scale() << " exponent scale: " << std::setw(3) << _exponent.scale() << " posit scale: " << std::setw(3) << scale(p) << std::endl;
	}
	return nrOfTestFailures;
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	std::cout << "Experiments with the scale of posit numbers\n";

	std::string tag = "Posit Scales failed";

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	bool bReportIndividualTestCases = false;
	nrOfFailedTestCases += ReportTestResult(ValidateScales<4, 1>(tag, bReportIndividualTestCases), "posit<4,0>", "scales");

#else

	ReportPositScales();
	ReportStandardPositScales();
	ReportSmallPositScales();

#ifdef STRESS_TEST

#endif // STRESS_TESTING


#endif // MANUAL_TESTING
 
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& e) {
	std::cerr << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}



