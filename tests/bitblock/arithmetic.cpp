// arithmetic.cpp :  test suite for bitblock-based arithmetic operators
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 1
#undef BITBLOCK_ROUND_TIES_AWAY_FROM_ZERO
#undef BITBLOCK_ROUND_TIES_TO_ZERO
#include <universal/internal/bitblock/bitblock.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/bitblock_test_suite.hpp>

int Conversions() {
	using namespace sw::universal::internal;
	const unsigned nbits = 33;
	int nrOfFailedTestCases = 0;
	bitblock<nbits> a, b, ref;

	std::cout << "Binary conversions" << std::endl;

	ref = convert_to_bitblock<nbits, uint64_t>(uint64_t(0x155555555));
	a = flip_sign_bit(convert_to_bitblock<nbits,uint64_t>(uint64_t(0x55555555)));
	nrOfFailedTestCases += (a != ref ? 1 : 0);

	b = convert_to_bitblock<nbits,uint64_t>(uint64_t(0x5));

	std::cout << "1's complement of a = " << ones_complement(a) << std::endl;
	ref = convert_to_bitblock<nbits, uint64_t>(uint64_t(0xAAAAAAAA));
	nrOfFailedTestCases += (ones_complement(a) != ref ? 1 : 0);
	std::cout << "1's complement of b = " << ones_complement(b) << std::endl;
	ref = convert_to_bitblock<nbits, uint64_t>(uint64_t(0x1FFFFFFFA));
	nrOfFailedTestCases += (ones_complement(b) != ref ? 1 : 0);

	const unsigned nnbits = 9;
	bitblock<nnbits> c, ref2;
	c = convert_to_bitblock<9,int8_t>(int8_t(-128));  // this looks like -1 for a 9bit posit
	std::cout << "c                   = " << c << std::endl;
	ref2 = convert_to_bitblock<nnbits, uint64_t>(uint64_t(0x180));
	nrOfFailedTestCases += (c != ref2 ? 1 : 0);

	c = twos_complement(c);							// this looks like  1 for a 9bit posit
	std::cout << "2's Complement      = " << c << std::endl;
	ref2 = convert_to_bitblock<nnbits, uint64_t>(uint64_t(0x080));
	nrOfFailedTestCases += (c != ref2 ? 1 : 0);

	bitblock<9> d;
	d = convert_to_bitblock<9,int64_t>(int64_t(int8_t(-128)));
	std::cout << "d                   = " << d << std::endl;
	d = twos_complement(d);
	std::cout << "2's complement      = " << d << std::endl;
	std::cout << std::endl;
	nrOfFailedTestCases += (c != d ? 1 : 0);

	return nrOfFailedTestCases;
}

// ? what is this trying to test TODO
int IncrementRightAdjustedBitset() {
	using namespace sw::universal::internal;
	const unsigned nbits = 5;
	int nrOfFailedTestCases = 0;

	bitblock<nbits> r1;
	bool carry;

	std::cout << "Increments" << std::endl;
	for (unsigned i = 0; i < nbits; i++) {
		r1.reset();
		r1.set(nbits - 1 - i, true);
		carry = false;
		std::cout << "carry " << (carry ? "1" : "0") << " r1 " << r1 << " <-- input" << std::endl;
		carry = increment_unsigned(r1, i);
		std::cout << "carry " << (carry ? "1" : "0") << " r1 " << r1 << " <-- result" << std::endl;
	}

	return nrOfFailedTestCases;
}

template<unsigned src_size, unsigned tgt_size>
int VerifyCopyInto(bool bReportIndividualTestCases = false) {
	using namespace sw::universal::internal;
	int nrOfFailedTestCases = 0;

	bitblock<src_size> operand;
	bitblock<tgt_size> addend;
	bitblock<tgt_size> reference;
	
	// use a programmatic pattern of alternating bits
	// so it is easy to spot any differences
	for (unsigned i = 0; i < src_size; i = i + 2) {
		reference.set(i, true);
		operand.set(i, true);
	}

	for (unsigned i = 0; i < tgt_size - src_size + 1; i++) {
		copy_into<src_size, tgt_size>(operand, i, addend);

		if (reference != addend) {
			nrOfFailedTestCases++;
			if (bReportIndividualTestCases) std::cout << "FAIL operand : " << operand << " at i=" << i << " result   : " << addend << " reference: " << reference << std::endl;
		}
		else {
			if (bReportIndividualTestCases) std::cout << "PASS operand : " << operand << " at i=" << i << " result   : " << addend << " reference: " << reference << std::endl;
		}


		reference <<= 1; // each time around the loop, shift left by 1	
	}

	return nrOfFailedTestCases;
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::internal;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Bitblock arithmetic operation failed";

#if MANUAL_TESTING
	const unsigned nbits = 8;
	bitblock<nbits> a = convert_to_bitblock<nbits, uint32_t>(55);
	bitblock<nbits> b = convert_to_bitblock<nbits, uint32_t>(5);
	bitblock<nbits> r = convert_to_bitblock<nbits, uint32_t>(11);
	bitblock<nbits+1> sum, diff;
	bool borrow = subtract_unsigned(a, b, diff);
	cout << diff << " borrow " << borrow << endl;
	bool carry = add_unsigned(a, twos_complement(b), diff);
	cout << diff << " carry  " << carry << endl;
	bitblock<2 * nbits> mul;
	multiply_unsigned(a, b, mul);
	cout << "mul " << mul << endl;
	cout << "a   " << a << endl;
	cout << "b   " << b << endl;
	cout << "ref " << r << endl;
	bitblock<2*nbits> div;
	integer_divide_unsigned(a, b, div);
	cout << "div " << div << endl;

	constexpr unsigned result_size = 2 * nbits + 3;
	bitblock<result_size> div_with_fraction;
	a = convert_to_bitblock<nbits, uint32_t>(0x80);  // representing 1.0000000
	b = convert_to_bitblock<nbits, uint32_t>(0xA0);  // representing 1.0100000
	divide_with_fraction(a, b, div_with_fraction);
	cout << "a      " << a << endl;
	cout << "b      " << b << endl;
	cout << "div with fraction " << div_with_fraction << endl;
	// radix point comes out at at result_size - operand_size
	div_with_fraction <<= result_size - nbits;
	cout << "result " << div_with_fraction << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyBitsetAddition<3>(true), "bitblock<3>", "+");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetSubtraction<3>(true), "bitblock<3>", "-");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetMultiplication<3>(true), "bitblock<3>", "*");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetDivision<3>(true), "bitblock<3>", "/");

