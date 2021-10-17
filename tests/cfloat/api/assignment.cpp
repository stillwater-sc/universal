// assignment.cpp: functional tests for assignments of native types to cfloats
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
// Configure the cfloat template environment
// first: enable general or specialized configurations
#define CFLOAT_FAST_SPECIALIZATION
// second: enable/disable cfloat arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
// enabling tracing
#define TRACE_CONVERSION 0
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>

// print the constexpr values of the cfloat class
template<size_t nbits, size_t es, typename bt>
void configuration() {
	sw::universal::cfloat<nbits, es, bt> a;
	a.debug();
}

// free function that does the same as the private copyBits function of the cfloat class
template<typename ArgumentBlockType, typename BlockType>
void copyBits(ArgumentBlockType v, BlockType _block[16]) {
	size_t bitsInBlock = sizeof(BlockType) * 8;
	size_t nrBlocks = 16;
	int blocksRequired = (8 * sizeof(v)) / bitsInBlock;
	int maxBlockNr = (blocksRequired < nrBlocks ? blocksRequired : nrBlocks);
	BlockType b{ 0 }; b = ~b;
	ArgumentBlockType mask = ArgumentBlockType(b);
	size_t shift = 0;
	for (int i = 0; i < maxBlockNr; ++i) {
		_block[i] = (mask & v) >> shift;
		mask <<= bitsInBlock;
		shift += bitsInBlock;
	}
}

// verify the subnormals of an cfloat configuration
template<size_t nbits, size_t es, typename bt = uint8_t, typename NativeFloatingPointType = double>
int VerifySubnormalReverseSampling(bool bReportIndividualTestCases = false, bool verbose = false) {
	// subnormals exist in the exponent = 0 range
	constexpr size_t fbits = nbits - 1ull - es - 1ull;
	constexpr size_t NR_SAMPLES = (1ull << (fbits + 1ull)); //  the first segment of fbits+ubit are subnormals
	int nrOfFailedTestCases = 0;
	using Real = sw::universal::cfloat<nbits, es, bt>;
	Real ref{ 0 }; Real result{ 0 };
	for (size_t i = 0; i < NR_SAMPLES; i += 2) {
		ref.setbits(i);
		NativeFloatingPointType input = NativeFloatingPointType(ref);
		result = input;
		if (result != ref) {
			nrOfFailedTestCases++;
			//			std::cout << "------->  " << i << " " << sw::universal::to_binary(input) << " " << sw::universal::to_binary(result) << std::endl;
			if (bReportIndividualTestCases) ReportAssignmentError("FAIL", "=", input, result, ref);
		}
		else {
			if (verbose && bReportIndividualTestCases) ReportAssignmentSuccess("PASS", "=", input, result, ref);
		}
	}
	return nrOfFailedTestCases;
}

template<size_t nbits, size_t es, typename bt = uint8_t, typename NativeFloatingPointType = double>
int VerifyReverseSampling(bool bReportIndividualTestCases = false, bool verbose = false) {
	constexpr size_t NR_SAMPLES = (1ull << nbits);
	int nrOfFailedTestCases = 0;
	using Real = sw::universal::cfloat<nbits, es, bt>;
	Real ref{ 0 }; Real result{ 0 };
	std::cout << std::setw(40) << typeid(result).name() << "   : ";
	for (size_t i = 0; i < NR_SAMPLES; i += 2) {
		ref.setbits(i);
		NativeFloatingPointType input = NativeFloatingPointType(ref);
		result = input;
		// special cases do not have consistent compiler behavior
		if (ref.iszero()) {
			// optimization compilers may destroy the sign on -0
			if (input != 0) {
				nrOfFailedTestCases++;
				if (bReportIndividualTestCases) ReportAssignmentError("FAIL", "=", input, result, ref);
			}
			else {
				if (verbose && bReportIndividualTestCases) ReportAssignmentSuccess("PASS", "=", input, result, ref);
			}
		}
		else if (ref.isnan()) {
			// optimization compilers may change signalling NaNs to quiet NaNs
			if (std::fpclassify(input) != FP_NAN) {
				nrOfFailedTestCases++;
				if (bReportIndividualTestCases) ReportAssignmentError("FAIL", "=", input, result, ref);
			}
			else {
				if (verbose && bReportIndividualTestCases) ReportAssignmentSuccess("PASS", "=", input, result, ref);
			}
		}
		else if (ref.isinf()) {
			// optimization compilers may destroy the sign on -0
			if (std::fpclassify(input) != FP_INFINITE) {
				nrOfFailedTestCases++;
				if (bReportIndividualTestCases) ReportAssignmentError("FAIL", "=", input, result, ref);
			}
			else {
				if (verbose && bReportIndividualTestCases) ReportAssignmentSuccess("PASS", "=", input, result, ref);
			}
		}
		else if (result != ref) {
                          
			nrOfFailedTestCases++;
//			std::cout << "------->  " << i << " " << sw::universal::to_binary(input) << " " << sw::universal::to_binary(result) << std::endl;
			if (bReportIndividualTestCases) ReportAssignmentError("FAIL", "=", input, result, ref);
		}
		else {
			if (verbose && bReportIndividualTestCases) ReportAssignmentSuccess("PASS", "=", input, result, ref);
		}
	}
	return nrOfFailedTestCases;
}

