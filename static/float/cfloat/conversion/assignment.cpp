// assignment.cpp: functional tests for assignments of native types to cfloats
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
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
#include <universal/utility/architecture.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>

// print the constexpr values of the cfloat clastr
template<size_t nbits, size_t es, typename bt>
void configuration() {
	sw::universal::cfloat<nbits, es, bt> a;
	a.debug();
}

// free function that does the same as the private copyBits function of the cfloat clastr
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
template<typename CfloatConfiguration, typename NativeFloatingPointType = double>
int VerifySubnormalReverseSampling(bool reportTestCases = false, bool verbose = false) {
	constexpr size_t nbits = CfloatConfiguration::nbits;
	constexpr size_t es = CfloatConfiguration::es;
	using bt = typename CfloatConfiguration::BlockType;
	constexpr bool hasSubnormals = CfloatConfiguration::hasSubnormals;
	constexpr bool hasMaxExpValues = CfloatConfiguration::hasMaxExpValues;
	constexpr bool isSaturating = CfloatConfiguration::isSaturating;
	using Real = sw::universal::cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;

	// subnormals exist in the exponent = 0 range
	constexpr size_t fbits = nbits - 1ull - es - 1ull;
	constexpr size_t NR_SAMPLES = (1ull << (fbits + 1ull)); //  the first segment of fbits+ubit are subnormals
	int nrOfFailedTestCases = 0;
	Real ref{ 0 };
	for (size_t i = 0; i < NR_SAMPLES; ++i) {
		ref.setbits(i);
		NativeFloatingPointType input = NativeFloatingPointType(ref);
		Real result = input;
		if (result != ref) {
			if (ref.iszero() && result.iszero()) continue; // optimization may destroy sign on zero
			nrOfFailedTestCases++;
			//			std::cout << "------->  " << i << " " << sw::universal::to_binary(input) << " " << sw::universal::to_binary(result) << std::endl;
			if (reportTestCases && nrOfFailedTestCases < 5) ReportAssignmentError("FAIL", "=", input, result, ref);
		}
		else {
			if (verbose && reportTestCases) ReportAssignmentSuccess("PASS", "=", input, result, ref);
		}
	}
	return nrOfFailedTestCases;
}

// verify that conversion is closed and consistent
template<typename CfloatConfiguration, typename NativeFloatingPointType = double>
int VerifyReverseSampling(bool reportTestCases = false, bool verbose = false) {
	constexpr size_t nbits = CfloatConfiguration::nbits;
	constexpr size_t es    = CfloatConfiguration::es;
	using bt = typename CfloatConfiguration::BlockType;
	constexpr bool hasSubnormals   = CfloatConfiguration::hasSubnormals;
	constexpr bool hasMaxExpValues = CfloatConfiguration::hasMaxExpValues;
	constexpr bool isSaturating    = CfloatConfiguration::isSaturating;
	using Real = sw::universal::cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>;

	constexpr size_t NR_SAMPLES = (1ull << nbits);
	int nrOfFailedTestCases = 0;
	Real ref{ 0 }; Real result{ 0 };
	std::cout << std::setw(40) << typeid(result).name() << "   : ";
	for (size_t i = 0; i < NR_SAMPLES; ++i) {
		ref.setbits(i);
		if constexpr (!hasSubnormals) if (ref.isdenormal()) continue; // ignore the subnormal encodings
		NativeFloatingPointType input = NativeFloatingPointType(ref);
		result = input;
		// special cases do not have consistent compiler behavior
		if (ref.iszero()) {
			// optimization may destroy the sign on -0
			if (input != 0) {
				nrOfFailedTestCases++;
				if (reportTestCases && nrOfFailedTestCases < 5) ReportAssignmentError("FAIL", "=", input, result, ref);
			}
			else {
				if (verbose && reportTestCases) ReportAssignmentSuccess("PASS", "=", input, result, ref);
			}
		}
		else if (ref.isnan()) {
			// optimization may change signalling NaNs to quiet NaNs
			if (std::fpclassify(input) != FP_NAN) {
				nrOfFailedTestCases++;
				if (reportTestCases && nrOfFailedTestCases < 5) ReportAssignmentError("FAIL", "=", input, result, ref);
			}
			else {
				if (verbose && reportTestCases) ReportAssignmentSuccess("PASS", "=", input, result, ref);
			}
		}
		else if (ref.isinf()) {
			// optimization may destroy the sign on -0
			if (std::fpclassify(input) != FP_INFINITE) {
				nrOfFailedTestCases++;
				if (reportTestCases && nrOfFailedTestCases < 5) ReportAssignmentError("FAIL", "=", input, result, ref);
			}
			else {
				if (verbose && reportTestCases) ReportAssignmentSuccess("PASS", "=", input, result, ref);
			}
		}
		else if (result != ref) {
                          
			nrOfFailedTestCases++;
//			std::cout << "------->  " << i << " " << sw::universal::to_binary(input) << " " << sw::universal::to_binary(result) << std::endl;
			if (reportTestCases && nrOfFailedTestCases < 5) ReportAssignmentError("FAIL", "=", input, result, ref);
		}
		else {
			if (verbose && reportTestCases) ReportAssignmentSuccess("PASS", "=", input, result, ref);
		}
	}
	return nrOfFailedTestCases;
}

