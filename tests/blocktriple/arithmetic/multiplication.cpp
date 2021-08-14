// multiplication.cpp: functional tests for blocktriple number multiplication
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/bit_cast.hpp>
#include <iostream>
#include <iomanip>

// temporary
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/internal/bitblock/bitblock.hpp>
// minimum set of include files to reflect source code dependencies                    
#include <universal/native/ieee754.hpp>
// uncomment to enable operator tracing
#define BLOCKTRIPLE_VERBOSE_OUTPUT
#define BLOCKTRIPLE_TRACE_MUL
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/test_reporters.hpp>

// enumerate all multiplication cases for an blocktriple<nbits,BlockType> configuration
template<typename BlockTripleConfiguration>
int VerifyMultiplication(bool bReportIndividualTestCases) {
	constexpr size_t fbits = BlockTripleConfiguration::fbits;  // just the number of fraction bits
	constexpr size_t mbits = BlockTripleConfiguration::mbits;
	using BlockType = typename BlockTripleConfiguration::BlockType;

	// for the test we are going to enumerate the fbits state space
	// and then shift the values into place into the declared ALU inputs.
	// forall i in NR_VALUES
	//    setBits(i + shiftLeft + hiddenBit);
	constexpr size_t NR_VALUES = (size_t(1) << fbits);

	using namespace std;
	using namespace sw::universal;
	
	cout << endl;
	cout << "blocktriple<" <<fbits << ',' << typeid(BlockType).name() << '>' << endl;
	cout << "Fraction        bits : " << fbits << endl;
	cout << "Multiplication  bits : " << mbits << endl;

	/*
		blocktriple<fbits> has fbits fraction bits in the form h.<fbits>

		example: blocktriple<3> represents values 
		1.000
		1.001
		1.010
		...
		1.101
		1.110
		1.111

		The scale shifts these value relative to 1. So a scale of -3 shifts these bits to the right, a scale of +3 shifts them to the left

	 */

	int nrOfFailedTests = 0;

	// when multiplying, we generate 2*fhbits result bits
	// which we'll need to round using round-nearest-tie-to-even
	//       lsb|guard|round|sticky.

	/*
	 * the structure of a blocktriple is always h.ffff, that is,
	 * we have an explicit normal bit.
	 */

	blocktriple<mbits> a, b, c, refResult;
	constexpr size_t hiddenBit = (size_t(1) << mbits);
	a.setnormal();
	b.setnormal();
	c.setnormal();  // we are only enumerating normal values, special handling is not tested here

	// key problem: the mul operator will change the arguments
	// This implies that we need to set up the values of the blocktriples in the inner loop,
	// that is, the test input values will not remain invariant as they are manipulated by mul();
	double aref, bref, cref;
	for (int scale = -2; scale < 3; ++scale) {
		for (size_t i = 0; i < NR_VALUES; i++) {
			for (size_t j = 0; j < NR_VALUES; j++) {
				// set the a input test value
				a.setbits(i*8 + hiddenBit);  // mix in the hidden bit in the blockfraction
				a.setscale(scale);
				// set the b input test value
				b.setbits(j*8 + hiddenBit);
				b.setscale(0);

				cout << to_binary(a) << " * " << to_binary(b) << endl;
				cout << a << " * " << b << " = " << c << endl;

				c.mul(a, b); // generate the mul value under test
				
				aref = double(a); // cast to double is reasonable constraint for exhaustive test
				bref = double(b); // cast to double is reasonable constraint for exhaustive test
				cref = aref + bref; // calculate the reference test value

				refResult = cref; // sample the reference test value

				if (c != refResult) {
//					cout << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(c) << endl;
//					cout << a << " * " << b << " = " << c << endl;
//					cout << aref << " * " << bref << " = " << cref << " vs " << refResult << endl;

					++nrOfFailedTests;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", a, b, c, refResult);
//					cout << "---------------------\n";
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, c, refResult);
				}
//				if (nrOfFailedTests > 24) return nrOfFailedTests;
			}
			//		if (i % 1024 == 0) cout << '.'; /// if you enable this, put the endl back
		}
	}
//	cout << endl;
	return nrOfFailedTests;
}

