// block_rem.cpp: functional tests for block binary number remainder
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>
// minimum set of include files to reflect source code dependencies
#include "universal/blockbin/blockbinary.hpp"
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"
#include "../utils/blockbinary_helpers.hpp"


// generic division test generator
template<typename Scalar>
void GenerateRemTest(const Scalar& x, const Scalar& y, Scalar& z) {
	using namespace sw::unum;
	z = x % y;
	std::cout << typeid(Scalar).name() << ": " << to_binary(x) << " % " << to_binary(y) << " = " << to_binary(z) << std::endl;
}

// enumerate all multiplication cases for an blockbinary<nbits,BlockType> configuration
template<size_t nbits, typename BlockType = uint8_t>
int VerifyRemainder(std::string tag, bool bReportIndividualTestCases) {
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	using namespace std;
	using namespace sw::unum;

	cout << endl;
	cout << "blockbinary<" << nbits << ',' << typeid(BlockType).name() << '>' << endl;

	bool bReportOverflowCondition = false;
	int nrOfFailedTests = 0;
	int nrOfOverflows = 0;   // ref > maxpos
	int nrOfUnderflows = 0;  // ref < maxneg
	blockbinary<nbits, BlockType> a, b, result, refResult;
	int64_t aref, bref, cref;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.set_raw_bits(i);
		aref = int64_t(a.to_long_long()); // cast to long long is reasonable constraint for exhaustive test
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.set_raw_bits(j);
			bref = int64_t(b.to_long_long()); // cast to long long is reasonable constraint for exhaustive test
			result = a % b;
		
			if (bref == 0) continue;
			cref = aref % bref;

			if (cref < -(1 << (nbits - 1))) {
				if (bReportOverflowCondition) cout << setw(5) << aref << " % " << setw(5) << bref << " = " << setw(5) << cref << " : ";
				if (bReportOverflowCondition) cout << "underflow: " << setw(5) << cref << " < " << setw(5) << -(1 << (nbits - 1)) << "(maxneg) assigned value = " << setw(5) << result.to_long_long() << " " << setw(5) << to_hex(result) << " vs " << to_binary(cref, 12) << endl;
				++nrOfUnderflows;
			}
			else if (cref > ((1 << (nbits - 1)) - 1)) {
				if (bReportOverflowCondition) cout << setw(5) << aref << " % " << setw(5) << bref << " = " << setw(5) << cref << " : ";
				if (bReportOverflowCondition) cout << "overflow: " << setw(5) << cref << " > " << setw(5) << (1 << (nbits - 1)) - 1 << "(maxpos) assigned value = " << setw(5) << result.to_long_long() << " " << setw(5) << to_hex(result) << " vs " << to_binary(cref, 12) << endl;
				++nrOfOverflows;
			}

			refResult.set_raw_bits(cref);
			if (result != refResult) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "%", a, b, result, cref);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "%", a, b, result, cref);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
		//		if (i % 1024 == 0) std::cout << '.';
	}
	cout << "Total State Space: " << setw(10) << NR_VALUES * NR_VALUES << " Overflows: " << setw(10) << nrOfOverflows << " Underflows " << setw(10) << nrOfUnderflows << endl;
	return nrOfFailedTests;
}

template<size_t nbits, typename BlockType = uint8_t>
void TestMostSignificantBit() {
	using namespace std;
	using namespace sw::unum;
	blockbinary<nbits, BlockType> a;
	cout << to_binary(a) << ' ' << a.msb() << endl;
	a = 1;
	for (size_t i = 0; i < nbits; ++i) {
		cout << to_binary(a) << ' ' << a.msb() << endl;
		a <<= 1;
	}
}
// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "block binary division: ";

#if MANUAL_TESTING

//	TestMostSignificantBit<27, uint8_t>();
//	TestMostSignificantBit<27, uint16_t>();
//	TestMostSignificantBit<33, uint32_t>();

	int ia, ib, ic;
	blockbinary<4> a, b, c, d;
	a = -8;
	b = 3;
	c = a % b;
	cout << c.to_long_long() << " " << to_binary(c) << endl;
	d.set_raw_bits(-8 % 3);
	cout << d.to_long_long() << " " << to_binary(d) << endl;



	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<4, uint8_t>(tag, bReportIndividualTestCases), "blockbinary<4>", "division");


#if STRESS_TESTING

#endif

#else

	cout << "block binary division validation" << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<4, uint8_t>(tag, bReportIndividualTestCases), "blockbinary<4,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<5, uint8_t>(tag, bReportIndividualTestCases), "blockbinary<5,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<6, uint8_t>(tag, bReportIndividualTestCases), "blockbinary<6,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<7, uint8_t>(tag, bReportIndividualTestCases), "blockbinary<7,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<8, uint8_t>(tag, bReportIndividualTestCases), "blockbinary<8,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<9, uint8_t>(tag, bReportIndividualTestCases), "blockbinary<9,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<10, uint8_t>(tag, bReportIndividualTestCases), "blockbinary<10,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<12, uint8_t>(tag, bReportIndividualTestCases), "blockbinary<12,uint8_t>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<9, uint16_t>(tag, bReportIndividualTestCases), "blockbinary<9,uint16_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<11, uint16_t>(tag, bReportIndividualTestCases), "blockbinary<11,uint16_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<13, uint16_t>(tag, bReportIndividualTestCases), "blockbinary<13,uint16_t>", "division");

	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<12, uint32_t>(tag, bReportIndividualTestCases), "blockbinary<12,uint32_t>", "division");

#if STRESS_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<12, uint8_t>(tag, bReportIndividualTestCases), "blockbinary<12,uint8_t>", "division");
	nrOfFailedTestCases += ReportTestResult(VerifyRemainder<14, uint8_t>(tag, bReportIndividualTestCases), "blockbinary<14,uint8_t>", "division");


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
