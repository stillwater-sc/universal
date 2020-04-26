// sat_division.cpp: functional tests for arbitrary configuration fixed-point saturating division
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <typeinfo>

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1

// minimum set of include files to reflect source code dependencies
#include <universal/fixpnt/fixed_point.hpp>
// fixed-point type manipulators such as pretty printers
#include <universal/native/integers.hpp>
#include <universal/fixpnt/fixpnt_manipulators.hpp>
#include <universal/fixpnt/fixpnt_functions.hpp>
#include "../utils/fixpnt_test_suite.hpp"

// unrounded multiplication, returns a blockbinary that is of size 2*nbits
// using nbits modulo arithmetic with final sign
template<size_t nbits, typename BlockType>
inline sw::unum::blockbinary<2 * nbits, BlockType> unrounded_mul(const sw::unum::blockbinary<nbits, BlockType>& a, const sw::unum::blockbinary<nbits, BlockType>& b) {
	using namespace sw::unum;
	blockbinary<2 * nbits, BlockType> result;
	if (a.iszero() || b.iszero()) return result;

	// compute the result
	bool result_sign = a.sign() ^ b.sign();
	// normalize both arguments to positive in new size
	blockbinary<nbits + 1, BlockType> a_new(a); // TODO optimize: now create a, create _a.bb, copy, destroy _a.bb_copy
	blockbinary<nbits + 1, BlockType> b_new(b);
	if (a.sign()) a_new.twoscomplement();
	if (b.sign()) b_new.twoscomplement();
	blockbinary<2 * nbits, BlockType> multiplicant(b_new);

	std::cout << "    " << a_new << " * " << b_new << std::endl;
	std::cout << std::setw(3) << 0 << ' ' << multiplicant << ' ' << result << std::endl;

	for (size_t i = 0; i < (nbits + 1); ++i) {
		if (a_new.at(i)) {
			result += multiplicant;  // if multiplicant is not the same size as result, the assignment will get sign-extended if the MSB is true, this is not correct because we are assuming unsigned binaries in this loop
		}
		multiplicant <<= 1;
		std::cout << std::setw(3) << i << ' ' << multiplicant << ' ' << result << std::endl;

	}
	if (result_sign) result.twoscomplement();

	std::cout << "fnl " << result << std::endl;
	return result;
}


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
	blockbinary<2 * nbits + roundingBits, BlockType> decimator(a_new);
	blockbinary<2 * nbits + roundingBits, BlockType> subtractand(b_new); // prepare the subtractand
	blockbinary<2 * nbits + roundingBits, BlockType> quotient;

	int msp = nbits + roundingBits - 1; // msp = most significant position
	decimator <<= msp; // scale the decimator to the largest possible positive value

	std::cout << "  " << to_binary(decimator) << ' ' << to_binary(subtractand) << std::endl;

	int msb_b = subtractand.msb();
	int msb_a = decimator.msb();
	int shift = msb_a - msb_b;
	int scale = shift - msp;   // scale of the quotient
	subtractand <<= shift;

	std::cout << "  " << to_binary(decimator) << std::endl;
	std::cout << "- " << to_binary(subtractand) << " shift: " << shift << " scale: " << scale << " msb_a: " << msb_a << " msb_b: " << msb_b << std::endl;

	// long division
	for (int i = msb_a; i >= 0; --i) {

		if (subtractand <= decimator) {
			decimator -= subtractand;
			quotient.set(i);
		}
		else {
			quotient.reset(i);
		}
		subtractand >>= 1;

		std::cout << "  " << to_binary(decimator) << ' ' << to_binary(quotient) << std::endl;
		std::cout << "- " << to_binary(subtractand) << std::endl;

	}
	quotient <<= scale;
	r.assign(quotient); // copy the lowest bits which represent the bits on which we need to apply the rounding test
	return quotient;
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

template<size_t nbits, size_t rbits>
void GenerateValueTable() {
	using namespace std;
	using namespace sw::unum;
	size_t NR_VALUES = (1 << nbits);

	fixpnt<nbits, rbits> a;
	cout << "Fixed-point type: " << typeid(a).name() << endl;

	for (size_t i = 0; i < NR_VALUES; ++i) {
		a.set_raw_bits(i);
		cout << to_binary(i,nbits) << " : " << to_binary(a) << " = " << setw(10) << a << endl;
	}
}