template<typename TestType, typename NativeFloatingPointType>
int VerifySpecialCases(const std::string& tag, bool reportTestCases = false) {
	using namespace sw::universal;
	int nrOfFailedTests{ 0 };
	TestType a{ 0 };
	NativeFloatingPointType fa{ 0 };

	std::cout << "Verify special cases for " << typeid(NativeFloatingPointType).name() << '\n';
	std::cout << tag << '\n';

	// test sNaN
	// NOTE: cfloat encodes sNaN as the all-ones pattern (sign=1) and
	// qNaN as all-ones-except-sign (sign=0).  When sNaN is converted
	// to a native float/double, architectures such as RISC-V, ARM,
	// and POWER will quiet the sNaN (clear the signaling bit) and may
	// also canonicalise the NaN payload.  The resulting native qNaN
	// may then convert back to a cfloat encoding that no longer matches
	// the original sNaN — or may even lose the NaN classification
	// entirely for small cfloat formats.  We therefore only test the
	// sNaN round-trip on platforms where it is known to survive.
#if UNIVERSAL_SNAN_ROUND_TRIPS_NATIVE_FP
	a.setnan(NAN_TYPE_SIGNALLING);
	fa = NativeFloatingPointType(a);
	a = fa;
	if (!a.isnan(NAN_TYPE_SIGNALLING)) {
		++nrOfFailedTests;
		std::cout << type_tag(fa) << " : " << to_binary(fa) << " " << fa << " : ";
		std::cout << color_print(a) << " " << pretty_print(a) << " " << a << '\n';
		if (reportTestCases) std::cout << "FAIL snan\n";
	}
#endif

	// test qNaN
	a.setnan(NAN_TYPE_QUIET);
	fa = NativeFloatingPointType(a);
	a = fa;
	if (!a.isnan(NAN_TYPE_QUIET)) {
		++nrOfFailedTests;
		std::cout << type_tag(fa) << " : " << to_binary(fa) << " " << fa << " : ";
		std::cout << color_print(a) << " " << pretty_print(a) << " " << a << '\n';
		if (reportTestCases) std::cout << "FAIL qnan\n";
	}

	// test +inf
	a.setinf(false); // +inf
	fa = NativeFloatingPointType(a);
	a = fa;
	if (!a.isinf(INF_TYPE_POSITIVE)) {
		++nrOfFailedTests;
		std::cout << type_tag(fa) << " : " << to_binary(fa) << " " << fa << " : ";
		std::cout << color_print(a) << " " << pretty_print(a) << " " << a << '\n';
		if (reportTestCases) std::cout << "FAIL +inf\n";
	}

	// test -inf
	a.setinf(true); // -inf
	fa = NativeFloatingPointType(a);
	a = fa;
	if (!a.isinf(INF_TYPE_NEGATIVE)) {
		++nrOfFailedTests;
		std::cout << type_tag(fa) << " : " << to_binary(fa) << " " << fa << " : ";
		std::cout << color_print(a) << " " << pretty_print(a) << " " << a << '\n';
		if (reportTestCases) std::cout << "FAIL -inf\n";
	}

#ifdef INCONSISTENT
	this fails on MSVC because INFINITY is defined as a macro that expands to a constant expression 
	((float)1.0e300) which does not yield infinity when converted to double

		double : 0b0.11111100011.0111111001000011110010001000000000000111010110011100 1e+300 : 0111111000110111111001000011110010001000000000000111010110011100 0 : 11111100011 : 0111111001000011110010001000000000000111010110011100 1e+300
		FAIL + inf
		double : 0b1.11111100011.0111111001000011110010001000000000000111010110011100 - 1e+300 : 1111111000110111111001000011110010001000000000000111010110011100 1 : 11111100011 : 0111111001000011110010001000000000000111010110011100 - 1e+300
		FAIL - inf

	fa = INFINITY;
	a = fa;
	if (reportTestCases) {
		std::cout << "Test +inf\n";
		std::cout << to_binary(fa) << " " << fa << '\n';
		std::cout << to_binary(a) << " " << a << '\n';
	}
	if (!a.isinf(INF_TYPE_POSITIVE)) {
		++nrOfFailedTests;
		std::cout << type_tag(fa) << " : " << to_binary(fa) << " " << fa << " : ";
		std::cout << color_print(a) << " " << pretty_print(a) << " " << a << '\n';
		if (reportTestCases) std::cout << "FAIL +inf\n";
	}
	fa = -INFINITY;
	a = fa;
	if (!a.isinf(INF_TYPE_NEGATIVE)) {
		++nrOfFailedTests;
		std::cout << type_tag(fa) << " : " << to_binary(fa) << " " << fa << " : ";
		std::cout << color_print(a) << " " << pretty_print(a) << " " << a << '\n';
		if (reportTestCases) std::cout << "FAIL -inf\n";
	}
#endif

	std::cout << "Representations of zero in " << typeid(NativeFloatingPointType).name() << '\n';
	NativeFloatingPointType zero;
	zero = 0.0;
//	std::cout << "+0.0 = " << to_binary(+zero) << " " << zero << '\n';
//	std::cout << "-0.0 = " << to_binary(-zero) << " " << -zero << '\n';

	// test 0.0
	std::cout << "Test positive 0.0\n";
	fa = zero;
	a = fa;
	if (!a.iszero()) {
		++nrOfFailedTests;
		std::cout << "reference  a = " << a << " " << to_binary(fa) << " " << fa << " : ";
		std::cout << "assignment a = " << color_print(a) << " " << pretty_print(a) << " " << a << '\n';
		if (reportTestCases) std::cout << "FAIL +0 != iszero()\n";
	}

	// Testing problem: the optimizer might destroy the sign of a copy of a -0.0
	// test -0.0
	std::cout << "Test negative 0.0\n";
	fa = -zero;
	a = fa;
	if (!a.iszero()) {
		++nrOfFailedTests;
		std::cout << "reference  a = " << a << " " << to_binary(fa) << " " << fa << " : ";
		std::cout << "assignment a = " << color_print(a) << " " << pretty_print(a) << " " << a << '\n';
		if (reportTestCases) std::cout << "FAIL -0 != iszero()\n";
	}

	return nrOfFailedTests;
}

