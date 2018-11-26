//  multi_word.cpp :  test suite for bitblock-based multi-word operators
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include "common.hpp"
#include "../../posit/exceptions.hpp"	// TODO: remove namespace polution
#include "../../bitblock/bitblock.hpp"
#include "../tests/test_helpers.hpp"
#include "../bitblock_test_helpers.hpp"

#if defined(_MSC_VER)
// Disable warnings for Microsoft Visual Studio
//  warning C4293: '<<': shift count negative or too big, undefined behavior
// for 64 bit shifts on the mask
#pragma warning (disable : 4293)

// disable this warning: as the logic is correct
// warning C4146 : unary minus operator applied to unsigned type, result still unsigned
#pragma warning( disable : 4146)

void CheckUnsignedNegationBehavior() {
	unsigned long x = 0x7fffffff;
	cout << hex;
	cout << " +x = " << x << endl;
	cout << " -x = " << -x << endl;
	cout << dec;
	cout << " #1 = " << universal_bitset::popcnt(x) << endl;
}

void CheckMultiWordBehavior() {

	using WordT = unsigned char;

	// checking UBB_WORDS
	cout << "UBB_WORDS for key sizes\n";
	cout << "UBB_WORDS(8)    : " << NrWords<8, sizeof(WordT)>() << endl;
	cout << "UBB_WORDS(12)   : " << NrWords<12, sizeof(WordT)>() << endl;
	cout << "UBB_WORDS(16)   : " << NrWords<16, sizeof(WordT)>() << endl;
	cout << "UBB_WORDS(20)   : " << NrWords<20, sizeof(WordT)>() << endl;
	cout << "UBB_WORDS(24)   : " << NrWords<24, sizeof(WordT)>() << endl;
	cout << "UBB_WORDS(32)   : " << NrWords<32, sizeof(WordT)>() << endl;
	cout << "UBB_WORDS(40)   : " << NrWords<40, sizeof(WordT)>() << endl;
	cout << "UBB_WORDS(48)   : " << NrWords<48, sizeof(WordT)>() << endl;
	cout << "UBB_WORDS(56)   : " << NrWords<56, sizeof(WordT)>() << endl;
	cout << "UBB_WORDS(64)   : " << NrWords<64, sizeof(WordT)>() << endl;
	cout << "UBB_WORDS(80)   : " << NrWords<80, sizeof(WordT)>() << endl;
	cout << "UBB_WORDS(96)   : " << NrWords<96, sizeof(WordT)>() << endl;
	cout << "UBB_WORDS(112)  : " << NrWords<112, sizeof(WordT)>() << endl;
	cout << "UBB_WORDS(128)  : " << NrWords<128, sizeof(WordT)>() << endl;

	// this creates and masks the size of the requested number of bits Nb
	// val & ~((~static_cast<unsigned long long>(0)) << Nb)
	cout << hex << ((~static_cast<unsigned long long>(0)) << 8) << endl;
	cout << hex << ((~static_cast<unsigned long long>(0)) << 16) << endl;
	cout << hex << ((~static_cast<unsigned long long>(0)) << 32) << endl;
	cout << hex << ((~static_cast<unsigned long long>(0)) << 64) << endl;
	cout << hex << "0x" << (0x5555555555555555 & ~((~static_cast<unsigned long long>(0)) << 8)) << endl;
	cout << hex << "0x" << (0x5555555555555555 & ~((~static_cast<unsigned long long>(0)) << 16)) << endl;
	cout << hex << "0x" << (0x5555555555555555 & ~((~static_cast<unsigned long long>(0)) << 32)) << endl;
	cout << hex << "0x" << (0x5555555555555555 & ~((~static_cast<unsigned long long>(0)) << 64)) << endl;
	cout << dec;

	bitblock<8> bb008;
	bitblock<16> bb016;
	bitblock<32> bb032;
	bitblock<64> bb064;
	bitblock<128> bb128, bbu64, bbl64;

	bb008 = 0x55u;
	bb016 = 0x5555u;
	bb032 = 0x55555555ul;
	bb064 = 0x5555555555555555;
	bbl64 = 0x5555555555555555;
	bbu64 = 0x5555555555555555;
	bbu64 <<= 64;
	bb128 = bbu64;
	bb128 |= bbl64;

	cout << "bb008: " << bb008 << " size in bytes: " << sizeof(bb008) << endl;
	cout << "bb016: " << bb016 << " size in bytes: " << sizeof(bb016) << endl;
	cout << "bb032: " << bb032 << " size in bytes: " << sizeof(bb032) << endl;
	cout << "bb064: " << bb064 << " size in bytes: " << sizeof(bb064) << endl;
	cout << "bb128: " << bb128 << " size in bytes: " << sizeof(bb128) << endl;
}

