// division.cpp: functional tests for arbitrary configuration fixed-point division
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1

// minimum set of include files to reflect source code dependencies
#include <universal/fixpnt/fixed_point.hpp>
// fixed-point type manipulators such as pretty printers
#include <universal/fixpnt/fixpnt_manipulators.hpp>
#include <universal/fixpnt/fixpnt_functions.hpp>
#include "../utils/fixpnt_test_suite.hpp"

// unrounded division, returns a blockbinary that is of size 2*nbits
template<size_t nbits, size_t roundingBits, typename BlockType>
inline sw::unum::blockbinary<2 * nbits + roundingBits, BlockType> unrounded_div(const sw::unum::blockbinary<nbits, BlockType>& a, const sw::unum::blockbinary<nbits, BlockType>& b, sw::unum::blockbinary<roundingBits, BlockType>& r) {
	using namespace sw::unum;

	if (b.iszero()) {
		// division by zero
		throw "urdiv divide by zero";
	}
	// generate the absolute values to do long division 
	// 2's complement special case -max requires an signed int that is 1 bit bigger to represent abs()
	bool a_sign = a.sign();
	bool b_sign = b.sign();
	bool result_negative = (a_sign ^ b_sign);

	// normalize both arguments to positive in new size
	blockbinary<nbits + 1, BlockType> a_new(a); // TODO optimize: now create a, create _a.bb, copy, destroy _a.bb_copy
	blockbinary<nbits + 1, BlockType> b_new(b);
	if (a_sign) a_new.twoscomplement();
	if (b_sign) b_new.twoscomplement();

	// initialize the long division
	blockbinary<2 * nbits + roundingBits, BlockType> decimator(a);
	decimator <<= nbits + roundingBits - 1; // scale the decimator to the largest possible positive value
	blockbinary<2 * nbits + roundingBits, BlockType> subtractand(b); // prepare the subtractand
	blockbinary<2 * nbits + roundingBits, BlockType> result;


	std::cout << to_binary(subtractand) << ' ' << to_binary(decimator) << std::endl;

	int msb_b = subtractand.msb();
	int msb_a = decimator.msb();
	int shift = msb_a - msb_b;
	subtractand <<= shift;

	std::cout << to_binary(subtractand) << ' ' << to_binary(decimator) << ' ' << to_binary(result) << " shift: " << shift << std::endl;

	// long division
	for (int i = shift; i >= 0; --i) {

		std::cout << to_binary(subtractand) << ' ' << to_binary(decimator) << std::endl;

		if (subtractand <= decimator) {
			decimator -= subtractand;
			result.set(i);
		}
		else {
			result.reset(i);
		}
		subtractand >>= 1;

		std::cout << to_binary(subtractand) << ' ' << to_binary(decimator) << ' ' << to_binary(result) << std::endl;

	}
	r.assign(result); // copy the lowest bits which represent the bits on which we need to apply the rounding test
	return result;
}

// generate specific test case that you can trace with the trace conditions in fixed_point.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t rbits, typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	Ty ref;
	sw::unum::fixpnt<nbits, rbits> a, b, cref, result;
	a = _a;
	b = _b;
	result = a / b;
	ref = _a / _b;
	cref = ref;
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << _a << " / " << std::setw(nbits) << _b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << a << " / " << b << " = " << result << " (reference: " << cref << ")   " ;
	std::cout << (cref == result ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	int nrOfFailedTestCases = 0;

	std::string tag = "modular division: ";

#if MANUAL_TESTING

	constexpr size_t nbits = 8;
	constexpr size_t rbits = 4;

	{
		constexpr size_t nbits = 8;
		constexpr size_t rbits = 4;

		fixpnt<nbits,rbits> a, b;
		a.set_raw_bits(0xCC);
		b.set_raw_bits(0x55);
		constexpr size_t roundingDecisionBits = 4; // guard, round, and 2 sticky bits
		blockbinary<roundingDecisionBits> roundingBits;
		blockbinary<2 * nbits> c = unrounded_div(a.getbb(), b.getbb(), roundingBits);
		std::cout << a << " / " << b << std::endl;
		std::cout << a.getbb() << " / " << b.getbb() << " = " << to_binary(c) << " rounding bits " << to_binary(roundingBits);
		bool roundUp = c.roundingMode(rbits + 4);
		c >>= nbits + roundingDecisionBits - 1;
		if (roundUp) ++c;
		std::cout << " rounded " << to_binary(c) << std::endl;
		//this->bb = c; // select the lower nbits of the result
	}

	return 0;

	// generate individual testcases to hand trace/debug
//	fixpnt<4, 1> a, b, c;
	fixpnt<6, 2> a, b, c;
	a.set_raw_bits(0xCC);
	cout << a << ' ' << to_binary(a) << ' ' << float(a) << endl;
	b.set_raw_bits(0x55);
	cout << b << ' ' << to_binary(b) << ' ' << float(b) << endl;
	c = a * b;
	c = b * a;

	GenerateTestCase<4, 1>(float(a), float(b)); // -52 / 85 in 8bit 2's complement


	nrOfFailedTestCases += ReportTestResult(VerifyDivision<4, 0, Modular, uint8_t>("Manual Testing", true), "fixpnt<4,0,Modular,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<4, 1, Modular, uint8_t>("Manual Testing", true), "fixpnt<4,1,Modular,uint8_t>", "division");
	//	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 4, Modular, uint8_t>("Manual Testing", true), "fixpnt<8,4,Modular,uint8_t>", "division");


#if STRESS_TESTING

	// manual exhaustive test
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<4, 0, Modular, uint8_t>("Manual Testing", true), "fixpnt<4,0,Modular,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<4, 1, Modular, uint8_t>("Manual Testing", true), "fixpnt<4,1,Modular,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<4, 2, Modular, uint8_t>("Manual Testing", true), "fixpnt<4,2,Modular,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<4, 3, Modular, uint8_t>("Manual Testing", true), "fixpnt<4,3,Modular,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<4, 4, Modular, uint8_t>("Manual Testing", true), "fixpnt<4,4,Modular,uint8_t>", "division");

#endif

	nrOfFailedTestCases = 0; // ignore any failures in MANUAL mode
#else
	bool bReportIndividualTestCases = false;

	cout << "Fixed-point modular division validation" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 0, Modular, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,0,Modular,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 1, Modular, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,1,Modular,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 2, Modular, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,2,Modular,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 3, Modular, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,3,Modular,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 4, Modular, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,4,Modular,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 5, Modular, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,5,Modular,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 6, Modular, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,6,Modular,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 7, Modular, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,7,Modular,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 8, Modular, uint8_t>(tag, bReportIndividualTestCases), "fixpnt<8,8,Modular,uint8_t>", "division");

#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::unum::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::unum::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught fixpnt internal exception: " << err.what() << std::endl;
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