template<typename TestType, typename NativeFloatingPointType>
int VerifySpecialCases(const std::string& tag, bool bReportIndividualTestCases = false) {
	using namespace sw::universal;
	int nrOfFailedTests{ 0 };
	TestType a{ 0 };
	NativeFloatingPointType fa{ 0 };

	std::cout << "Verify special cases for " << typeid(NativeFloatingPointType).name() << '\n';
	std::cout << tag << '\n';

	// test sNaN
	a.setnan(NAN_TYPE_SIGNALLING);
	fa = NativeFloatingPointType(a);
	std::cout << to_binary(fa) << " " << fa << " : ";
	a = fa;
	std::cout << color_print(a) << " " << pretty_print(a) << " " << a << '\n';
	if (!a.isnan(NAN_TYPE_SIGNALLING)) ++nrOfFailedTests;
	if (bReportIndividualTestCases && a.isnan(NAN_TYPE_SIGNALLING)) std::cout << "PASS snan\n"; else std::cout << "FAIL snan\n";

	// test qNaN
	a.setnan(NAN_TYPE_QUIET);
	fa = NativeFloatingPointType(a);
	std::cout << to_binary(fa) << " " << fa << " : ";
	a = fa;
	std::cout << color_print(a) << " " << pretty_print(a) << " " << a << '\n';
	if (!a.isnan(NAN_TYPE_QUIET)) ++nrOfFailedTests;
	if (bReportIndividualTestCases && a.isnan(NAN_TYPE_QUIET)) std::cout << "PASS qnan\n"; else std::cout << "FAIL qnan\n";

	// test +inf
	a.setinf(false); // +inf
	fa = NativeFloatingPointType(a);
	std::cout << to_binary(fa) << " " << fa << " : ";
	a = fa;
	std::cout << color_print(a) << " " << pretty_print(a) << " " << a << '\n';
	if (!a.isinf(INF_TYPE_POSITIVE)) ++nrOfFailedTests;
	if (bReportIndividualTestCases && a.isinf(INF_TYPE_POSITIVE)) std::cout << "PASS +inf\n"; else std::cout << "FAIL +inf\n";

	// test -inf
	a.setinf(true); // -inf
	fa = NativeFloatingPointType(a);
	std::cout << to_binary(fa) << " " << fa << " : ";
	a = fa;
	std::cout << color_print(a) << " " << pretty_print(a) << " " << a << '\n';
	if (!a.isinf(INF_TYPE_NEGATIVE)) ++nrOfFailedTests;
	if (bReportIndividualTestCases && a.isinf(INF_TYPE_NEGATIVE)) std::cout << "PASS -inf\n"; else std::cout << "FAIL -inf\n";

	std::cout << "Representations of zero in " << typeid(NativeFloatingPointType).name() << '\n';
	NativeFloatingPointType zero;
	zero = 0.0;
	std::cout << "+0.0 = " << to_binary(+zero) << " " << zero << '\n';
	std::cout << "-0.0 = " << to_binary(-zero) << " " << -zero << '\n';

	// test 0.0
	std::cout << "Test positive 0.0\n";
	a.setbits(0x00);
	std::cout << "conversion(a)= " << NativeFloatingPointType(a) << '\n';
	fa = NativeFloatingPointType(a);
	std::cout << "reference  a = " << a << " " << to_binary(fa) << " " << fa << " : ";
	a = fa;
	std::cout << "assignment a = " << color_print(a) << " " << pretty_print(a) << " " << a << '\n';
	if (!a.iszero()) ++nrOfFailedTests;
	if (bReportIndividualTestCases && a.iszero()) std::cout << "PASS +0 == iszero()\n"; else std::cout << "FAIL +0 != iszero()\n";

	// Testing problem: the optimizer might destroy the sign of a copy of a -0.0
	// test -0.0
	std::cout << "Test negative 0.0\n";
	a.setbits(0x80);
	std::cout << "conversion(a)= " << double(a) << '\n';
	fa = NativeFloatingPointType(a);
	std::cout << "reference  a = " << a << " " << to_binary(fa) << " " << fa << " : ";
	a = fa;
	std::cout << "assignment a = " << color_print(a) << " " << pretty_print(a) << " " << a << '\n';
	if (!a.iszero()) ++nrOfFailedTests;
	if (bReportIndividualTestCases && a.iszero()) std::cout << "PASS -0 == iszero()\n"; else std::cout << "FAIL -0 != iszero()\n";

	return nrOfFailedTests;
}