#endif

int Conversions() {
	using namespace sw::unum;
	const size_t nbits = 33;
	int nrOfFailedTestCases = 0;
	bitblock<nbits> a, b, ref, sum;

	std::cout << "Binary conversions" << std::endl;

	ref = convert_to_bitblock<nbits, uint64_t>(uint64_t(0x155555555));
	a = flip_sign_bit(convert_to_bitblock<nbits, uint64_t>(uint64_t(0x55555555)));
	nrOfFailedTestCases += (a != ref ? 1 : 0);

	b = convert_to_bitblock<nbits, uint64_t>(uint64_t(0x5));

	std::cout << "1's complement of a = " << to_binary(ones_complement(a)) << std::endl;
	ref = convert_to_bitblock<nbits, uint64_t>(uint64_t(0xAAAAAAAA));
	nrOfFailedTestCases += (ones_complement(a) != ref ? 1 : 0);
	std::cout << "1's complement of b = " << to_binary(ones_complement(b)) << std::endl;
	ref = convert_to_bitblock<nbits, uint64_t>(uint64_t(0x1FFFFFFFA));
	nrOfFailedTestCases += (ones_complement(b) != ref ? 1 : 0);

	const size_t nnbits = 9;
	bitblock<nnbits> c, ref2;
	c = convert_to_bitblock<9, int8_t>(int8_t(-128));  // this looks like -1 for a 9bit posit
	std::cout << "c                   = " << to_binary(c) << std::endl;
	ref2 = convert_to_bitblock<nnbits, uint64_t>(uint64_t(0x180));
	nrOfFailedTestCases += (c != ref2 ? 1 : 0);

	c = twos_complement(c);							// this looks like  1 for a 9bit posit
	std::cout << "2's Complement      = " << to_binary(c) << std::endl;
	ref2 = convert_to_bitblock<nnbits, uint64_t>(uint64_t(0x080));
	nrOfFailedTestCases += (c != ref2 ? 1 : 0);

	bitblock<9> d;
	d = convert_to_bitblock<9, int64_t>(int64_t(int8_t(-128)));
	std::cout << "d                   = " << to_binary(d) << std::endl;
	d = twos_complement(d);
	std::cout << "2's complement      = " << to_binary(d) << std::endl;
	std::cout << std::endl;
	nrOfFailedTestCases += (c != d ? 1 : 0);

	return nrOfFailedTestCases;
}

template<size_t src_size, size_t tgt_size>
int VerifyCopyInto(bool bReportIndividualTestCases = false) {
	int nrOfFailedTestCases = 0;

	sw::unum::bitblock<src_size> operand;
	sw::unum::bitblock<tgt_size> addend;
	sw::unum::bitblock<tgt_size> reference;
	
	// use a programmatic pattern of alternating bits
	// so it is easy to spot any differences
	for (size_t i = 0; i < src_size; i = i + 2) {
		reference.set(i, true);
		operand.set(i, true);
	}

	for (size_t i = 0; i < tgt_size - src_size + 1; i++) {
		sw::unum::copy_into<src_size, tgt_size>(operand, i, addend);

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

template<size_t bits_in_value, size_t bytes_in_word>
size_t NrWords() {
	return ((bits_in_value + (8 * bytes_in_word) - 1) / (8 * bytes_in_word));
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Bitblock arithmetic operation failed";

#if MANUAL_TESTING

#if defined(_MSC_VER)
	CheckUnsignedNegationBehavior();
	CheckMultiWordBehavior();
#endif

	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<3>(true), "bitblock<3>", "+");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetSubtraction<3>(true), "bitblock<3>", "-");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<3>(true), "bitblock<3>", "*");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetDivision<3>(true), "bitblock<3>", "/");
	if (bReportIndividualTestCases) {
		cout << "/n";
	}

