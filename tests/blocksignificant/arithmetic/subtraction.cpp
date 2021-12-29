// subtraction.cpp: functional tests for blocksignificant subtraction
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>
#include <iomanip>

#include <universal/native/integers.hpp> // for to_binary(int)
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blocksignificant/blocksignificant.hpp>
#include <universal/verification/test_suite.hpp>


// enumerate all addition cases for an blocksignificant configuration
template<typename blocksignificantConfiguration>
int VerifyBlockSignificantSubtraction(bool reportTestCases) {
	constexpr size_t nbits = blocksignificantConfiguration::nbits;
	using BlockType = typename blocksignificantConfiguration::BlockType;

	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	using namespace sw::universal;

//	cout << endl;
//	cout << "blocksignificant<" << nbits << ',' << typeid(BlockType).name() << '>' << endl;

	int nrOfFailedTests = 0;

	blocksignificant<nbits, BlockType> a, b, c;
	blockbinary<nbits, BlockType> aref, bref, cref, refResult;
	for (size_t i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		aref.setbits(i);
		for (size_t j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			bref.setbits(j);
			cref = aref - bref;
			c.sub(a, b);
			for (size_t k = 0; k < blockbinary<nbits, BlockType>::nrBlocks; ++k) {
				refResult.setblock(k, c.block(k));
			}
			if (refResult != cref) {
				nrOfFailedTests++;
				if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "-", a, b, c, cref);
			}
			else {
				// if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "-", a, b, c, cref);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
//		if (i % 1024 == 0) cout << '.'; /// if you enable this, put the endl back
	}
//	cout << endl;
	return nrOfFailedTests;
}

// generate specific test case that you can trace with the trace conditions in blocksignificant
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, typename BlockType>
void GenerateTestCase(const sw::universal::blocksignificant<nbits, BlockType>& lhs, const sw::universal::blocksignificant <nbits, BlockType>& rhs) {
	using namespace sw::universal;

	blocksignificant<nbits, BlockType> a, b, c;

	a = lhs;
	b = rhs;
	c.sub(a, b);

	double _a, _b, _c;
	_a = double(a);
	_b = double(b);
	_c = _a - _b;

	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << lhs << " - " << std::setw(nbits) << rhs 
		<< " = " << std::setw(nbits) << c << '\n';
	std::cout << std::setw(nbits) << _a << " - " << std::setw(nbits) << _b 
		<< " = " << std::setw(nbits) << _c << '\n';
	std::cout << to_binary(a) << " - " << to_binary(b) 
		<< " = " << to_binary(c) << " (reference: " << _c << ")   " << '\n';
	double cref = double(c);
	std::cout << (_c == cref ? "PASS" : "FAIL") << '\n' << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
}

void GenerateMaxValues() {
	unsigned max = (uint64_t(1) << 8) - 1;
	std::cout << "max = " << max << std::endl;
	max = (uint64_t(1) << 16) - 1;
	std::cout << "max = " << max << std::endl;
	max = (uint64_t(1) << 32) - 1;
	std::cout << "max = " << max << std::endl;
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
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {

	using namespace sw::universal;
	
	std::string test_suite  = "blocksignificant subtraction validation";
	std::string test_tag    = "blocksignificant subtraction";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug
	{
		blocksignificant<8, uint32_t> a, b;
		a.set_raw_bits(0x40);
		b.set_raw_bits(0x41);
		GenerateTestCase(a, b);
	}

	blocksignificant<12, uint8_t> a, b;
	a.set_raw_bits(0xfff);
	b = twosComplement(a);
	cout << to_hex(a) << ' ' << to_hex(b) << ' ' << to_hex(twosComplement(b)) << endl;

	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant<4, uint8_t> >(true),  "blocksignificant<4, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant<4, uint16_t> >(true), "blocksignificant<4, uint16_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant<4, uint32_t> >(true), "blocksignificant<4, uint32_t>", "subtraction");
//	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant<4, uint64_t> >(true), "blocksignificant<4, uint64_t>", "subtraction");

	nrOfFailedTestCases = (reportTestCases ? 0 : -1);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant< 4, uint8_t> >(reportTestCases),  "blocksignificant< 4, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant< 4, uint16_t> >(reportTestCases), "blocksignificant< 4, uint16_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant< 4, uint32_t> >(reportTestCases), "blocksignificant< 4, uint32_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant< 4, uint64_t> >(reportTestCases), "blocksignificant< 4, uint64_t>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant< 8, uint8_t> >(reportTestCases),  "blocksignificant< 8, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant< 8, uint16_t> >(reportTestCases), "blocksignificant< 8, uint16_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant< 8, uint32_t> >(reportTestCases), "blocksignificant< 8, uint32_t>", "subtraction");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant< 9, uint8_t> >(reportTestCases),  "blocksignificant< 9, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant< 9, uint16_t> >(reportTestCases), "blocksignificant< 9, uint16_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant< 9, uint32_t> >(reportTestCases), "blocksignificant< 9, uint32_t>", "subtraction");

	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant<10, uint8_t> >(reportTestCases),  "blocksignificant<10, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant<10, uint16_t> >(reportTestCases), "blocksignificant<10, uint16_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant<10, uint32_t> >(reportTestCases), "blocksignificant<10, uint32_t>", "subtraction");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant<11, uint8_t> >(reportTestCases),  "blocksignificant<11, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant<11, uint16_t> >(reportTestCases), "blocksignificant<11, uint16_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant<11, uint32_t> >(reportTestCases), "blocksignificant<11, uint32_t>", "subtraction");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant<12, uint8_t> >(reportTestCases),  "blocksignificant<12, uint8_t >", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant<12, uint16_t> >(reportTestCases), "blocksignificant<12, uint16_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult(VerifyBlockSignificantSubtraction< blocksignificant<12, uint32_t> >(reportTestCases), "blocksignificant<12, uint32_t>", "subtraction");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
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
