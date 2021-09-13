// from_blocktriple.cpp: test suite runner for conversion tests between blocktriple and cfloats
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

#include <universal/number/cfloat/cfloat_impl.hpp>
#include <universal/number/cfloat/manipulators.hpp>
#include <universal/number/cfloat/mathlib.hpp>
#include <universal/verification/test_suite_conversion.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

/*
   DESIGN and IMPLEMENTATION HISTORY

   The first floating-point back-end design, value<fbits>, had a fraction 
   bit parameter to select  among different normalizations for 
   addition, multiplication, and division. Inside, these operators
   we would expand and align the operands as needed, requiring a copy.
   
   But the normalization is NOT a generic op, it is very specific for 
   add, mul, div, or sqrt, thus having a fully parameterized interface 
   creates a state space for bugs that could get triggered by incorrect 
   calling of the normalize method. Secondly, no efficient unit test was 
   feasible as most of the state space would NOT be valid conversions.
   Given that context of the experience with value<> we decided to clamp down
   on this parameterization overkill and create explicit normalization 
   conversions for add, mul, div, and sqrt. 
 */

namespace sw::universal {

	/// <summary>
	/// convert a blocktriple to a cfloat
	/// </summary>
	/// <typeparam name="CfloatConfiguration"></typeparam>
	/// <param name="bReportIndividualTestCases"></param>
	/// <returns></returns>
	template<typename CfloatConfiguration, BlockTripleOperator op>
	int VerifyCfloatFromBlocktripleConversion(bool bReportIndividualTestCases) {
		using namespace sw::universal;
		constexpr size_t nbits = CfloatConfiguration::nbits;
		constexpr size_t es = CfloatConfiguration::es;
		using bt = typename CfloatConfiguration::BlockType;
		constexpr bool hasSubnormals = CfloatConfiguration::hasSubnormals;
		constexpr bool hasSupernormals = CfloatConfiguration::hasSupernormals;
		constexpr bool isSaturating = CfloatConfiguration::isSaturating;
		constexpr size_t fbits = CfloatConfiguration::fbits;

		int nrOfTestFailures{ 0 };

		cfloat<nbits, es, bt, hasSubnormals, hasSupernormals, isSaturating> a, nut;

		/// blocktriple addition and subtraction is done in a 2's complement format 0ii.fffff.
		/// blocktriple multiplication is done in a 1's complement format of ii.fffff
		/// blocktriple division is done in a ?'s complement format of ???????
		/// 
		/// blocktriples can be in overflow configuration, but not in denormalized form
		/// 
		/// BlockTripleOperator::ADD  blocktriple type that comes out of an addition or subtraction operation
		/// BlockTripleOperator::MUL  blocktriple type that comes out of a multiplication operation
 		/// BlockTripleOperator::DIV  blocktriple type that comes out of a division operation

		using BlockTripleConfiguration = blocktriple<fbits, op, bt>;
		BlockTripleConfiguration b;
		std::cout << "\n+-----\n" << type_tag(b) << "  radix point at " << BlockTripleConfiguration::radix << '\n';
		for (int scale = -8; scale < 8; ++scale) {
			// if ADD, pattern is  0ii.fffff, without 000.fffff     // convert does not expect negative 2's complement numbers
			// if MUL, patterns is  ii.fffff, without  00.fffff
			// blocktriples are normal or overflown, so we need to enumerate 2^2 * 2^fbits cases
			size_t fractionBits{ 0 };
			size_t integerSet{ 0 };
			if constexpr (op == BlockTripleOperator::ADD) {
				fractionBits = fbits; // make it explicit for ease of understanding
				integerSet = 4;
			}
			if constexpr (op == BlockTripleOperator::MUL) {
				fractionBits = 2 * fbits;
				integerSet = 4;
			}
			size_t NR_VALUES = (1ull << fractionBits);
			b.setscale(scale);
			for (size_t i = 1; i < integerSet; ++i) {  // 01, 10, 11.fffff: state 00 is not part of the encoding as that would represent a denormal
				size_t integerBits = i * NR_VALUES;
				for (size_t f = 0; f < NR_VALUES; ++f) {
					b.setbits(integerBits + f);

//					std::cout << "blocktriple: " << to_binary(b) << " : " << b << '\n';

					convert(b, nut);

					// get the reference by marshalling the blocktriple value through a double value and assigning it to the cfloat
					a = double(b);
					if (a != nut) {
//						std::cout << "blocktriple: " << to_binary(b) << " : " << b << " vs " << to_binary(nut) << " : " << nut << '\n';

						if (a.isnan() && b.isnan()) continue;
						if (a.isinf() && b.isinf()) continue;

						++nrOfTestFailures;
						if (bReportIndividualTestCases) std::cout << "FAIL: " << to_triple(b) << " : " << std::setw(10) << b << " -> " << to_binary(nut) << " != ref " << to_binary(a) << " or " << nut << " != " << a << '\n';
					}
					else {
#ifndef VERBOSE_POSITIVITY
						if (bReportIndividualTestCases) std::cout << "PASS: " << to_triple(b) << " : " << std::setw(10) << b << " -> " << to_binary(nut) << " == ref " << to_binary(a) << " or " << nut << " == " << a << '\n';
#endif
					}
				}
			}
		}
		return nrOfTestFailures;
	}

}