#else

	cout << "Test of operators on bitblocks" << endl;
	nrOfFailedTestCases += Conversions();

	cout << "Register management" << endl;
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<3, 8>(bReportIndividualTestCases),   "bitblock<  5>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<4, 8>(bReportIndividualTestCases),   "bitblock<  8>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<8, 16>(bReportIndividualTestCases),  "bitblock< 16>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<8, 24>(bReportIndividualTestCases),  "bitblock< 24>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<8, 32>(bReportIndividualTestCases),  "bitblock< 32>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<8, 48>(bReportIndividualTestCases),  "bitblock< 48>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<8, 64>(bReportIndividualTestCases),  "bitblock< 64>", "copyInto");
	nrOfFailedTestCases += ReportTestResult(VerifyCopyInto<8, 128>(bReportIndividualTestCases), "bitblock<128>", "copyInto");

	cout << "Arithmetic: addition" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<3>(bReportIndividualTestCases), "bitblock<3>", "+");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<4>(bReportIndividualTestCases), "bitblock<4>", "+");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<5>(bReportIndividualTestCases), "bitblock<5>", "+");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<6>(bReportIndividualTestCases), "bitblock<6>", "+");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<7>(bReportIndividualTestCases), "bitblock<7>", "+");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<8>(bReportIndividualTestCases), "bitblock<8>", "+");

	cout << "Arithmetic: subtraction" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetSubtraction<3>(bReportIndividualTestCases), "bitblock<3>", "-");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetSubtraction<4>(bReportIndividualTestCases), "bitblock<4>", "-");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetSubtraction<5>(bReportIndividualTestCases), "bitblock<5>", "-");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetSubtraction<6>(bReportIndividualTestCases), "bitblock<6>", "-");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetSubtraction<7>(bReportIndividualTestCases), "bitblock<7>", "-");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetSubtraction<8>(bReportIndividualTestCases), "bitblock<8>", "-");

	cout << "Arithmetic: multiplication" << endl;
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<3>(bReportIndividualTestCases), "bitblock<3>", "*");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<4>(bReportIndividualTestCases), "bitblock<4>", "*");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<5>(bReportIndividualTestCases), "bitblock<5>", "*");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<6>(bReportIndividualTestCases), "bitblock<6>", "*");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<7>(bReportIndividualTestCases), "bitblock<7>", "*");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<8>(bReportIndividualTestCases), "bitblock<8>", "*");

	cout << "Arithmetic: division" << endl;
	bitblock<8> a, b;
	bitblock<16> c;
	try {
		integer_divide_unsigned(a, b, c); // divide by zero
	}
	catch (const integer_divide_by_zero& e) {
		cout << "Properly caught exception: " << e.what() << endl;
	}
	catch (...) {
		cout << "Why can't I catch this specific exception type?" << endl;
	}

	nrOfFailedTestCases += ReportTestResult(ValidateBitsetDivision<3>(bReportIndividualTestCases), "bitblock<3>", "/");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetDivision<4>(bReportIndividualTestCases), "bitblock<4>", "/");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetDivision<5>(bReportIndividualTestCases), "bitblock<5>", "/");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetDivision<6>(bReportIndividualTestCases), "bitblock<6>", "/");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetDivision<7>(bReportIndividualTestCases), "bitblock<7>", "/");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetDivision<8>(bReportIndividualTestCases), "bitblock<8>", "/");

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(ValidateBitsetAddition<16>(bReportIndividualTestCases), "bitblock<8>", "+");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetSubtraction<16>(bReportIndividualTestCases), "bitblock<8>", "-");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetMultiplication<16>(bReportIndividualTestCases), "bitblock<8>", "*");
	nrOfFailedTestCases += ReportTestResult(ValidateBitsetDivision<16>(bReportIndividualTestCases), "bitblock<8>", "/");

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
