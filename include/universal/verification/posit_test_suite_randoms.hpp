#pragma once
// posit_test_suite_randoms.hpp : posit verification functions based on random operand generation testing
// Needs to be included after posit type is declared.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>

#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

	//////////////////////////////////// RANDOMIZED TEST SUITE FOR LARGE POSITS ////////////////////////

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
	const int IPCODE_HYPOT = 51;
	const int OPCODE_RAN   = 60;

	// Execute a binary operator
	template<typename TestType>
	void executeBinary(int opcode, double da, double db, double& dc, const TestType& testa, const TestType& testb, TestType& testc, TestType& testref) {
		dc = 0.0;
		switch (opcode) {
		case OPCODE_ADD:
			testc = testa + testb;
			dc = da + db;
			break;
		case OPCODE_SUB:
			testc = testa - testb;
			dc = da - db;
			break;
		case OPCODE_MUL:
			testc = testa * testb;
			dc = da * db;
			break;
		case OPCODE_DIV:
			testc = testa / testb;
			dc = da / db;
			break;
		case OPCODE_IPA:
			testc = testa;
			testc += testb;
			dc = da + db;
			break;
		case OPCODE_IPS:
			testc = testa;
			testc -= testb;
			dc = da - db;
			break;
		case OPCODE_IPM:
			testc = testa;
			testc *= testb;
			dc = da * db;
			break;
		case OPCODE_IPD:
			testc = testa;
			testc /= testb;
			dc = da / db;
			break;
		case OPCODE_POW:
			testc = sw::universal::pow(testa, testb);
			dc = std::pow(da, db);
			break;
		case OPCODE_NOP:
		default:
			std::cerr << "unary operators not supported in executeBinary: operation ignored\n";
			break;
		}
		testref = dc;
	}

	// Execute a unary operator
	template<typename TestType>
	void executeUnary(int opcode, double da, double& dc, const TestType& testa, TestType& testc, TestType& testref, double dminpos) {
		dc = 0.0;
		switch (opcode) {
		case OPCODE_SQRT:
			testc = sw::universal::sqrt(testa);
			dc = std::sqrt(da);
			break;
		case OPCODE_EXP:
			testc = sw::universal::exp(testa);
			dc = std::exp(da);
			if (0.0 == dc) dc = dminpos;
			break;
		case OPCODE_EXP2:
			testc = sw::universal::exp2(testa);
			dc = std::exp2(da);
			if (0.0 == dc) dc = dminpos;
			break;
		case OPCODE_LOG:
			testc = sw::universal::log(testa);
			dc = std::log(da);
			break;
		case OPCODE_LOG2:
			testc = sw::universal::log2(testa);
			dc = std::log2(da);
			break;
		case OPCODE_LOG10:
			testc = sw::universal::log10(testa);
			dc = std::log10(da);
			break;
		case OPCODE_SIN:
			testc = sw::universal::sin(testa);
			dc = std::sin(da);
			break;
		case OPCODE_COS:
			testc = sw::universal::cos(testa);
			dc = std::cos(da);
			break;
		case OPCODE_TAN:
			testc = sw::universal::tan(testa);
			dc = std::tan(da);
			break;
		case OPCODE_ASIN:
			testc = sw::universal::asin(testa);
			dc = std::asin(da);
			break;
		case OPCODE_ACOS:
			testc = sw::universal::acos(testa);
			dc = std::acos(da);
			break;
		case OPCODE_ATAN:
			testc = sw::universal::atan(testa);
			dc = std::atan(da);
			break;
		case OPCODE_SINH:
			testc = sw::universal::sinh(testa);
			dc = std::sinh(da);
			break;
		case OPCODE_COSH:
			testc = sw::universal::cosh(testa);
			dc = std::cosh(da);
			break;
		case OPCODE_TANH:
			testc = sw::universal::tanh(testa);
			dc = std::tanh(da);
			break;
		case OPCODE_ASINH:
			testc = sw::universal::asinh(testa);
			dc = std::asinh(da);
			break;
		case OPCODE_ACOSH:
			testc = sw::universal::acosh(testa);
			dc = std::acosh(da);
			break;
		case OPCODE_ATANH:
			testc = sw::universal::atanh(testa);
			dc = std::atanh(da);
			break;
		case OPCODE_NOP:
		default:
			std::cerr << "Unsupported binary operator: operation ignored\n";
			break;
		}
		testref = dc;
	}

	// generate a random set of operands to test the binary operators for a posit configuration
	// Basic design is that we generate nrOfRandom posit values and store them in an operand array.
	// We will then execute the binary operator nrOfRandom combinations.
	template<typename TestType>
	int VerifyBinaryOperatorThroughRandoms(bool reportTestCases, int opcode, uint32_t nrOfRandoms) {
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
		// generate the full state space set of valid posit values
		std::random_device rd;     // get a random seed from the OS entropy device, or whatever
		std::mt19937_64 eng(rd()); // use the 64-bit Mersenne Twister 19937 generator and seed it with entropy.
		// define the distribution, by default it goes from 0 to MAX(unsigned long long)
		std::uniform_int_distribution<unsigned long long> distr;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
		bool firstNaRCall = true;
		bool firstDivideByZeroCall = true;
#endif
		int nrOfFailedTests = 0;
		for (unsigned i = 1; i < nrOfRandoms; i++) {
			TestType testa, testb, testc, testref;
			testa.setbits(distr(eng));
			testb.setbits(distr(eng));
			double da = double(testa);
			double db = double(testb);
			double dc = 0.0;
			// in case you have numeric_limits<long double>::digits trouble... this will show that
			//std::cout << "sizeof da: " << sizeof(da) << " bits in significant " << (std::numeric_limits<long double>::digits - 1) << " value da " << da << " at index " << ia << " testa " << testa << std::endl;
			//std::cout << "sizeof db: " << sizeof(db) << " bits in significant " << (std::numeric_limits<long double>::digits - 1) << " value db " << db << " at index " << ia << " testa " << testb << std::endl;

#if POSIT_THROW_ARITHMETIC_EXCEPTION
			try {
				executeBinary(opcode, da, db, dc, testa, testb, testc, testref);
			}
			catch (const posit_arithmetic_exception& err) {
				if (testa.isnar() || testb.isnar()) {
					if (reportTestCases && firstNaRCall) std::cerr << "Correctly caught arithmetic exception: " << err.what() << std::endl;
					firstNaRCall = false;
					continue;
				}
				else if (((opcode == OPCODE_DIV || opcode == OPCODE_IPD) && testb.iszero())) {
					if (reportTestCases && firstDivideByZeroCall) std::cerr << "Correctly caught arithmetic exception: " << err.what() << std::endl;
					firstDivideByZeroCall = false;
					continue;
				}
				else {
					throw; // rethrow
				}
			}
#else
			executeBinary(opcode, da, db, dc, testa, testb, testc, testref);
#endif

			if (testc != testref) {
				nrOfFailedTests++;
				if (reportTestCases) ReportBinaryArithmeticError("FAIL", operation_string, testa, testb, testc, testref);
			}
			else {
				//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", operation_string, testa, testb, testc, testref);
			}
		}
		return nrOfFailedTests;
	}

	// generate a random set of operands to test the unary operators for an arithmetic type configuration
	// Basic design is that we generate nrOfRandom posit values and store them in an operand array.
	// We will then execute the binary operator nrOfRandom combinations.
	// provide 		double dminpos = double(minpos<nbits, es>(pminpos));
	template<typename TestType>
	int VerifyUnaryOperatorThroughRandoms(bool reportTestCases, int opcode, uint32_t nrOfRandoms, double dminpos) {
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
		case OPCODE_EXP2:
		case OPCODE_LOG:
		case OPCODE_LOG2:
		case OPCODE_LOG10:
		case OPCODE_SIN:
		case OPCODE_COS:
		case OPCODE_TAN:
		case OPCODE_ASIN:
		case OPCODE_ACOS:
		case OPCODE_ATAN:
		case OPCODE_SINH:
		case OPCODE_COSH:
		case OPCODE_TANH:
		case OPCODE_ASINH:
		case OPCODE_ACOSH:
		case OPCODE_ATANH:
			// valid unary function
			break;
			// two operand elementary functions
		case OPCODE_POW:
			std::cerr << "Unsupported binary function, test cancelled\n";
			return 1;
		}
		// generate the full state space set of valid posit values
		std::random_device rd;     // get a random seed from the OS entropy device, or whatever
		std::mt19937_64 eng(rd()); // use the 64-bit Mersenne Twister 19937 generator and seed it with entropy.
		// define the distribution, by default it goes from 0 to MAX(unsigned long long)
		std::uniform_int_distribution<unsigned long long> distr;
		int nrOfFailedTests = 0;
		for (unsigned i = 1; i < nrOfRandoms; i++) {
			TestType testa, testc, testref;
			testa.setbits(distr(eng));
			if (sqrtOperator && testa < 0) testa = -testa;
			double da = double(testa);
			double dc = 0.0;
			// in case you have numeric_limits<long double>::digits trouble... this will show that
			//std::cout << "sizeof da: " << sizeof(da) << " bits in significant " << (std::numeric_limits<long double>::digits - 1) << " value da " << da << " at index " << ia << " testa " << testa << std::endl;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
			try {
				executeUnary(opcode, da, dc, testa, testc, testref, dminpos);
			}
			catch (const posit_arithmetic_exception& err) {
				if (testa.isnar()) {
					if (reportTestCases) std::cerr << "Correctly caught arithmetic exception: " << err.what() << std::endl;
				}
				else {
					throw;  // rethrow
				}
			}
#else
			executeUnary(opcode, da, dc, testa, testc, testref, dminpos);
#endif
			if (testc != testref) {
				nrOfFailedTests++;
				if (reportTestCases) ReportUnaryArithmeticError("FAIL", operation_string, testa, testc, testref);
			}
			else {
				//if (reportTestCases) ReportUnaryArithmeticSuccess("PASS", operation_string, testa, testc, testref);
			}
		}

		return nrOfFailedTests;
	}

	template<size_t nbits, size_t es>
	int Compare(long double input, const posit<nbits, es>& testresult, const posit<nbits, es>& ptarget, const posit<nbits+1,es>& pref, bool reportTestCases) {
		int fail = 0;
		if (testresult != ptarget) {
			fail++;
			if (reportTestCases) {
				ReportConversionError("FAIL", "=", input, (long double)(ptarget), testresult);
				std::cout << "reference   : " << pref.get() << std::endl;
				std::cout << "target bits : " << ptarget.get() << std::endl;
				std::cout << "actual bits : " << testresult.get() << std::endl;
			}
		}
		else {
			// if (reportTestCases) ReportConversionSuccess("PASS", "=", input, reference, testresult);
		}
		return fail;
	}


	// generate a random set of conversion cases
	template<size_t nbits, size_t es>
	int VerifyConversionThroughRandoms(const std::string& tag, bool reportTestCases, uint32_t nrOfRandoms) {
		// we are going to generate a test set that consists of all posit configs and their midpoints
		// we do this by enumerating a posit that is 1-bit larger than the test posit configuration
		// These larger posits will be at the mid-point between the smaller posit sample values
		// and we'll enumerate the exact value, and a perturbation smaller and a perturbation larger
		// to test the rounding logic of the conversion.
		posit<nbits + 1, es> pref, pprev, pnext;

		// setup the random number generator
		std::random_device rd;     //Get a random seed from the OS entropy device, or whatever
		std::mt19937_64 eng(rd()); //Use the 64-bit Mersenne Twister 19937 generator and seed it with entropy.
									//Define the distribution, by default it goes from 0 to MAX(unsigned long long)
		std::uniform_int_distribution<unsigned long long> distr;

		// execute the test
		int nrOfFailedTests = 0;
		for (uint32_t i = 0; i < nrOfRandoms; ++i) {
			posit<nbits, es> testresult, ptarget;
			// generate random value
			unsigned long long value = distr(eng);
			pref.setbits(value);   // assign to a posit<nbits+1,es> to generate the reference we know how to perturb

/*
			long double da = (long double)(pref);
			std::cout << std::hex << "0x" << value << std::endl;
			std::cout << std::dec << da << std::endl;
			long double eps;
			if (value == 0) {
				eps = minpos / 2.0;
			}
			else {
				eps = da > 0 ? da * 1.0e-6 : da * -1.0e-6;
			}
*/

			pprev = pnext = pref;
			--pprev;
			++pnext;
			bitblock<nbits> raw_target;
			long double input;
			if (value % 2) {
				// for odd values, we are between posit values, so we create the round-up and round-down cases

				// round-down case
				input = (long double)(pprev);
				testresult = input;
				truncate(pprev.get(), raw_target);
				ptarget.set(raw_target);
				nrOfFailedTests += Compare(input, testresult, ptarget, pref, reportTestCases);
				// round-up
				input = (long double)(pnext);
				testresult = input;
				truncate(pnext.get(), raw_target);
				ptarget.set(raw_target);
				nrOfFailedTests += Compare(input, testresult, ptarget, pref, reportTestCases);
			}
			else {
				// for even values, we are on a posit value, so we create the round-up and round-down cases
				// by perturbing negative for rounding up, and perturbing positive for rounding down
				// The problem you will run into for large posits is that you need 128-bit floats to be
				// able to make the perturbation small enough not to end up on a completely different posit.

				// round-up
				input = (long double)(pprev);
				testresult = input;
				ptarget = (long double)(pref);
				//nrOfFailedTests += Compare(input, testresult, ptarget, pref, reportTestCases);
				// round-down
				input = (long double)(pnext);
				testresult = input;
				ptarget = (long double)(pref);
				nrOfFailedTests += Compare(input, testresult, ptarget, pref, reportTestCases);
			}
		}
		return nrOfFailedTests;
	}

}} // namespace sw::universal