#else

	std::cout << "Test of operators on bitblocks\n";
	nrOfFailedTestCases += Conversions();

	std::cout << "Register management\n";
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<3, 8>(bReportIndividualTestCases),   "bitblock<  5>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<4, 8>(bReportIndividualTestCases),   "bitblock<  8>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<8, 16>(bReportIndividualTestCases),  "bitblock< 16>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<8, 24>(bReportIndividualTestCases),  "bitblock< 24>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<8, 32>(bReportIndividualTestCases),  "bitblock< 32>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<8, 48>(bReportIndividualTestCases),  "bitblock< 48>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<8, 64>(bReportIndividualTestCases),  "bitblock< 64>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<8, 128>(bReportIndividualTestCases), "bitblock<128>", "copyInto");

	std::cout << "Arithmetic: addition\n";
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetAddition<3>(bReportIndividualTestCases), "bitblock<3>", "+");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetAddition<4>(bReportIndividualTestCases), "bitblock<4>", "+");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetAddition<5>(bReportIndividualTestCases), "bitblock<5>", "+");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetAddition<6>(bReportIndividualTestCases), "bitblock<6>", "+");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetAddition<7>(bReportIndividualTestCases), "bitblock<7>", "+");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetAddition<8>(bReportIndividualTestCases), "bitblock<8>", "+");

	std::cout << "Arithmetic: subtraction\n";
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetSubtraction<3>(bReportIndividualTestCases), "bitblock<3>", "-");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetSubtraction<4>(bReportIndividualTestCases), "bitblock<4>", "-");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetSubtraction<5>(bReportIndividualTestCases), "bitblock<5>", "-");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetSubtraction<6>(bReportIndividualTestCases), "bitblock<6>", "-");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetSubtraction<7>(bReportIndividualTestCases), "bitblock<7>", "-");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetSubtraction<8>(bReportIndividualTestCases), "bitblock<8>", "-");

	std::cout << "Arithmetic: multiplication\n";
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetMultiplication<3>(bReportIndividualTestCases), "bitblock<3>", "*");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetMultiplication<4>(bReportIndividualTestCases), "bitblock<4>", "*");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetMultiplication<5>(bReportIndividualTestCases), "bitblock<5>", "*");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetMultiplication<6>(bReportIndividualTestCases), "bitblock<6>", "*");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetMultiplication<7>(bReportIndividualTestCases), "bitblock<7>", "*");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetMultiplication<8>(bReportIndividualTestCases), "bitblock<8>", "*");

	std::cout << "Arithmetic: division\n";
	bitblock<8> a, b;
	bitblock<16> c;
	try {
		integer_divide_unsigned(a, b, c); // divide by zero
	}
	catch (const bitblock_divide_by_zero& e) {
		std::cerr << "Properly caught exception: " << e.what() << std::endl;
	}
	catch (...) {
		std::cerr << "Why can't I catch this specific exception type?" << std::endl;
	}

	nrOfFailedTestCases += ReportTestResult(VerifyBitsetDivision<3>(bReportIndividualTestCases), "bitblock<3>", "/");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetDivision<4>(bReportIndividualTestCases), "bitblock<4>", "/");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetDivision<5>(bReportIndividualTestCases), "bitblock<5>", "/");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetDivision<6>(bReportIndividualTestCases), "bitblock<6>", "/");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetDivision<7>(bReportIndividualTestCases), "bitblock<7>", "/");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetDivision<8>(bReportIndividualTestCases), "bitblock<8>", "/");

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyBitsetAddition<16>(bReportIndividualTestCases), "bitblock<8>", "+");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetSubtraction<16>(bReportIndividualTestCases), "bitblock<8>", "-");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetMultiplication<16>(bReportIndividualTestCases), "bitblock<8>", "*");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsetDivision<16>(bReportIndividualTestCases), "bitblock<8>", "/");

#endif // STRESS_TESTING

#endif // MANUAL_TESTING

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
