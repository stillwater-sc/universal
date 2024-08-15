// sat_division.cpp: test suite runner for arbitrary configuration fixed-point saturating division
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <typeinfo>

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/native/integers.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/verification/fixpnt_test_suite.hpp>

// unrounded multiplication, returns a blockbinary that is of size 2*nbits
// using nbits modulo arithmetic with final sign
template<unsigned nbits, typename BlockType>
inline sw::universal::blockbinary<2 * nbits, BlockType> unrounded_mul(const sw::universal::blockbinary<nbits, BlockType>& a, const sw::universal::blockbinary<nbits, BlockType>& b) {
	using namespace sw::universal;
	blockbinary<2 * nbits, BlockType> result(0);
	if (a.iszero() || b.iszero()) return result;

	// compute the result
	bool result_sign = a.sign() ^ b.sign();
	// normalize both arguments to positive in new size
	blockbinary<nbits + 1, BlockType> a_new(a); // TODO optimize: now create a, create _a.bb, copy, destroy _a.bb_copy
	blockbinary<nbits + 1, BlockType> b_new(b);
	if (a.sign()) a_new.twosComplement();
	if (b.sign()) b_new.twosComplement();
	blockbinary<2 * nbits, BlockType> multiplicant(b_new);

	std::cout << "    " << a_new << " * " << b_new << std::endl;
	std::cout << std::setw(3) << 0 << ' ' << multiplicant << ' ' << result << std::endl;

	for (unsigned i = 0; i < (nbits + 1); ++i) {
		if (a_new.at(i)) {
			result += multiplicant;  // if multiplicant is not the same size as result, the assignment will get sign-extended if the MSB is true, this is not correct because we are assuming unsigned binaries in this loop
		}
		multiplicant <<= 1;
		std::cout << std::setw(3) << i << ' ' << multiplicant << ' ' << result << std::endl;

	}
	if (result_sign) result.twosComplement();

	std::cout << "fnl " << result << std::endl;
	return result;
}


