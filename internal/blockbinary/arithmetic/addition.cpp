// addition.cpp: functional tests for block binary number addition
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/blockbinary_test_status.hpp>

// enumerate all addition cases for an blockbinary<nbits,BlockType> configuration
template<unsigned nbits, typename BlockType = uint8_t>
int VerifyAddition(bool bReportIndividualTestCases) {
	constexpr unsigned NR_VALUES = (unsigned(1) << nbits);
	using namespace sw::universal;
	
	std::cout << "blockbinary<" << nbits << ',' << typeid(BlockType).name() << ">\n";

	constexpr bool bReportOverflowCondition = false;

	int nrOfFailedTests = 0;
	int nrOfOverflows = 0;   // ref > maxpos
	int nrOfUnderflows = 0;  // ref < maxneg
	blockbinary<nbits, BlockType> a, b, result, refResult;
	int64_t aref, bref, cref;
	for (unsigned i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		aref = int64_t(a.to_sll()); // cast to long long is reasonable constraint for exhaustive test
		for (unsigned j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			bref = int64_t(b.to_sll()); // cast to long long is reasonable constraint for exhaustive test
			cref = aref + bref;
			result = a + b;

			if (bReportOverflowCondition) std::cout << std::setw(5) << aref << " + " << std::setw(5) << bref << " = " << std::setw(5) << cref << " : ";
			if (cref < -(1 << (nbits - 1))) {
				if (bReportOverflowCondition) std::cout << "underflow: " << std::setw(5) << cref << " < " << std::setw(5) << -(1 << (nbits - 1)) << "(maxneg) assigned value = " << std::setw(5) << result.to_sll() << " " << std::setw(5) << to_hex(result) << " vs " << to_binary(cref, false, 12) << '\n';
				++nrOfUnderflows;
			}
			else if (cref > ((1 << (nbits - 1)) - 1)) {
				if (bReportOverflowCondition) std::cout << "overflow: " << std::setw(5) << cref << " > " << std::setw(5) << (1 << (nbits - 1)) - 1 << "(maxpos) assigned value = " << std::setw(5) << result.to_sll() << " " << std::setw(5) << to_hex(result) << " vs " << to_binary(cref, false, 12) << '\n';
				++nrOfOverflows;
			}
			else {
				if (bReportOverflowCondition) std::cout << std::endl;
			}

			refResult.setbits(static_cast<uint64_t>(cref));
			if (result != refResult) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", a, b, result, cref);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, result, cref);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
//		if (i % 1024 == 0) cout << '.'; /// if you enable this, put the endl back
	}
	std::cout << "Total State Space: " << std::setw(10) << NR_VALUES*NR_VALUES 
		<< " Overflows: " << std::setw(10) << nrOfOverflows << " Underflows " << std::setw(10) << nrOfUnderflows << '\n';
	return nrOfFailedTests;
}

// generate specific test case that you can trace with the trace conditions in blockbinary
// for most bugs they are traceable with _trace_conversion and _trace_add
template<unsigned nbits, typename StorageBlockType = uint8_t>
void GenerateTestCase(int64_t lhs, int64_t rhs) {
	using namespace sw::universal;
	blockbinary<nbits, StorageBlockType> a, b, result, reference;

	a.setbits(uint64_t(lhs));
	b.setbits(uint64_t(rhs));
	result = a + b;

	long long _a, _b, _c;
	_a = (long long)a;
	_b = (long long)b;
	_c = _a + _b;

	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << _a << " + " << std::setw(nbits) << _b << " = " << std::setw(nbits) << _c << std::endl;
	std::cout << to_binary(a) << " + " << to_binary(b) << " = " << to_binary(result) << " (reference: " << _c << ")   " << std::endl;
	//	std::cout << to_hex(a) << " * " << to_hex(b) << " = " << to_hex(result) << " (reference: " << std::hex << ref << ")   ";
	reference.set_raw_bits(_c);
	std::cout << (result == reference ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
}

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc > 1) std::cout << argv[0] << std::endl; 
	
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "modular addition failed: ";

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	GenerateTestCase<18, uint8_t>(12345, 54321); // result is 66,666, thus needs 18 bits to be represented by 2's complement
	GenerateTestCase<18, uint8_t>(66666, -54321); // result is 12,345

	int maxneg = -0x20000;
	GenerateTestCase<18, uint8_t>(maxneg, -1); // result is overflow on the negative side

	GenerateTestCase<12, uint16_t>(0, 0x100);

	unsigned max = (uint64_t(1) << 8) - 1;
	std::cout << "max = " << max << std::endl;
	max = (uint64_t(1) << 16) - 1;
	std::cout << "max = " << max << std::endl;
	max = (uint64_t(1) << 32) - 1;
	std::cout << "max = " << max << std::endl;

	blockbinary<12> a, b, c;
	a = (long long)-1024;
	b = a;
	c = a + b;
	a.sign();
	cout << (a.sign() ? "neg" : "pos") << endl;
	cout << (c.sign() ? "neg" : "pos") << endl;
	cout << a.to_sll() << endl;	
	cout << c.to_sll() << endl;


	nrOfFailedTestCases += ReportTestResult(VerifyAddition<12, uint8_t>(bReportIndividualTestCases), "uint8_t<12>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<12, uint16_t>(bReportIndividualTestCases), "uint16_t<12>", "addition");


//	nrOfFailedTestCases += ReportTestResult(VerifyAddition<4, uint8_t>(true), "uint8_t<4>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyAddition<4, uint16_t>(true), "uint16_t<4>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyAddition<4, uint32_t>(true), "uint32_t<4>", "addition");
//	nrOfFailedTestCases += ReportTestResult(VerifyAddition<4, uint64_t>(true), "uint64_t<4>", "addition");


#if STRESS_TESTING

#endif

#else

	std::cout << "block addition validation\n";

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<4, uint8_t>(bReportIndividualTestCases), "blockbinary<4,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<4, uint16_t>(bReportIndividualTestCases), "blockbinary<4,uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<4, uint32_t>(bReportIndividualTestCases), "blockbinary<4,uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, uint8_t>(bReportIndividualTestCases), "blockbinary<8,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, uint16_t>(bReportIndividualTestCases), "blockbinary<8,uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<8, uint32_t>(bReportIndividualTestCases), "blockbinary<8,uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<9, uint8_t>(bReportIndividualTestCases), "blockbinary<9,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<9, uint16_t>(bReportIndividualTestCases), "blockbinary<9,uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<9, uint32_t>(bReportIndividualTestCases), "blockbinary<9,uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<10, uint8_t>(bReportIndividualTestCases), "blockbinary<10,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<10, uint16_t>(bReportIndividualTestCases), "blockbinary<10,uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<10, uint32_t>(bReportIndividualTestCases), "blockbinary<10,uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<11, uint8_t>(bReportIndividualTestCases), "blockbinary<11,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<11, uint16_t>(bReportIndividualTestCases), "blockbinary<11,uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<11, uint32_t>(bReportIndividualTestCases), "blockbinary<11,uint32_t>", "addition");

	nrOfFailedTestCases += ReportTestResult(VerifyAddition<12, uint8_t>(bReportIndividualTestCases), "blockbinary<12,uint8_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<12, uint16_t>(bReportIndividualTestCases), "blockbinary<12,uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<12, uint32_t>(bReportIndividualTestCases), "blockbinary<12,uint32_t>", "addition");

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
