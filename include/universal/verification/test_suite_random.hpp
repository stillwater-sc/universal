#pragma once
//  posit_test_randoms.hpp : posit verification functions based on random operand generation testing
// Needs to be included after posit type is declared.
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>

#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/test_reporters.hpp>

namespace sw::universal {

	///////////////////////// Randomized Test Case Generation ////////////////////////

	// for testing posit configs that are nbits > 14-15, we need a more efficient approach.
	// One simple, brute force approach is to generate randoms.
	//
	// A more white box approach is to focus on the testcases 
	// where something special happens in the posit arithmetic, such as rounding,
	// or the geometric rounding and inward projections.

	// operation opcodes
	const int OPCODE_NOP   =  0;
	const int OPCODE_ADD   =  1;
	const int OPCODE_SUB   =  2;
	const int OPCODE_MUL   =  3;
	const int OPCODE_DIV   =  4;
	const int OPCODE_IPA   =  5;         // in-place addition
	const int OPCODE_IPS   =  6;
	const int OPCODE_IPM   =  7;
	const int OPCODE_IPD   =  8;
	// elementary functions with one operand
	const int OPCODE_SQRT  = 20;
	const int OPCODE_EXP   = 21;
	const int OPCODE_EXP2  = 22;
	const int OPCODE_LOG   = 23;
	const int OPCODE_LOG2  = 24;
	const int OPCODE_LOG10 = 25;
	const int OPCODE_SIN   = 26;
	const int OPCODE_COS   = 27;
	const int OPCODE_TAN   = 28;
	const int OPCODE_ASIN  = 29;
	const int OPCODE_ACOS  = 30;
	const int OPCODE_ATAN  = 31;
	const int OPCODE_SINH  = 32;
	const int OPCODE_COSH  = 33;
	const int OPCODE_TANH  = 34;
	const int OPCODE_ASINH = 35;
	const int OPCODE_ACOSH = 36;
	const int OPCODE_ATANH = 37;
	// elementary functions with two operands
	const int OPCODE_POW   = 50;
	const int OPCODE_RAN   = 60;

	// Execute a binary operator
	template<typename TestType>
	void executeBinary(int opcode, 
		double da, double db, 
		const TestType& testa, const TestType& testb, 
		TestType& testresult, TestType& testref) {
		double reference = 0.0;
		switch (opcode) {
		case OPCODE_ADD:
			testresult = testa + testb;
			reference = da + db;
			break;
		case OPCODE_SUB:
			testresult = testa - testb;
			reference = da - db;
			break;
		case OPCODE_MUL:
			testresult = testa * testb;
			reference = da * db;
			break;
		case OPCODE_DIV:
			testresult = testa / testb;
			reference = da / db;
			break;
		case OPCODE_IPA:
			testresult = testa;
			testresult += testb;
			reference = da + db;
			break;
		case OPCODE_IPS:
			testresult = testa;
			testresult -= testb;
			reference = da - db;
			break;
		case OPCODE_IPM:
			testresult = testa;
			testresult *= testb;
			reference = da * db;
			break;
		case OPCODE_IPD:
			testresult = testa;
			testresult /= testb;
			reference = da / db;
			break;
		case OPCODE_POW:
			testresult = sw::universal::pow(testa, testb);
			reference = std::pow(da, db);
			break;
		case OPCODE_NOP:
		default:
			std::cerr << "Unsupported unary operator: operation ignored\n";
			break;
		}
		testref = reference;
	}