/*
How do you test the conversion state space of blocktriple to cfloat.
We need to convert the blocktriple that comes out of an ADD, a MUL, and a DIV operation.
The blocktriples have bits that need to be rounded by convert.
How do you test that rounding?

Convert the blocktriple to a value.
Use the cfloat operator=() to round. That is your reference. This assumes that cfloat operator=() has been validated.
Use convert() to convert to a cfloat.
Compare the operator=() and convert() cfloat patterns to check correctness
 */

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	constexpr bool hasSubnormals = true;
	constexpr bool hasSupernormals = true;
	constexpr bool isSaturating = false;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;
	std::string tag = "conversion ";

#if MANUAL_TESTING

	// cfloat<> is a linear floating-point

	std::cout << "Conversion from blocktriple to cfloat\n\n";

	// to track conversion in more detail
	std::cout << std::setprecision(8);
	std::cerr << std::setprecision(8);

	{
		// how do you round a non-normalized blocktriple?
		// you would need to modify the lsb/guard/round/sticky bit masks
		// so that you use all info to make the rounding decision,
		// then normalize (basically shift to the right) and apply
		// the rounding decision.
		{
			using Cfloat = cfloat<4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
			constexpr size_t fbits = Cfloat::fbits;
			typedef Cfloat::BlockType bt;
			blocktriple<fbits, BlockTripleOperator::ADD, bt> b;
			// 0b001.1  == 0.75, scale = -1
			b.setbits(0x03);
			b.setscale(-1);
			float v = float(b);
			Cfloat nut, ref;
			convert(b, nut);
			ref = v;
			std::cout << "blocktriple: " << to_binary(b) << " : " << float(b) << '\n';
			std::cout << "cfloat     : " << to_binary(nut) << " : " << nut << '\n';
			std::cout << "cfloat ref : " << to_binary(ref) << " : " << ref << '\n';
		}

		{
			// checking the other side of the exponential adjustments with cfloats
			// that expand on the dynamic range of IEEE-754
			using Cfloat = cfloat<80, 15, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
			Cfloat a;
			a = -1.0f;
			std::cout << type_tag(a) << '\n' << to_binary(a) << " : " << a << '\n';
			//			a.constexprClassParameters();
		}

		{
			using Cfloat = cfloat<5, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>;
			constexpr size_t fbits = Cfloat::fbits;
			typedef Cfloat::BlockType bt;
			blocktriple<fbits, BlockTripleOperator::MUL, bt> b; // blocktriple type that comes out of a multiplication operation
			// 0b01.1110  == 1.875
			b.setbits(0x1e);
			float v = float(b);
			Cfloat nut, ref;
			convert(b, nut);
			ref = v;
			std::cout << "blocktriple: " << to_binary(b) << " : " << float(b) << '\n';
			std::cout << "cfloat     : " << to_binary(nut) << " : " << nut << '\n';
			std::cout << "cfloat ref : " << to_binary(ref) << " : " << ref << '\n';
		}
	}

	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD>(bReportIndividualTestCases), tag, "cfloat<8,1> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD>(bReportIndividualTestCases), tag, "cfloat<8,2> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD>(bReportIndividualTestCases), tag, "cfloat<8,3> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD>(bReportIndividualTestCases), tag, "cfloat<8,4> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD>(bReportIndividualTestCases), tag, "cfloat<8,5> from blocktriple ADD");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatFromBlocktripleConversion<cfloat<8, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD>(bReportIndividualTestCases), tag, "cfloat<8,6> from blocktriple ADD");

	std::cout << "failed tests: " << nrOfFailedTestCases << '\n';
	nrOfFailedTestCases = 0; // in manual testing we ignore failures for the regression system

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else  // !MANUAL_TESTING

	std::cout << "cfloat from blocktriple conversion validation" << '\n';

	// es = 1
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 3, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 3,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 4, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 4,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 5, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 5,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 6, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 6,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 7,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 8,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 9, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 9,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<10,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<12,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<16, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<16,1>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 1, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<18,1>");   // 3 blocks


	// es = 2
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 4, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 4,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 5, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 5,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 6, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 6,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 7,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 8,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<10,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<12,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<14,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<16, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<16,2>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 2, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<18,2>");   // 3 blocks


	// es = 3
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 5, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 5,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 6, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 6,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 7,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 8,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<10,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<12,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<14,3>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 3, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<18,3>");   // 3 blocks


	// es = 4
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 6, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 6,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 7,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 8,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<10,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<12,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<14,4>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 4, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<18,4>");   // 3 blocks


	// es = 5
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 7, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 7,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 8,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<10,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<12,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<14,5>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<18, 5, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<18,5>");   // 3 blocks


	// es = 6
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 8, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 8,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 9, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 9,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<10,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<12,6>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 6, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<14,6>");


	// es = 7
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat< 9, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat< 9,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<10, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<10,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<12,7>");
	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 7, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<14,7>");

	// still failing
	// es = 8
//	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<11, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<11,8>");
//	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<12, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<12,8>");
//	nrOfFailedTestCases = ReportTestResult(VerifyCfloatFromBlocktripleConversion< cfloat<14, 8, uint8_t, hasSubnormals, hasSupernormals, isSaturating>, BlockTripleOperator::ADD >(bReportIndividualTestCases), tag, "cfloat<14,8>");


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


/*

  To generate:
  	GenerateFixedPointComparisonTable<4, 0>(std::string("-"));
	GenerateFixedPointComparisonTable<4, 1>(std::string("-"));
	GenerateFixedPointComparisonTable<4, 2>(std::string("-"));
	

 */