template<typename CfloatConfiguration>
int TestSpecialCases(bool reportTestCases) {
	int nrOfFailedTestCases = 0;
	nrOfFailedTestCases += VerifySpecialCases<CfloatConfiguration, float>("float->cfloat special cases", reportTestCases);
	nrOfFailedTestCases += VerifySpecialCases<CfloatConfiguration, double>("double->cfloat special cases", true);
#if LONG_DOUBLE_SUPPORT
	//nrOfFailedTestCases += VerifySpecialCases<CfloatConfiguration, long double>("long double->cfloat special cases", reportTestCases);
	// TODO: ignore failures for the moment
	VerifySpecialCases<CfloatConfiguration, long double>("long double->cfloat special cases", reportTestCases);
#endif
	return nrOfFailedTestCases;
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

template<size_t es, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating, typename NativeFloatingPointType>
int TestSingleBlockRepresentations(const std::string& op, bool reportTestCases, bool bVerbose) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;

	// 1 block representations

	std::string testcase;

	if constexpr (es < 2) {
		using CfloatConfiguration = cfloat<4, es, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< CfloatConfiguration, NativeFloatingPointType>(reportTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 3) {
		using CfloatConfiguration = cfloat<5, es, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< CfloatConfiguration, NativeFloatingPointType>(reportTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 4) {
		using CfloatConfiguration = cfloat<6, es, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< CfloatConfiguration, NativeFloatingPointType>(reportTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 5) {
		using CfloatConfiguration = cfloat<7, es, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< CfloatConfiguration, NativeFloatingPointType>(reportTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 6) {
		using CfloatConfiguration = cfloat<8, es, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< CfloatConfiguration, NativeFloatingPointType>(reportTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 7) {
		using CfloatConfiguration = cfloat<9, es, uint16_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<CfloatConfiguration, NativeFloatingPointType>(reportTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 8) {
		using CfloatConfiguration = cfloat<10, es, uint16_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<CfloatConfiguration, NativeFloatingPointType>(reportTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 10) {
		using CfloatConfiguration = cfloat<12, es, uint16_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<CfloatConfiguration, NativeFloatingPointType>(reportTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 12) {
		using CfloatConfiguration = cfloat<14, es, uint16_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<CfloatConfiguration, NativeFloatingPointType>(reportTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 14) {
		using CfloatConfiguration = cfloat<16, es, uint16_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<CfloatConfiguration, NativeFloatingPointType>(reportTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 18) {
		using CfloatConfiguration = cfloat<20, es, uint32_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<CfloatConfiguration, NativeFloatingPointType>(reportTestCases, bVerbose), testcase, op);
	}

	return nrOfFailedTestCases;
}

template<size_t es, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating, typename NativeFloatingPointType>
int TestDoubleBlockRepresentations(const std::string& op, bool reportTestCases, bool bVerbose) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;

	// 2 block representations

	std::string testcase = "-";

	if constexpr (es < 7) {
		using CfloatConfiguration = cfloat<9, es, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<CfloatConfiguration, NativeFloatingPointType>(reportTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 8) {
		using CfloatConfiguration = cfloat<10, es, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<CfloatConfiguration, NativeFloatingPointType>(reportTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 10) {
		using CfloatConfiguration = cfloat<12, es, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<CfloatConfiguration, NativeFloatingPointType>(reportTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 12) {
		using CfloatConfiguration = cfloat<14, es, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<CfloatConfiguration, NativeFloatingPointType>(reportTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 14) {
		using CfloatConfiguration = cfloat<16, es, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<CfloatConfiguration, NativeFloatingPointType>(reportTestCases, bVerbose), testcase, op);
	}
	if constexpr (es < 18) {
		using CfloatConfiguration = cfloat<20, es, uint16_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<CfloatConfiguration, NativeFloatingPointType>(reportTestCases, bVerbose), testcase, op);
	}

	return nrOfFailedTestCases;
}

template<size_t es, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating, typename NativeFloatingPointType>
int TestTripleBlockRepresentations(const std::string& op, bool reportTestCases, bool bVerbose) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;

	// 3 block representations

	std::string testcase;

	if constexpr (es < 18) {
		using CfloatConfiguration = cfloat<20, es, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating>;
		nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling<CfloatConfiguration, NativeFloatingPointType>(reportTestCases, bVerbose), testcase, op);
	}

	return nrOfFailedTestCases;
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
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

	std::string test_suite  = "cfloat<> assignment";
	std::string test_tag    = "assignment";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	using Real = sw::universal::cfloat<8, 2>;

	bool bConversionTest = true;
	if (bConversionTest) {
		float test = 0.0625f;
		std::cout << to_binary(test) << " : " << test << std::endl;
		ConversionTest<cfloat<8, 1, uint8_t, true, true, false>>(test);  // es = 1 requires sub and max-exponent values to be configured
		ConversionTest<cfloat<8, 2>>(test);
		ConversionTest<cfloat<8, 3>>(test);
		ConversionTest<cfloat<8, 4>>(test);
	}

	{
		sw::universal::cfloat<9, 1, uint8_t, true, true, false> a = -0.03125f;
		std::cout << color_print(a) << " : " << a << std::endl;
	}

	nrOfFailedTestCases += ReportTestResult(VerifySubnormalReverseSampling< cfloat<9, 1, uint8_t, true, true, false>, float>(reportTestCases, false), "cfloat<9,1, uint8_t>", "=float");
	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< cfloat<9, 1, uint8_t, true, true, false>, float>(true, false), "cfloat<9,1, uint8_t>", "=float");

	nrOfFailedTestCases += ReportTestResult(VerifyReverseSampling< cfloat<5, 2, uint8_t, false, false, false>, float>(reportTestCases, false), "cfloat<5,2> normals only", "=float");
	nrOfFailedTestCases += TestSingleBlockRepresentations<2, false, false, false, float>("=float", false, false);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else //!MANUAL_TESTING
	constexpr bool bVerbose        = false;
	constexpr bool hasSubnormals   = true;
	constexpr bool noSubnormals    = false;
	constexpr bool hasMaxExpValues = true;
	constexpr bool noSupernormals  = false;
	constexpr bool notSaturating   = false;

#if REGRESSION_LEVEL_1

	std::cout << "Special cases: zero, inf, nan\n";
	nrOfFailedTestCases += ReportTestResult(TestSpecialCases<sw::universal::cfloat< 8, 2, std::uint8_t, noSubnormals, noSupernormals, notSaturating> >(reportTestCases), "cfloat< 8, 2, std::uint8_t, noSubnormals, noSupernormals, notSaturating>", "special cases");
	nrOfFailedTestCases += ReportTestResult(TestSpecialCases<sw::universal::cfloat< 8, 2, std::uint8_t, hasSubnormals, noSupernormals, notSaturating> >(reportTestCases), "cfloat< 8, 2, std::uint8_t, hasSubnormals, noSupernormals, notSaturating>", "special cases");
	nrOfFailedTestCases += ReportTestResult(TestSpecialCases<sw::universal::cfloat< 8, 2, std::uint8_t, noSubnormals, hasMaxExpValues, notSaturating> >(reportTestCases), "cfloat< 8, 2, std::uint8_t, noSubnormals, hasMaxExpValues, notSaturating>", "special cases");
	nrOfFailedTestCases += ReportTestResult(TestSpecialCases<sw::universal::cfloat< 8, 2, std::uint8_t, hasSubnormals, hasMaxExpValues, notSaturating> >(reportTestCases), "cfloat< 8, 2, std::uint8_t, hasSubnormals, hasMaxExpValues, notSaturating>", "special cases");
	nrOfFailedTestCases += ReportTestResult(TestSpecialCases<sw::universal::cfloat<32, 8, std::uint32_t, noSubnormals, noSupernormals, notSaturating> >(reportTestCases), "cfloat<32, 8, std::uint32_t, noSubnormals, noSupernormals, notSaturating>", "special cases");
	nrOfFailedTestCases += ReportTestResult(TestSpecialCases<sw::universal::cfloat<32, 8, std::uint32_t, hasSubnormals, noSupernormals, notSaturating> >(reportTestCases), "cfloat<32, 8, std::uint32_t, hasSubnormals, noSupernormals, notSaturating>", "special cases");
	nrOfFailedTestCases += ReportTestResult(TestSpecialCases<sw::universal::cfloat<32, 8, std::uint32_t, noSubnormals, hasMaxExpValues, notSaturating> >(reportTestCases), "cfloat<32, 8, std::uint32_t, noSubnormals, hasMaxExpValues, notSaturating>", "special cases");
	nrOfFailedTestCases += ReportTestResult(TestSpecialCases<sw::universal::cfloat<32, 8, std::uint32_t, hasSubnormals, hasMaxExpValues, notSaturating> >(reportTestCases), "cfloat<32, 8, std::uint32_t, hasSubnormals, hasMaxExpValues, notSaturating>", "special cases");
	nrOfFailedTestCases += ReportTestResult(TestSpecialCases<sw::universal::cfloat<64,11, std::uint32_t, noSubnormals, noSupernormals, notSaturating> >(reportTestCases), "cfloat<64,11, std::uint32_t, noSubnormals, noSupernormals, notSaturating>", "special cases");
	nrOfFailedTestCases += ReportTestResult(TestSpecialCases<sw::universal::cfloat<64,11, std::uint32_t, hasSubnormals, noSupernormals, notSaturating> >(reportTestCases), "cfloat<64,11, std::uint32_t, hasSubnormals, noSupernormals, notSaturating>", "special cases");
	nrOfFailedTestCases += ReportTestResult(TestSpecialCases<sw::universal::cfloat<64,11, std::uint32_t, noSubnormals, hasMaxExpValues, notSaturating> >(reportTestCases), "cfloat<64,11, std::uint32_t, noSubnormals, hasMaxExpValues, notSaturating>", "special cases");
	nrOfFailedTestCases += ReportTestResult(TestSpecialCases<sw::universal::cfloat<64,11, std::uint32_t, hasSubnormals, hasMaxExpValues, notSaturating> >(reportTestCases), "cfloat<64,11, std::uint32_t, hasSubnormals, hasMaxExpValues, notSaturating>", "special cases");

	std::cout << "\ncfloat<> with only normal encodings\n";
	std::cout << "Single block representations\n--------------------------------------------- es = 2 encodings\n";
	nrOfFailedTestCases += TestSingleBlockRepresentations<2, noSubnormals, noSupernormals, notSaturating, float>("=float", true, bVerbose);
	nrOfFailedTestCases += TestSingleBlockRepresentations<2, noSubnormals, noSupernormals, notSaturating, double>("=double", reportTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 3 encodings\n";
	nrOfFailedTestCases += TestSingleBlockRepresentations<3, noSubnormals, noSupernormals, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestSingleBlockRepresentations<3, noSubnormals, noSupernormals, notSaturating, double>("=double", reportTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 4 encodings\n";
	nrOfFailedTestCases += TestSingleBlockRepresentations<4, noSubnormals, noSupernormals, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestSingleBlockRepresentations<4, noSubnormals, noSupernormals, notSaturating, double>("=double", reportTestCases, bVerbose);

	std::cout << "Double block representations\n--------------------------------------------- es = 2 encodings\n";
	nrOfFailedTestCases += TestDoubleBlockRepresentations<2, noSubnormals, noSupernormals, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestDoubleBlockRepresentations<2, noSubnormals, noSupernormals, notSaturating, double>("=double", reportTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 3 encodings\n";
	nrOfFailedTestCases += TestDoubleBlockRepresentations<3, noSubnormals, noSupernormals, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestDoubleBlockRepresentations<3, noSubnormals, noSupernormals, notSaturating, double>("=double", reportTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 4 encodings\n";
	nrOfFailedTestCases += TestDoubleBlockRepresentations<4, noSubnormals, noSupernormals, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestDoubleBlockRepresentations<4, noSubnormals, noSupernormals, notSaturating, double>("=double", reportTestCases, bVerbose);

	std::cout << "Triple block representations\n--------------------------------------------- es = 2 encodings\n";
	nrOfFailedTestCases += TestTripleBlockRepresentations<2, noSubnormals, noSupernormals, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestTripleBlockRepresentations<2, noSubnormals, noSupernormals, notSaturating, double>("=double", reportTestCases, bVerbose);

	std::cout << "\ncfloat<> with subnormal + normal\n";
	std::cout << "Single block representations\n--------------------------------------------- es = 2 encodings\n";
	nrOfFailedTestCases += TestSingleBlockRepresentations<2, hasSubnormals, noSupernormals, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestSingleBlockRepresentations<2, hasSubnormals, noSupernormals, notSaturating, double>("=double", reportTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 3 encodings\n";
	nrOfFailedTestCases += TestSingleBlockRepresentations<3, hasSubnormals, noSupernormals, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestSingleBlockRepresentations<3, hasSubnormals, noSupernormals, notSaturating, double>("=double", reportTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 4 encodings\n";
	nrOfFailedTestCases += TestSingleBlockRepresentations<4, hasSubnormals, noSupernormals, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestSingleBlockRepresentations<4, hasSubnormals, noSupernormals, notSaturating, double>("=double", reportTestCases, bVerbose);

	std::cout << "Double block representations\n--------------------------------------------- es = 2 encodings\n";
	nrOfFailedTestCases += TestDoubleBlockRepresentations<2, hasSubnormals, noSupernormals, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestDoubleBlockRepresentations<2, hasSubnormals, noSupernormals, notSaturating, double>("=double", reportTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 3 encodings\n";
	nrOfFailedTestCases += TestDoubleBlockRepresentations<3, hasSubnormals, noSupernormals, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestDoubleBlockRepresentations<3, hasSubnormals, noSupernormals, notSaturating, double>("=double", reportTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 4 encodings\n";
	nrOfFailedTestCases += TestDoubleBlockRepresentations<4, hasSubnormals, noSupernormals, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestDoubleBlockRepresentations<4, hasSubnormals, noSupernormals, notSaturating, double>("=double", reportTestCases, bVerbose);

	std::cout << "Triple block representations\n--------------------------------------------- es = 2 encodings\n";
	nrOfFailedTestCases += TestTripleBlockRepresentations<2, hasSubnormals, noSupernormals, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestTripleBlockRepresentations<2, hasSubnormals, noSupernormals, notSaturating, double>("=double", reportTestCases, bVerbose);

	std::cout << "\ncfloat<> with normal + max-exponent value\n";
	std::cout << "Single block representations\n--------------------------------------------- es = 2 encodings\n";
	nrOfFailedTestCases += TestSingleBlockRepresentations<2, noSubnormals, hasMaxExpValues, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestSingleBlockRepresentations<2, noSubnormals, hasMaxExpValues, notSaturating, double>("=double", reportTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 3 encodings\n";
	nrOfFailedTestCases += TestSingleBlockRepresentations<3, noSubnormals, hasMaxExpValues, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestSingleBlockRepresentations<3, noSubnormals, hasMaxExpValues, notSaturating, double>("=double", reportTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 4 encodings\n";
	nrOfFailedTestCases += TestSingleBlockRepresentations<4, noSubnormals, hasMaxExpValues, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestSingleBlockRepresentations<4, noSubnormals, hasMaxExpValues, notSaturating, double>("=double", reportTestCases, bVerbose);

	std::cout << "Double block representations\n--------------------------------------------- es = 2 encodings\n";
	nrOfFailedTestCases += TestDoubleBlockRepresentations<2, noSubnormals, hasMaxExpValues, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestDoubleBlockRepresentations<2, noSubnormals, hasMaxExpValues, notSaturating, double>("=double", reportTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 3 encodings\n";
	nrOfFailedTestCases += TestDoubleBlockRepresentations<3, noSubnormals, hasMaxExpValues, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestDoubleBlockRepresentations<3, noSubnormals, hasMaxExpValues, notSaturating, double>("=double", reportTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 4 encodings\n";
	nrOfFailedTestCases += TestDoubleBlockRepresentations<4, noSubnormals, hasMaxExpValues, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestDoubleBlockRepresentations<4, noSubnormals, hasMaxExpValues, notSaturating, double>("=double", reportTestCases, bVerbose);

	std::cout << "Triple block representations\n--------------------------------------------- es = 2 encodings\n";
	nrOfFailedTestCases += TestTripleBlockRepresentations<2, noSubnormals, hasMaxExpValues, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestTripleBlockRepresentations<2, noSubnormals, hasMaxExpValues, notSaturating, double>("=double", reportTestCases, bVerbose);

	std::cout << "\ncfloat<> with subnormal + normal + max-exponent value\n";
	std::cout << "Single block representations\n--------------------------------------------- es = 1 encodings\n";
	nrOfFailedTestCases += TestSingleBlockRepresentations<1, hasSubnormals, hasMaxExpValues, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestSingleBlockRepresentations<1, hasSubnormals, hasMaxExpValues, notSaturating, double>("=double", reportTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 2 encodings\n";
	nrOfFailedTestCases += TestSingleBlockRepresentations<2, hasSubnormals, hasMaxExpValues, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestSingleBlockRepresentations<2, hasSubnormals, hasMaxExpValues, notSaturating, double>("=double", reportTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 3 encodings\n";
	nrOfFailedTestCases += TestSingleBlockRepresentations<3, hasSubnormals, hasMaxExpValues, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestSingleBlockRepresentations<3, hasSubnormals, hasMaxExpValues, notSaturating, double>("=double", reportTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 4 encodings\n";
	nrOfFailedTestCases += TestSingleBlockRepresentations<4, hasSubnormals, hasMaxExpValues, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestSingleBlockRepresentations<4, hasSubnormals, hasMaxExpValues, notSaturating, double>("=double", reportTestCases, bVerbose);

	std::cout << "Double block representations\n--------------------------------------------- es = 1 encodings\n";
	nrOfFailedTestCases += TestDoubleBlockRepresentations<1, hasSubnormals, hasMaxExpValues, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestDoubleBlockRepresentations<1, hasSubnormals, hasMaxExpValues, notSaturating, double>("=double", reportTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 2 encodings\n";
	nrOfFailedTestCases += TestDoubleBlockRepresentations<2, hasSubnormals, hasMaxExpValues, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestDoubleBlockRepresentations<2, hasSubnormals, hasMaxExpValues, notSaturating, double>("=double", reportTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 3 encodings\n";
	nrOfFailedTestCases += TestDoubleBlockRepresentations<3, hasSubnormals, hasMaxExpValues, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestDoubleBlockRepresentations<3, hasSubnormals, hasMaxExpValues, notSaturating, double>("=double", reportTestCases, bVerbose);
	std::cout << "--------------------------------------------- es = 4 encodings\n";
	nrOfFailedTestCases += TestDoubleBlockRepresentations<4, hasSubnormals, hasMaxExpValues, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestDoubleBlockRepresentations<4, hasSubnormals, hasMaxExpValues, notSaturating, double>("=double", reportTestCases, bVerbose);

	std::cout << "Triple block representations\n--------------------------------------------- es = 1 encodings\n";
	nrOfFailedTestCases += TestTripleBlockRepresentations<1, hasSubnormals, hasMaxExpValues, notSaturating, float>("=float", reportTestCases, bVerbose);
	nrOfFailedTestCases += TestTripleBlockRepresentations<1, hasSubnormals, hasMaxExpValues, notSaturating, double>("=double", reportTestCases, bVerbose);
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