// unrounded division, returns a blockbinary that is of size 2*nbits
template<unsigned nbits, unsigned roundingBits, typename BlockType>
inline sw::universal::blockbinary<2 * nbits + roundingBits, BlockType> unrounded_div(const sw::universal::blockbinary<nbits, BlockType>& a, const sw::universal::blockbinary<nbits, BlockType>& b, sw::universal::blockbinary<roundingBits, BlockType>& r) {
	using namespace sw::universal;

	if (b.iszero()) {
		// division by zero
		throw "urdiv divide by zero";
	}
	// generate the absolute values to do long division 
	// 2's complement special case -max requires an signed int that is 1 bit bigger to represent abs()
	bool a_sign = a.sign();
	bool b_sign = b.sign();
	//bool result_negative = (a_sign ^ b_sign);

	// normalize both arguments to positive in new size
	blockbinary<nbits + 1, BlockType> a_new(a); // TODO optimize: now create a, create _a.bb, copy, destroy _a.bb_copy
	blockbinary<nbits + 1, BlockType> b_new(b);
	if (a_sign) a_new.twosComplement();
	if (b_sign) b_new.twosComplement();

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
			quotient.setbit(static_cast<unsigned>(i));
		}
		else {
			quotient.setbit(static_cast<unsigned>(i), false);
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
template<unsigned nbits, unsigned rbits, typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	Ty ref;
	sw::universal::fixpnt<nbits, rbits, sw::universal::Saturate> a, b, cref, result;
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

template<unsigned nbits, unsigned rbits>
void GenerateValueTable() {
	using namespace sw::universal;
	unsigned NR_VALUES = (1 << nbits);

	fixpnt<nbits, rbits> a;
	std::cout << "Fixed-point type: " << typeid(a).name() << '\n';

	for (unsigned i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		std::cout << to_binary(i,false, nbits) << " : " << to_binary(a) << " = " << std::setw(10) << a << '\n';
	}
}

template<unsigned nbits, unsigned rbits>
void GenerateComparison(unsigned a_bits, unsigned b_bits) {
	using namespace sw::universal;

	fixpnt<nbits, rbits> a, b, c;
	a.setbits(a_bits);
	b.setbits(b_bits);
	c = a * b;
	float fa = float(a);
	float fb = float(b);
	float fc = fa * fb;

	std::cout << "fixpnt: " << a << " * " << b << " = " << c << " reference: " << fixpnt<nbits, rbits>(fc) << '\n';
	std::cout << "float : " << fa << " * " << fb << " = " << fc << '\n';

	{
		std::cout << "multiplication trace\n";

		blockbinary<2 * nbits> cc = unrounded_mul(a.bits(), b.bits());
		bool roundUp = cc.roundingMode(rbits);
		cc >>= rbits;
		if (roundUp) ++cc;
		fixpnt<nbits, rbits> result; result = cc; // select the lower nbits of the result
		std::cout << "final result: " << result << '\n';
	}

	std::cout << "fixpnt: " << c << " / " << a << " = " << c / a << " reference: " << fixpnt<nbits, rbits>(fc / fa) << '\n';
	std::cout << "fixpnt: " << c << " / " << b << " = " << c / b << " reference: " << fixpnt<nbits, rbits>(fc / fb) << '\n';
	std::cout << "float : " << fc << " / " << fa << " = " << fc / fa << '\n';
	std::cout << "float : " << fc << " / " << fb << " = " << fc / fb << '\n';

	{
		std::cout << "division trace\n";
		constexpr unsigned roundingDecisionBits = 4; // guard, round, and 2 sticky bits
		blockbinary<2 * nbits + roundingDecisionBits> unrounded;

		{
			std::cout << "----------------------------------------------\n";
			std::cout << c << " / " << b << '\n';

			blockbinary<roundingDecisionBits> roundingBits;
			unrounded = unrounded_div(c.bits(), b.bits(), roundingBits);
			std::cout << c.bits() << " / " << b.bits() << " = " << unrounded << " rounding bits " << roundingBits;
			bool roundUp = unrounded.roundingMode(rbits + roundingDecisionBits);
			unrounded >>= rbits + nbits + roundingDecisionBits - 1;
			if (roundUp) ++unrounded;
			std::cout << " rounded " << unrounded << '\n';
			fixpnt<nbits, rbits> result; result = unrounded; // select the lower nbits of the result
			std::cout << "final result: " << to_binary(result) << " : " << result << '\n';
		}

		{
			std::cout << "----------------------------------------------\n";
			std::cout << c << " / " << a << '\n';

			blockbinary<roundingDecisionBits> roundingBits;
			unrounded = unrounded_div(c.bits(), a.bits(), roundingBits);
			std::cout << c.bits() << " / " << a.bits() << " = " << unrounded << " rounding bits " << roundingBits;
			bool roundUp = unrounded.roundingMode(rbits + roundingDecisionBits);
			unrounded >>= rbits + nbits + roundingDecisionBits - 1;
			if (roundUp) ++unrounded;
			std::cout << " rounded " << unrounded << '\n';
			fixpnt<nbits, rbits> result; result = unrounded; // select the lower nbits of the result
			std::cout << "final result: " << to_binary(result) << " : " << result << '\n';
		}
	}
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
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "fixed-point saturating division ";
	std::string test_tag    = "saturating division";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	constexpr unsigned nbits = 4;
	constexpr unsigned rbits = 1;

	GenerateValueTable<nbits, rbits>();

	GenerateComparison<nbits, rbits>(0x3, 0x4); // 0110 and 0100 in 4bit formats
	GenerateComparison<nbits, rbits>(0x4, 0x1); // 010.0 / 000.1 = 2 / 0.5 = 4 = 100.0 = -4

	// generate individual testcases to hand trace/debug
	GenerateTestCase<4, 1>(3.0f, 1.5f); 

	std::cout << "Saturated fixpnt division not implemented yet\n";
	nrOfFailedTestCases = 1;
	// nrOfFailedTestCases += ReportTestResult(VerifyDivision<4, 0, Saturating, uint8_t>(reportTestCases), "fixpnt<4,0,Saturating,uint8_t>", test_tag);
	// nrOfFailedTestCases += ReportTestResult(VerifyDivision<4, 1, Saturating, uint8_t>(reportTestCases), "fixpnt<4,1,Saturating,uint8_t>", test_tag);
	
	// a stress test
	// nrOfFailedTestCases += ReportTestResult(VerifyDivision<8, 4, Saturating, uint8_t>(reportTestCases), "fixpnt<8,4,Saturating,uint8_t>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 4, 0, Saturating, uint8_t>(reportTestCases), "fixpnt< 4, 0,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 4, 1, Saturating, uint8_t>(reportTestCases), "fixpnt< 4, 1,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 4, 2, Saturating, uint8_t>(reportTestCases), "fixpnt< 4, 2,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 4, 3, Saturating, uint8_t>(reportTestCases), "fixpnt< 4, 3,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 4, 4, Saturating, uint8_t>(reportTestCases), "fixpnt< 4, 4,Saturating,uint8_t>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 8, 0, Saturating, uint8_t>(reportTestCases), "fixpnt< 8, 0,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 8, 1, Saturating, uint8_t>(reportTestCases), "fixpnt< 8, 1,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 8, 2, Saturating, uint8_t>(reportTestCases), "fixpnt< 8, 2,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 8, 3, Saturating, uint8_t>(reportTestCases), "fixpnt< 8, 3,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 8, 4, Saturating, uint8_t>(reportTestCases), "fixpnt< 8, 4,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 8, 5, Saturating, uint8_t>(reportTestCases), "fixpnt< 8, 5,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 8, 6, Saturating, uint8_t>(reportTestCases), "fixpnt< 8, 6,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 8, 7, Saturating, uint8_t>(reportTestCases), "fixpnt< 8, 7,Saturating,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDivision< 8, 8, Saturating, uint8_t>(reportTestCases), "fixpnt< 8, 8,Saturating,uint8_t>", test_tag);
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
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_internal_exception& err) {
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