// generate specific test case that you can trace with the trace conditions in blocktriple
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, typename ArgumentType>
void GenerateTestCase(ArgumentType lhs, ArgumentType rhs) {
	using namespace sw::universal;
	blocktriple<nbits> a, b;
	blocktriple<nbits+1> result, reference;

	// convert to blocktriple
	a = lhs;
	b = rhs;
	result.mul(a, b);

	// convert blocktriples back to argument type
	ArgumentType _a, _b, _c;
	_a = ArgumentType(a);
	_b = ArgumentType(b);
	_c = _a * _b;

	// check that the round-trip through the blocktriple yields the same value as direct conversion
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << lhs << " * " << std::setw(nbits) << rhs << " = " << std::setw(nbits) << lhs + rhs << '\n';
	std::cout << std::setw(nbits) << _a << " * " << std::setw(nbits) << _b << " = " << std::setw(nbits) << _c << '\n';
	std::cout << to_binary(a) << " * " << to_binary(b) << " = " << to_binary(result) << " (reference: " << _c << ")   " << '\n';
	reference = _c;
	std::cout << (result == reference ? "PASS" : "FAIL") << '\n' << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	print_cmd_line(argc, argv);
	
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "blocktriple multiplication failed: ";

#if MANUAL_TESTING

	{
		blocktripleM<2, uint8_t> a, b, c;
		std::cout << to_binary(a) << '\n';
		a = 1.0f;
		b = 0.5f;
		c.mul(a, b);
		std::cout << a << " * " << b << " = " << c << '\n';
		std::cout << to_triple(a) << " * " << to_triple(b) << " = " << to_triple(c) << '\n';
	}
	return 0;
	// 2^4 = 16 * 16 * 13 =  3328 : 568 fails
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple< 2, uint8_t> >(bReportIndividualTestCases), "blocktriple<2, uint8_t>", "multiplication");
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple< 4, uint8_t> >(bReportIndividualTestCases), "blocktriple<4, uint8_t>", "multiplication");
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple< 8, uint8_t> >(bReportIndividualTestCases), "blocktriple<8, uint8_t>", "multiplication");
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<12, uint8_t> >(bReportIndividualTestCases), "blocktriple<12, uint8_t>", "multiplication");
//	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<12, uint16_t> >(bReportIndividualTestCases), "blocktriple<12, uint16_t>", "multiplication");

#if STRESS_TESTING

#endif

	// manual test does not report failures
	nrOfFailedTestCases = 0;

#else

	cout << "blocktriple multiplication validation" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<4, uint8_t> >(bReportIndividualTestCases), "blocktriple<4,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<4, uint16_t> >(bReportIndividualTestCases), "blocktriple<4,uint16_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<4, uint32_t> >(bReportIndividualTestCases), "blocktriple<4,uint32_t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<8, uint8_t> >(bReportIndividualTestCases), "blocktriple<8,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<8, uint16_t> >(bReportIndividualTestCases), "blocktriple<8,uint16_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<8, uint32_t> >(bReportIndividualTestCases), "blocktriple<8,uint32_t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<9, uint8_t> >(bReportIndividualTestCases), "blocktriple<9,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<9, uint16_t> >(bReportIndividualTestCases), "blocktriple<9,uint16_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<9, uint32_t> >(bReportIndividualTestCases), "blocktriple<9,uint32_t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<10, uint8_t> >(bReportIndividualTestCases), "blocktriple<10,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<10, uint16_t> >(bReportIndividualTestCases), "blocktriple<10,uint16_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<10, uint32_t> >(bReportIndividualTestCases), "blocktriple<10,uint32_t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<11, uint8_t> >(bReportIndividualTestCases), "blocktriple<11,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<11, uint16_t> >(bReportIndividualTestCases), "blocktriple<11,uint16_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<11, uint32_t> >(bReportIndividualTestCases), "blocktriple<11,uint32_t>", "multiplication");

	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<12, uint8_t> >(bReportIndividualTestCases), "blocktriple<12,uint8_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<12, uint16_t> >(bReportIndividualTestCases), "blocktriple<12,uint16_t>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyMultiplication< blocktriple<12, uint32_t> >(bReportIndividualTestCases), "blocktriple<12,uint32_t>", "multiplication");

#if STRESS_TESTING



#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
