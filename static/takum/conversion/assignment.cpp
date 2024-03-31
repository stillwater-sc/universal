// assignment.cpp: test suite runner for assignments of native types to takums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
// Configure the cfloat template environment
// first: enable general or specialized configurations
#define TAKUM_FAST_SPECIALIZATION
// second: enable/disable cfloat arithmetic exceptions
#define TAKUM_THROW_ARITHMETIC_EXCEPTION 0
// enabling tracing
#define TRACE_CONVERSION 0
#include <universal/number/takum/takum.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>


template<typename TestType, typename NativeFloatingPointType>
void ConversionTest(NativeFloatingPointType& value) {
	using namespace sw::universal;
	std::cout << color_print(value) << " " << value << '\n';
	TestType a = value;
	std::cout << color_print(a) << " " << pretty_print(a) << " " << a << '\n';
}


// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

/*
* e = exponent bit, m = most significant fraction bit, f = fraction bit, h = hidden bit
float       s-eee'eeee'efff'ffff'ffff'ffff'ffff'ffff (23 fraction bits, 1 hidden bit)
                                                                                  float fbits = 0x007F'FFFF  fbits   hidden+raw    0x00FF'FFFF            shift right == 24 - fbits
cfloat<3,1>                                      'sem   fraction = '0000'0000'0000'0000'0000'0000'0000'000h     1    sticky mask = 0x00FF'FFFF   raw+hidden 0x00FF'FFFF >> 23 to get to 0x0000'0001
cfloat<4,1>                                     'semf   fraction = '0000'0000'0000'0000'0000'0000'0000'00h0     2    sticky mask = 0x007F'FFFF   raw+hidden 0x00FF'FFFF >> 22 to get to 0x0000'0003
cfloat<5,1>                                    s'emff   fraction = '0000'0000'0000'0000'0000'0000'0000'0h10     3    sticky mask = 0x003F'FFFF   raw+hidden 0x00FF'FFFF >> 21 to get to 0x0000'0007
cfloat<6,1>                                   se'mfff   fraction = '0000'0000'0000'0000'0000'0000'0000'h110     4    sticky mask = 0x001F'FFFF   raw+hidden 0x00FF'FFFF >> 20 to get to 0x0000'000F
cfloat<7,1>                                  sem'ffff   fraction = '0000'0000'0000'0000'0000'0000'000h'1110     5    sticky mask = 0x000F'FFFF   raw+hidden 0x00FF'FFFF >> 19 to get to 0x0000'001F
cfloat<8,1>                                'semf'ffff   fraction = '0000'0000'0000'0000'0000'0000'00h1'1110     6    sticky mask = 0x0007'FFFF   raw+hidden 0x00FF'FFFF >> 18 to get to 0x0000'003F
cfloat<9,1>                               s'emff'ffff   fraction = '0000'0000'0000'0000'0000'0000'0h11'1110     7    sticky mask = 0x0003'FFFF   raw+hidden 0x00FF'FFFF >> 17 to get to 0x0000'007F
cfloat<10,1>                             se'mfff'ffff   fraction = '0000'0000'0000'0000'0000'0000'h111'1110     8    sticky mask = 0x0001'FFFF   raw+hidden 0x00FF'FFFF >> 16 to get to 0x0000'00FF
cfloat<11,1>                            sem'ffff'ffff   fraction = '0000'0000'0000'0000'0000'000h'1111'1110     9    sticky mask = 0x0000'FFFF   raw+hidden 0x00FF'FFFF >> 15 to get to 0x0000'01FF
cfloat<12,1>                          'semf'ffff'ffff   fraction = '0000'0000'0000'0000'0000'00h1'1111'1110    10    sticky mask = 0x0000'7FFF   raw+hidden 0x00FF'FFFF >> 14 to get to 0x0000'03FF
cfloat<13,1>                         s'emff'ffff'ffff   fraction = '0000'0000'0000'0000'0000'0h11'1111'1110    11    sticky mask = 0x0000'3FFF   raw+hidden 0x00FF'FFFF >> 13 to get to 0x0000'07FF
cfloat<14,1>                        se'mfff'ffff'ffff   fraction = '0000'0000'0000'0000'0000'h111'1111'1110    12    sticky mask = 0x0000'1FFF   raw+hidden 0x00FF'FFFF >> 12 to get to 0x0000'0FFF
cfloat<15,1>                       sem'ffff'ffff'ffff   fraction = '0000'0000'0000'0000'000h'1111'1111'1110    13    sticky mask = 0x0000'0FFF   raw+hidden 0x00FF'FFFF >> 11 to get to 0x0000'1FFF
cfloat<16,1>                     'semf'ffff'ffff'ffff   fraction = '0000'0000'0000'0000'00h1'1111'1111'1110    14    sticky mask = 0x0000'07FF   raw+hidden 0x00FF'FFFF >> 10 to get to 0x0000'3FFF
cfloat<17,1>                    s'emff'ffff'ffff'ffff   fraction = '0000'0000'0000'0000'0h11'1111'1111'1110    15    sticky mask = 0x0000'03FF   raw+hidden 0x00FF'FFFF >>  9 to get to 0x0000'7FFF
cfloat<18,1>                   se'mfff'ffff'ffff'ffff   fraction = '0000'0000'0000'0000'h111'1111'1111'1110    16    sticky mask = 0x0000'01FF   raw+hidden 0x00FF'FFFF >>  8 to get to 0x0000'FFFF
cfloat<19,1>                  sem'ffff'ffff'ffff'ffff   fraction = '0000'0000'0000'000h'1111'1111'1111'1110    17    sticky mask = 0x0000'00FF   raw+hidden 0x00FF'FFFF >>  7 to get to 0x0001'FFFF
cfloat<20,1>                'semf'ffff'ffff'ffff'ffff   fraction = '0000'0000'0000'00h1'1111'1111'1111'1110    18    sticky mask = 0x0000'007F   raw+hidden 0x00FF'FFFF >>  6 to get to 0x0003'FFFF
cfloat<21,1>               s'emff'ffff'ffff'ffff'ffff   fraction = '0000'0000'0000'0h11'1111'1111'1111'1110    19    sticky mask = 0x0000'003F   raw+hidden 0x00FF'FFFF >>  5 to get to 0x0007'FFFF
cfloat<22,1>              se'mfff'ffff'ffff'ffff'ffff   fraction = '0000'0000'0000'h111'1111'1111'1111'1110    20    sticky mask = 0x0000'001F   raw+hidden 0x00FF'FFFF >>  4 to get to 0x000F'FFFF
cfloat<23,1>             sem'ffff'ffff'ffff'ffff'ffff   fraction = '0000'0000'000h'1111'1111'1111'1111'1110    21    sticky mask = 0x0000'000F   raw+hidden 0x00FF'FFFF >>  3 to get to 0x001F'FFFF
cfloat<24,1>           'semf'ffff'ffff'ffff'ffff'ffff   fraction = '0000'0000'00h1'1111'1111'1111'1111'1110    22    sticky mask = 0x0000'0007   raw+hidden 0x00FF'FFFF >>  2 to get to 0x003F'FFFF
cfloat<25,1>          s'emff'ffff'ffff'ffff'ffff'ffff   fraction = '0000'0000'0h11'1111'1111'1111'1111'1110    23    sticky mask = 0x0000'0003   raw+hidden 0x00FF'FFFF >>  1 to get to 0x007F'FFFF
cfloat<26,1>         se'mfff'ffff'ffff'ffff'ffff'ffff   fraction = '0000'0000'h111'1111'1111'1111'1111'1110    24    sticky mask = 0x0000'0001   raw+hidden 0x00FF'FFFF >>  0 to get to 0x00FF'FFFF
cfloat<27,1>      ' sem'ffff'ffff'ffff'ffff'ffff'ffff   fraction = '0000'000h'1111'1111'1111'1111'1111'1110    25    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -1 to get to 0x01FF'FFFF
cfloat<28,1>      'semf'ffff'ffff'ffff'ffff'ffff'ffff   fraction = '0000'00h1'1111'1111'1111'1111'1111'1110    26    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -2 to get to 0x03FF'FFFF
cfloat<29,1>     s'emff'ffff'ffff'ffff'ffff'ffff'ffff   fraction = '0000'0h11'1111'1111'1111'1111'1111'1110    27    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -3 to get to 0x07FF'FFFF
cfloat<30,1>    se'mfff'ffff'ffff'ffff'ffff'ffff'ffff   fraction = '0000'h111'1111'1111'1111'1111'1111'1110    28    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -4 to get to 0x0FFF'FFFF
cfloat<31,1> ' sem'ffff'ffff'ffff'ffff'ffff'ffff'ffff   fraction = '000h'1111'1111'1111'1111'1111'1111'1110    29    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -5 to get to 0x1FFF'FFFF
cfloat<32,1> 'semf'ffff'ffff'ffff'ffff'ffff'ffff'ffff   fraction = '00h1'1111'1111'1111'1111'1111'1111'1110    30    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -6 to get to 0x3FFF'FFFF

                                                                                 float fbits = 0x007F'FFFF  fbits   hidden+raw    0x00FF'FFFF            shift right == 24 - fbits
cfloat<4,2>                                      seem   fraction = '0000'0000'0000'0000'0000'0000'0000'000h     1    sticky mask = 0x00FF'FFFF   raw+hidden 0x00FF'FFFF >> 23 to get to 0x0000'0001
cfloat<5,2>                                    s'eemf   fraction = '0000'0000'0000'0000'0000'0000'0000'00h0     2    sticky mask = 0x003F'FFFF   raw+hidden 0x00FF'FFFF >> 22 to get to 0x0000'0003
cfloat<6,2>                                   se'emff   fraction = '0000'0000'0000'0000'0000'0000'0000'0h10     3    sticky mask = 0x001F'FFFF   raw+hidden 0x00FF'FFFF >> 21 to get to 0x0000'0007
cfloat<7,2>                                  see'mfff   fraction = '0000'0000'0000'0000'0000'0000'0000'h110     4    sticky mask = 0x000F'FFFF   raw+hidden 0x00FF'FFFF >> 20 to get to 0x0000'000F
cfloat<8,2>                                'seem'ffff   fraction = '0000'0000'0000'0000'0000'0000'000h'1110     5    sticky mask = 0x0007'FFFF   raw+hidden 0x00FF'FFFF >> 19 to get to 0x0000'001F
cfloat<9,2>                               s'eemf'ffff   fraction = '0000'0000'0000'0000'0000'0000'00h1'1110     6    sticky mask = 0x0003'FFFF   raw+hidden 0x00FF'FFFF >> 18 to get to 0x0000'003F
cfloat<10,2>                             se'emff'ffff   fraction = '0000'0000'0000'0000'0000'0000'0h11'1110     7    sticky mask = 0x0001'FFFF   raw+hidden 0x00FF'FFFF >> 17 to get to 0x0000'007F
cfloat<11,2>                            see'mfff'ffff   fraction = '0000'0000'0000'0000'0000'0000'h111'1110     8    sticky mask = 0x0000'FFFF   raw+hidden 0x00FF'FFFF >> 16 to get to 0x0000'00FF
cfloat<12,2>                          'seem'ffff'ffff   fraction = '0000'0000'0000'0000'0000'000h'1111'1110     9    sticky mask = 0x0000'7FFF   raw+hidden 0x00FF'FFFF >> 15 to get to 0x0000'01FF
cfloat<13,2>                         s'eemf'ffff'ffff   fraction = '0000'0000'0000'0000'0000'00h1'1111'1110    10    sticky mask = 0x0000'3FFF   raw+hidden 0x00FF'FFFF >> 14 to get to 0x0000'03FF
cfloat<14,2>                        se'emff'ffff'ffff   fraction = '0000'0000'0000'0000'0000'0h11'1111'1110    11    sticky mask = 0x0000'1FFF   raw+hidden 0x00FF'FFFF >> 13 to get to 0x0000'07FF
cfloat<15,2>                       see'mfff'ffff'ffff   fraction = '0000'0000'0000'0000'0000'h111'1111'1110    12    sticky mask = 0x0000'0FFF   raw+hidden 0x00FF'FFFF >> 12 to get to 0x0000'0FFF
cfloat<16,2>                     'seem'ffff'ffff'ffff   fraction = '0000'0000'0000'0000'000h'1111'1111'1110    13    sticky mask = 0x0000'07FF   raw+hidden 0x00FF'FFFF >> 11 to get to 0x0000'1FFF
cfloat<17,2>                    s'eemf'ffff'ffff'ffff   fraction = '0000'0000'0000'0000'00h1'1111'1111'1110    14    sticky mask = 0x0000'03FF   raw+hidden 0x00FF'FFFF >> 10 to get to 0x0000'3FFF
cfloat<18,2>                   se'emff'ffff'ffff'ffff   fraction = '0000'0000'0000'0000'0h11'1111'1111'1110    15    sticky mask = 0x0000'01FF   raw+hidden 0x00FF'FFFF >>  9 to get to 0x0000'7FFF
cfloat<19,2>                  see'mfff'ffff'ffff'ffff   fraction = '0000'0000'0000'0000'h111'1111'1111'1110    16    sticky mask = 0x0000'00FF   raw+hidden 0x00FF'FFFF >>  8 to get to 0x0000'FFFF
cfloat<20,2>                'seem'ffff'ffff'ffff'ffff   fraction = '0000'0000'0000'000h'1111'1111'1111'1110    17    sticky mask = 0x0000'007F   raw+hidden 0x00FF'FFFF >>  7 to get to 0x0001'FFFF
cfloat<21,2>               s'eemf'ffff'ffff'ffff'ffff   fraction = '0000'0000'0000'00h1'1111'1111'1111'1110    18    sticky mask = 0x0000'003F   raw+hidden 0x00FF'FFFF >>  6 to get to 0x0003'FFFF
cfloat<22,2>              se'emff'ffff'ffff'ffff'ffff   fraction = '0000'0000'0000'0h11'1111'1111'1111'1110    19    sticky mask = 0x0000'001F   raw+hidden 0x00FF'FFFF >>  5 to get to 0x0007'FFFF
cfloat<23,2>             see'mfff'ffff'ffff'ffff'ffff   fraction = '0000'0000'0000'h111'1111'1111'1111'1110    20    sticky mask = 0x0000'000F   raw+hidden 0x00FF'FFFF >>  4 to get to 0x000F'FFFF
cfloat<24,2>           'seem'ffff'ffff'ffff'ffff'ffff   fraction = '0000'0000'000h'1111'1111'1111'1111'1110    21    sticky mask = 0x0000'0007   raw+hidden 0x00FF'FFFF >>  3 to get to 0x001F'FFFF
cfloat<25,2>          s'eemf'ffff'ffff'ffff'ffff'ffff   fraction = '0000'0000'00h1'1111'1111'1111'1111'1110    22    sticky mask = 0x0000'0003   raw+hidden 0x00FF'FFFF >>  2 to get to 0x003F'FFFF
cfloat<26,2>         se'emff'ffff'ffff'ffff'ffff'ffff   fraction = '0000'0000'0h11'1111'1111'1111'1111'1110    23    sticky mask = 0x0000'0001   raw+hidden 0x00FF'FFFF >>  1 to get to 0x007F'FFFF
cfloat<27,2>        see'mfff'ffff'ffff'ffff'ffff'ffff   fraction = '0000'0000'h111'1111'1111'1111'1111'1110    24    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >>  0 to get to 0x00FF'FFFF
cfloat<28,2>      'seem'ffff'ffff'ffff'ffff'ffff'ffff   fraction = '0000'000h'1111'1111'1111'1111'1111'1110    25    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -1 to get to 0x01FF'FFFF
cfloat<29,2>     s'eemf'ffff'ffff'ffff'ffff'ffff'ffff   fraction = '0000'00h1'1111'1111'1111'1111'1111'1110    26    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -2 to get to 0x03FF'FFFF
cfloat<30,2>    se'emff'ffff'ffff'ffff'ffff'ffff'ffff   fraction = '0000'0h11'1111'1111'1111'1111'1111'1110    27    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -3 to get to 0x07FF'FFFF
cfloat<31,2>   see'mfff'ffff'ffff'ffff'ffff'ffff'ffff   fraction = '0000'h111'1111'1111'1111'1111'1111'1110    28    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -4 to get to 0x0FFF'FFFF
cfloat<32,2> 'seem'ffff'ffff'ffff'ffff'ffff'ffff'ffff   fraction = '000h'1111'1111'1111'1111'1111'1111'1110    29    sticky mask = 0x0000'0000   raw+hidden 0x00FF'FFFF >> -5 to get to 0x1FFF'FFFF

*/

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "takum<> assignment";
	std::string test_tag    = "assignment";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using Real = sw::universal::takum<16, uint16_t>;

	bool bConversionTest = true;
	if (bConversionTest) {
		float test = 0.0625f;
		std::cout << to_binary(test) << " : " << test << std::endl;
		ConversionTest<takum<12>>(test);
		ConversionTest<takum<14>>(test);
		ConversionTest<takum<16>>(test);
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else //!MANUAL_TESTING
	constexpr bool bVerbose        = false;
	constexpr bool hasSubnormals   = true;
	constexpr bool noSubnormals    = false;
	constexpr bool hasSupernormals = true;
	constexpr bool noSupernormals  = false;
	constexpr bool notSaturating   = false;

#if REGRESSION_LEVEL_1


#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
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
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