template<size_t nbits, size_t rbits>
void GenerateComparison(size_t a_bits, size_t b_bits) {
	using namespace std;
	using namespace sw::unum;

	fixpnt<nbits, rbits> a, b, c;
	a.set_raw_bits(a_bits);
	b.set_raw_bits(b_bits);
	c = a * b;
	float fa = float(a);
	float fb = float(b);
	float fc = fa * fb;

	cout << "fixpnt: " << a << " * " << b << " = " << c << " reference: " << fixpnt<nbits, rbits>(fc) << endl;
	cout << "float : " << fa << " * " << fb << " = " << fc << endl;

	{
		cout << "multiplication trace\n";

		blockbinary<2 * nbits> c = unrounded_mul(a.getbb(), b.getbb());
		bool roundUp = c.roundingMode(rbits);
		c >>= rbits;
		if (roundUp) ++c;
		fixpnt<nbits, rbits> result; result = c; // select the lower nbits of the result
		cout << "final result: " << result << endl;
	}

	cout << "fixpnt: " << c << " / " << a << " = " << c / a << " reference: " << fixpnt<nbits, rbits>(fc / fa) << endl;
	cout << "fixpnt: " << c << " / " << b << " = " << c / b << " reference: " << fixpnt<nbits, rbits>(fc / fb) << endl;
	cout << "float : " << fc << " / " << fa << " = " << fc / fa << endl;
	cout << "float : " << fc << " / " << fb << " = " << fc / fb << endl;

	{
		cout << "division trace\n";

		{
			cout << "----------------------------------------------\n";
			std::cout << c << " / " << b << std::endl;

			constexpr size_t roundingDecisionBits = 4; // guard, round, and 2 sticky bits
			blockbinary<roundingDecisionBits> roundingBits;
			blockbinary<2 * nbits + roundingDecisionBits> a = unrounded_div(c.getbb(), b.getbb(), roundingBits);
			std::cout << c.getbb() << " / " << b.getbb() << " = " << a << " rounding bits " << roundingBits;
			bool roundUp = a.roundingMode(rbits + roundingDecisionBits);
			a >>= rbits + nbits + roundingDecisionBits - 1;
			if (roundUp) ++a;
			std::cout << " rounded " << a << std::endl;
			fixpnt<nbits, rbits> result; result = a; // select the lower nbits of the result
			cout << "final result: " << to_binary(result) << " : " << result << endl;
		}

		{
			cout << "----------------------------------------------\n";
			std::cout << c << " / " << a << std::endl;

			constexpr size_t roundingDecisionBits = 4; // guard, round, and 2 sticky bits
			blockbinary<roundingDecisionBits> roundingBits;
			blockbinary<2 * nbits + roundingDecisionBits> b = unrounded_div(c.getbb(), a.getbb(), roundingBits);
			std::cout << c.getbb() << " / " << a.getbb() << " = " << b << " rounding bits " << roundingBits;
			bool roundUp = b.roundingMode(rbits + roundingDecisionBits);
			b >>= rbits + nbits + roundingDecisionBits - 1;
			if (roundUp) ++b;
			std::cout << " rounded " << b << std::endl;
			fixpnt<nbits, rbits> result; result = b; // select the lower nbits of the result
			cout << "final result: " << to_binary(result) << " : " << result << endl;
		}

	}

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

	constexpr size_t nbits = 4;
	constexpr size_t rbits = 1;

	GenerateValueTable<nbits, rbits>();

	GenerateComparison<nbits, rbits>(0x3, 0x4); // 0110 and 0100 in 4bit formats
	GenerateComparison<nbits, rbits>(0x4, 0x1); // 010.0 / 000.1 = 2 / 0.5 = 4 = 100.0 = -4

	return 0;

	// generate individual testcases to hand trace/debug
	GenerateTestCase<4, 1>(3.0f, 1.5f); 

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