	// Execute a unary operator
	template<typename TestType>
	void executeUnary(int opcode, double da, const TestType& testa, TestType& testref, TestType& testresult, double dminpos) {
		double reference = 0.0;
		switch (opcode) {
		case OPCODE_SQRT:
			testresult = sw::universal::sqrt(testa);
			reference = std::sqrt(da);
			break;
		case OPCODE_EXP:
			testresult = sw::universal::exp(testa);
			reference = std::exp(da);
			if (0.0 == reference) reference = dminpos;
			break;
		case OPCODE_EXP2:
			testresult = sw::universal::exp2(testa);
			reference = std::exp2(da);
			if (0.0 == reference) reference = dminpos;
			break;
		case OPCODE_LOG:
			testresult = sw::universal::log(testa);
			reference = std::log(da);
			break;
		case OPCODE_LOG2:
			testresult = sw::universal::log2(testa);
			reference = std::log2(da);
			break;
		case OPCODE_LOG10:
			testresult = sw::universal::log10(testa);
			reference = std::log10(da);
			break;
		case OPCODE_SIN:
			testresult = sw::universal::sin(testa);
			reference = std::sin(da);
			break;
		case OPCODE_COS:
			testresult = sw::universal::cos(testa);
			reference = std::cos(da);
			break;
		case OPCODE_TAN:
			testresult = sw::universal::tan(testa);
			reference = std::tan(da);
			break;
		case OPCODE_ASIN:
			testresult = sw::universal::asin(testa);
			reference = std::asin(da);
			break;
		case OPCODE_ACOS:
			testresult = sw::universal::acos(testa);
			reference = std::acos(da);
			break;
		case OPCODE_ATAN:
			testresult = sw::universal::atan(testa);
			reference = std::atan(da);
			break;
		case OPCODE_SINH:
			testresult = sw::universal::sinh(testa);
			reference = std::sinh(da);
			break;
		case OPCODE_COSH:
			testresult = sw::universal::cosh(testa);
			reference = std::cosh(da);
			break;
		case OPCODE_TANH:
			testresult = sw::universal::tanh(testa);
			reference = std::tanh(da);
			break;
		case OPCODE_ASINH:
			testresult = sw::universal::asinh(testa);
			reference = std::asinh(da);
			break;
		case OPCODE_ACOSH:
			testresult = sw::universal::acosh(testa);
			reference = std::acosh(da);
			break;
		case OPCODE_ATANH:
			testresult = sw::universal::atanh(testa);
			reference = std::atanh(da);
			break;
		case OPCODE_NOP:
		default:
			std::cerr << "Unsupported binary operator: operation ignored\n";
			break;
		}
		testref = reference;
	}