#ifdef EXPERIMENT
void projectToFloat() {
	uint32_t a = 0x3F55'5555;
	float f = *(float*)(&a);
	std::cout << sw::universal::to_binary(f) << " : " << f << std::endl;
	float f2{ 0.8333333f };
	std::cout << sw::universal::to_binary(f2) << " : " << f2 << std::endl;
}
#endif

template<typename TestType, typename NativeFloatingPointType>
void ConversionTest(NativeFloatingPointType& value) {
	using namespace sw::universal;
	std::cout << color_print(value) << " " << value << '\n';
	TestType a = value;
	std::cout << color_print(a) << " " << pretty_print(a) << " " << a << '\n';
}

template<size_t es, typename NativeFloatingPointType>
int TestSingleBlockRepresentations(const std::string& op, bool bReportIndividualTestCases, bool bVerbose) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;

	// 1 block representations

	std::string testcase;

	if constexpr (es < 2) {
		std::stringstream ss;
		ss << "cfloat<4, " << es << ", uint8_t> ";
		testcase = ss.str();
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< 4, es, uint8_t, NativeFloatingPointType>(bReportIndividualTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 3) {
		std::stringstream ss;
		ss << "cfloat<5, " << es << ", uint8_t> ";
		testcase = ss.str();
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< 5, es, uint8_t, NativeFloatingPointType>(bReportIndividualTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 4) {
		std::stringstream ss;
		ss << "cfloat<6, " << es << ", uint8_t> ";
		testcase = ss.str();
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< 6, es, uint8_t, NativeFloatingPointType>(bReportIndividualTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 5) {
		std::stringstream ss;
		ss << "cfloat<7, " << es << ", uint8_t> ";
		testcase = ss.str();
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< 7, es, uint8_t, NativeFloatingPointType>(bReportIndividualTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 6) {
		std::stringstream ss;
		ss << "cfloat<8, " << es << ", uint8_t> ";
		testcase = ss.str();
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< 8, es, uint8_t, NativeFloatingPointType>(bReportIndividualTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 7) {
		std::stringstream ss;
		ss << "cfloat<9, " << es << ", uint16_t> ";
		testcase = ss.str();
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< 9, es, uint16_t, NativeFloatingPointType>(bReportIndividualTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 8) {
		std::stringstream ss;
		ss << "cfloat<10, " << es << ", uint16_t> ";
		testcase = ss.str();
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<10, es, uint16_t, NativeFloatingPointType>(bReportIndividualTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 10) {
		std::stringstream ss;
		ss << "cfloat<12, " << es << ", uint16_t> ";
		testcase = ss.str();
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<12, es, uint16_t, NativeFloatingPointType>(bReportIndividualTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 12) {
		std::stringstream ss;
		ss << "cfloat<14, " << es << ", uint16_t> ";
		testcase = ss.str();
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<14, es, uint16_t, NativeFloatingPointType>(bReportIndividualTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 14) {
		std::stringstream ss;
		ss << "cfloat<16, " << es << ", uint16_t> ";
		testcase = ss.str();
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<16, es, uint16_t, NativeFloatingPointType>(bReportIndividualTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 18) {
		std::stringstream ss;
		ss << "cfloat<20, " << es << ", uint32_t> ";
		testcase = ss.str();
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<20, es, uint32_t, NativeFloatingPointType>(bReportIndividualTestCases, bVerbose), testcase, op);
	}

	return nrOfFailedTestCases;
}

template<size_t es, typename NativeFloatingPointType>
int TestDoubleBlockRepresentations(const std::string& op, bool bReportIndividualTestCases, bool bVerbose) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;

	// 2 block representations

	std::string testcase;

	if constexpr (es < 7) {
		std::stringstream ss;
		ss << "cfloat<9, " << es << ", uint8_t> ";
		testcase = ss.str();
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< 9, es, uint8_t, NativeFloatingPointType>(bReportIndividualTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 8) {
		std::stringstream ss;
		ss << "cfloat<10, " << es << ", uint8_t> ";
		testcase = ss.str();
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<10, es, uint8_t, NativeFloatingPointType>(bReportIndividualTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 10) {
		std::stringstream ss;
		ss << "cfloat<12, " << es << ", uint8_t> ";
		testcase = ss.str();
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<12, es, uint8_t, NativeFloatingPointType>(bReportIndividualTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 12) {
		std::stringstream ss;
		ss << "cfloat<14, " << es << ", uint8_t> ";
		testcase = ss.str();
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<14, es, uint8_t, NativeFloatingPointType>(bReportIndividualTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 14) {
		std::stringstream ss;
		ss << "cfloat<16, " << es << ", uint8_t> ";
		testcase = ss.str();
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<16, es, uint8_t, NativeFloatingPointType>(bReportIndividualTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 18) {
		std::stringstream ss;
		ss << "cfloat<20, " << es << ", uint16_t> ";
		testcase = ss.str();
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<20, es, uint16_t, NativeFloatingPointType>(bReportIndividualTestCases, bVerbose), testcase, op);
	}

	return nrOfFailedTestCases;
}

template<size_t es, typename NativeFloatingPointType>
int TestTripleBlockRepresentations(const std::string& op, bool bReportIndividualTestCases, bool bVerbose) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;

	// 3 block representations

	std::string testcase;

	if constexpr (es < 18) {
		std::stringstream ss;
		ss << "cfloat<20, " << es << ", uint8_t> ";
		testcase = ss.str();
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<20, es, uint8_t, NativeFloatingPointType>(bReportIndividualTestCases, bVerbose), testcase, op);
	}

	return nrOfFailedTestCases;
}

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

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

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	print_cmd_line(argc, argv);

	std::string tag = "cfloat assignment: ";

#if MANUAL_TESTING

	using Real = sw::universal::cfloat<8, 2>;

	bool bConversionTest = true;
	if (bConversionTest) {
		float test = 0.0625f;
		std::cout << to_binary(test) << " : " << test << std::endl;
		ConversionTest<cfloat<8, 1>>(test);
		ConversionTest<cfloat<8, 2>>(test);
		ConversionTest<cfloat<8, 3>>(test);
		ConversionTest<cfloat<8, 4>>(test);
	}

	{
		sw::universal::cfloat<9, 1> a = -0.03125f;
		std::cout << color_print(a) << " : " << a << std::endl;
	}

	nrOfFailedTestCases += ReportTestResult(VerifySubnormalReverseSampling<9, 1, uint8_t, float>(tag, true, false), "cfloat<9,1, uint8_t>", "=float");
	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<9, 1, uint8_t, float>(tag, true, false), "cfloat<9,1, uint8_t>", "=float");

	{
		float f;
		sw::universal::cfloat<9, 1> a;
		a.setbits(0x1FF); f = float(a);
		std::cout << "signalling NaN : " << color_print(a) << " : " << a << " : " << f << '\n';
		a.setbits(0x0FF); f = float(a);
		std::cout << "     quiet NaN : " << color_print(a) << " : " << a << " : " << f << '\n';
		a.setbits(0x1FE); f = float(a);
		std::cout << "     -INFINITY : " << color_print(a) << " : " << a << " : " << f << '\n';
		a.setbits(0x0FE); f = float(a);
		std::cout << "     +INFINITY : " << color_print(a) << " : " << a << " : " << f << '\n';
	}
	
	nrOfFailedTestCases += ReportTestResult(VerifySubnormalReverseSampling<5, 2, uint8_t, float>(tag, true, true), "cfloat<5,2, uint8_t>", "=float");
	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<5, 2, uint8_t, float>(tag, true, true), "cfloat<5,2, uint8_t>", "=float");

	{
		float f;
		sw::universal::cfloat<5, 2> a;
		a.setbits(0x18);
		std::cout << color_print(a) << " : " << a << '\n';
		f = float(a);
		a = f;
		std::cout << "source -2 : " << color_print(a) << " : " << a << " : " << f << '\n';

	}

#if STRESS_TESTING

	// manual exhaustive test

#endif

	std::cout << "Number of failed test cases : " << nrOfFailedTestCases << std::endl;
	nrOfFailedTestCases = 0; // disregard any test failures in manual testing mode

#else //!MANUAL_TESTING
	std::cout << "cfloat assignment validation\n";

	bool bVerbose = false;

	std::cout << "Special cases: zero, inf, nan\n";
	using Real = sw::universal::cfloat<8, 2>;
	nrOfFailedTestCases += VerifySpecialCases<Real, float>("float->cfloat special cases", bReportIndividualTestCases);
	nrOfFailedTestCases += VerifySpecialCases<Real, double>("double->cfloat special cases", bReportIndividualTestCases);
#if LONG_DOUBLE_SUPPORT
	nrOfFailedTestCases += VerifySpecialCases<Real, long double>("long double->cfloat special cases", bReportIndividualTestCases);
#endif

	std::cout << "Single block representations\n--------------------------------------------- es = 1 encodings\n";
	nrOfFailedTestCases += TestSingleBlockRepresentations<1, float>("=float", bReportIndividualTestCases, bVerbose);
	nrOfFailedTestCases += TestSingleBlockRepresentations<1, double>("=double", bReportIndividualTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 2 encodings\n";
	nrOfFailedTestCases += TestSingleBlockRepresentations<2, float>("=float", bReportIndividualTestCases, bVerbose);
	nrOfFailedTestCases += TestSingleBlockRepresentations<2, double>("=double", bReportIndividualTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 3 encodings\n";
	nrOfFailedTestCases += TestSingleBlockRepresentations<3, float>("=float", bReportIndividualTestCases, bVerbose);
	nrOfFailedTestCases += TestSingleBlockRepresentations<3, double>("=double", bReportIndividualTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 4 encodings\n";
	nrOfFailedTestCases += TestSingleBlockRepresentations<4, float>("=float", bReportIndividualTestCases, bVerbose);
	nrOfFailedTestCases += TestSingleBlockRepresentations<4, double>("=double", bReportIndividualTestCases, bVerbose);

	std::cout << "Double block representations\n--------------------------------------------- es = 1 encodings\n";
	nrOfFailedTestCases += TestDoubleBlockRepresentations<1, float>("=float", bReportIndividualTestCases, bVerbose);
	nrOfFailedTestCases += TestDoubleBlockRepresentations<1, double>("=double", bReportIndividualTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 2 encodings\n";
	nrOfFailedTestCases += TestDoubleBlockRepresentations<2, float>("=float", bReportIndividualTestCases, bVerbose);
	nrOfFailedTestCases += TestDoubleBlockRepresentations<2, double>("=double", bReportIndividualTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 3 encodings\n";
	nrOfFailedTestCases += TestDoubleBlockRepresentations<3, float>("=float", bReportIndividualTestCases, bVerbose);
	nrOfFailedTestCases += TestDoubleBlockRepresentations<3, double>("=double", bReportIndividualTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 4 encodings\n";
	nrOfFailedTestCases += TestDoubleBlockRepresentations<4, float>("=float", bReportIndividualTestCases, bVerbose);
	nrOfFailedTestCases += TestDoubleBlockRepresentations<4, double>("=double", bReportIndividualTestCases, bVerbose);

	std::cout << "Triple block representations\n--------------------------------------------- es = 1 encodings\n";
	nrOfFailedTestCases += TestTripleBlockRepresentations<1, float>("=float", bReportIndividualTestCases, bVerbose);
	nrOfFailedTestCases += TestTripleBlockRepresentations<1, double>("=double", bReportIndividualTestCases, bVerbose);


	/*
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::cfloat<4, 1,  uint8_t>, float>(bReportIndividualTestCases), tag, "cfloat<4,1,uint8_t>");

	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::cfloat<6, 1,  uint8_t>, float>(bReportIndividualTestCases), tag, "cfloat<6,1,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::cfloat<6, 2,  uint8_t>, float>(bReportIndividualTestCases), tag, "cfloat<6,2,uint8_t>");

	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::cfloat<8, 1,  uint8_t>, float>(bReportIndividualTestCases), tag, "cfloat<8,1,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::cfloat<8, 2,  uint8_t>, float>(bReportIndividualTestCases), tag, "cfloat<8,2,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::cfloat<8, 3,  uint8_t>, float>(bReportIndividualTestCases), tag, "cfloat<8,3,uint8_t>");

	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::cfloat<10, 1,  uint8_t>, float>(bReportIndividualTestCases), tag, "cfloat<10,1,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::cfloat<10, 2,  uint8_t>, float>(bReportIndividualTestCases), tag, "cfloat<10,2,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(ValidateAssignment< sw::universal::cfloat<10, 3,  uint8_t>, float>(bReportIndividualTestCases), tag, "cfloat<10,3,uint8_t>");
	*/

	nrOfFailedTestCases = 0;
#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_arithmetic_exception& err) {
	std::cerr << "Uncaught cfloat arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_internal_exception& err) {
	std::cerr << "Uncaught cfloat internal exception: " << err.what() << std::endl;
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