	// generate a random set of operands to test the binary operators for a posit configuration
	// Basic design is that we generate nrOfRandom posit values and store them in an operand array.
	// We will then execute the binary operator nrOfRandom combinations.
	template<typename TestType>
	int VerifyBinaryOperatorThroughRandoms(bool bReportIndividualTestCases, int opcode, uint32_t nrOfRandoms) {
		std::cerr << typeid(TestType).name() << " : ";

		std::string operation_string;
		switch (opcode) {
		case OPCODE_ADD:
			operation_string = "+";
			break;
		case OPCODE_SUB:
			operation_string = "-";
			break;
		case OPCODE_MUL:
			operation_string = "*";
			break;
		case OPCODE_DIV:
			operation_string = "/";
			break;
		case OPCODE_IPA:
			operation_string = "+=";
			break;
		case OPCODE_IPS:
			operation_string = "-=";
			break;
		case OPCODE_IPM:
			operation_string = "*=";
			break;
		case OPCODE_IPD:
			operation_string = "/=";
			break;
		case OPCODE_POW:
			operation_string = "pow";
			break;
		case OPCODE_NOP:
		default:
			std::cerr << "Unsupported unary operator, test cancelled\n";
			return 1;
		}
		// generate a random bit pattern, which we'll assign to the TestType
		std::random_device rd;     // get a random seed from the OS entropy device
		std::mt19937_64 eng(rd()); // use the 64-bit Mersenne Twister 19937 generator and seed it with entropy.
		// define the distribution, by default it goes from 0 to MAX(unsigned long long)
		std::uniform_int_distribution<unsigned long long> distr;
		int nrOfFailedTests = 0;
		if (bReportIndividualTestCases) std::cerr << '\n';
		for (unsigned i = 1; i < nrOfRandoms; i++) {
			TestType testa, testb, testresult, testref;
			testa.setbits(distr(eng));
			testb.setbits(distr(eng));
			double da = double(testa);
			double db = double(testb);
			// in case you have numeric_limits<long double>::digits trouble... this will show that
			//std::cout << "sizeof da: " << sizeof(da) << " bits in significant " << (std::numeric_limits<long double>::digits - 1) << " value da " << da << " at index " << ia << " testa " << testa << std::endl;
			//std::cout << "sizeof db: " << sizeof(db) << " bits in significant " << (std::numeric_limits<long double>::digits - 1) << " value db " << db << " at index " << ia << " testa " << testb << std::endl;

			executeBinary(opcode, da, db, testa, testb, testresult, testref);
			// check the result
			if (testresult != testref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases) ReportBinaryArithmeticError("FAIL", operation_string, testa, testb, testresult, testref);
			}
			else {
				//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", operation_string, testa, testb, testresult, testref);
			}
		}
		return nrOfFailedTests;
	}

	// generate a random set of operands to test the binary operators for a posit configuration
	// Basic design is that we generate nrOfRandom posit values and store them in an operand array.
	// We will then execute the binary operator nrOfRandom combinations.
	// provide 		double dminpos = double(minpos<nbits, es>(pminpos));
	template<typename TestType>
	int VerifyUnaryOperatorThroughRandoms(bool bReportIndividualTestCases, int opcode, uint32_t nrOfRandoms, double dminpos) {
		std::string operation_string;
		bool sqrtOperator = false;  // we need to filter negative values from the randoms
		switch (opcode) {
		default:
		case OPCODE_NOP:
			return 0;
		case OPCODE_ADD:
		case OPCODE_SUB:
		case OPCODE_MUL:
		case OPCODE_DIV:
		case OPCODE_IPA:
		case OPCODE_IPS:
		case OPCODE_IPM:
		case OPCODE_IPD:
			std::cerr << "Unsupported binary operator, test cancelled\n";
			return 1;
		case OPCODE_SQRT:
			operation_string = "sqrt";
			sqrtOperator = true;
			break;
		case OPCODE_EXP:
			break;
		case OPCODE_EXP2:
			break;
		case OPCODE_LOG:
			break;
		case OPCODE_LOG2:
			break;
		case OPCODE_LOG10:
			break;
		case OPCODE_SIN:
			break;
		case OPCODE_COS:
			break;
		case OPCODE_TAN:
			break;
		case OPCODE_ASIN:
			break;
		case OPCODE_ACOS:
			break;
		case OPCODE_ATAN:
			break;
		case OPCODE_SINH:
			break;
		case OPCODE_COSH:
			break;
		case OPCODE_TANH:
			break;
		case OPCODE_ASINH:
			break;
		case OPCODE_ACOSH:
			break;
		case OPCODE_ATANH:
			break;
		}
		// generate the full state space set of valid posit values
		std::random_device rd;     // get a random seed from the OS entropy device, or whatever
		std::mt19937_64 eng(rd()); // use the 64-bit Mersenne Twister 19937 generator and seed it with entropy.
		// define the distribution, by default it goes from 0 to MAX(unsigned long long)
		std::uniform_int_distribution<unsigned long long> distr;
		int nrOfFailedTests = 0;
		for (unsigned i = 1; i < nrOfRandoms; i++) {
			TestType testa, testresult, testref;
			testa.setbits(distr(eng));
			if (sqrtOperator && testa < 0) testa = -testa;
			double da = double(testa);

			executeUnary(opcode, da, testa, testref, testresult, dminpos);

			if (testresult != testref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases) ReportUnaryArithmeticError("FAIL", operation_string, testa, testresult, testref);
			}
			else {
				//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", operation_string, testa, testresult, testref);
			}
		}

		return nrOfFailedTests;
	}

	template<typename TestType>
	int Compare(long double input, const TestType& testresult, const TestType& ptarget, const TestType& pref, bool bReportIndividualTestCases) {
		int fail = 0;
		if (testresult != ptarget) {
			fail++;
			if (bReportIndividualTestCases) {
				ReportConversionError("FAIL", "=", input, (long double)(ptarget), testresult);
				std::cout << "reference   : " << pref.get() << std::endl;
				std::cout << "target bits : " << ptarget.get() << std::endl;
				std::cout << "actual bits : " << testresult.get() << std::endl;
			}
		}
		else {
			// if (bReportIndividualTestCases) ReportConversionSuccess("PASS", "=", input, reference, testresult);
		}
		return fail;
	}

} // namespace sw::universal
