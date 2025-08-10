#pragma once
// test_suite_random.hpp : verification functions based on random operand generation testing
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cmath>
//#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>

#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

	///////////////////////// Randomized Test Case Generation ////////////////////////

	// for testing posit configs that are nbits > 14-15, we need a more efficient approach.
	// One simple, brute force approach is to generate randoms.
	//
	// A more white box approach is to focus on the testcases 
	// where something special happens in the arithmetic, such as rounding,
	// or the geometric rounding and inward projections.

	enum class RandomsOp {
		// operation opcodes
		OPCODE_NOP = 0,
		OPCODE_ASSIGN = 1,
		OPCODE_ADD = 2,
		OPCODE_SUB = 3,
		OPCODE_MUL = 4,
		OPCODE_DIV = 5,
		OPCODE_IPA = 6,         // in-place addition
		OPCODE_IPS = 7,
		OPCODE_IPM = 8,
		OPCODE_IPD = 9,
		// elementary functions with one operand
		OPCODE_SQRT = 20,
		OPCODE_EXP = 21,
		OPCODE_EXP2 = 22,
		OPCODE_LOG = 23,
		OPCODE_LOG2 = 24,
		OPCODE_LOG10 = 25,
		OPCODE_SIN = 26,
		OPCODE_COS = 27,
		OPCODE_TAN = 28,
		OPCODE_ASIN = 29,
		OPCODE_ACOS = 30,
		OPCODE_ATAN = 31,
		OPCODE_SINH = 32,
		OPCODE_COSH = 33,
		OPCODE_TANH = 34,
		OPCODE_ASINH = 35,
		OPCODE_ACOSH = 36,
		OPCODE_ATANH = 37,
		// elementary functions with two operands
		OPCODE_POW = 50,
		OPCODE_HYPOT = 51,
		OPCODE_RAN = 60
	};

	// Execute a binary operator
	template<typename TestType>
	void executeBinary(RandomsOp opcode, double da, double db, const TestType& testa, const TestType& testb, TestType& testresult, TestType& testref) {
		double reference = 0.0;
		switch (opcode) {
		case RandomsOp::OPCODE_ADD:
			testresult = testa + testb;
			reference = da + db;
			break;
		case RandomsOp::OPCODE_SUB:
			testresult = testa - testb;
			reference = da - db;
			break;
		case RandomsOp::OPCODE_MUL:
			testresult = testa * testb;
			reference = da * db;
			break;
		case RandomsOp::OPCODE_DIV:
			testresult = testa / testb;
			reference = da / db;
			break;
		case RandomsOp::OPCODE_IPA:
			testresult = testa;
			testresult += testb;
			reference = da + db;
			break;
		case RandomsOp::OPCODE_IPS:
			testresult = testa;
			testresult -= testb;
			reference = da - db;
			break;
		case RandomsOp::OPCODE_IPM:
			testresult = testa;
			testresult *= testb;
			reference = da * db;
			break;
		case RandomsOp::OPCODE_IPD:
			testresult = testa;
			testresult /= testb;
			reference = da / db;
			break;
		case RandomsOp::OPCODE_POW:
			testresult = pow(testa, testb);
			reference = std::pow(da, db);
			break;
		case RandomsOp::OPCODE_NOP:
		default:
			std::cerr << "Unsupported unary operator: operation ignored\n";
			break;
		case RandomsOp::OPCODE_EXP:
		case RandomsOp::OPCODE_EXP2:
		case RandomsOp::OPCODE_LOG:
		case RandomsOp::OPCODE_LOG2:
		case RandomsOp::OPCODE_LOG10:
		case RandomsOp::OPCODE_SIN:
		case RandomsOp::OPCODE_COS:
		case RandomsOp::OPCODE_TAN:
		case RandomsOp::OPCODE_ASIN:
		case RandomsOp::OPCODE_ACOS:
		case RandomsOp::OPCODE_ATAN:
		case RandomsOp::OPCODE_SINH:
		case RandomsOp::OPCODE_COSH:
		case RandomsOp::OPCODE_TANH:
		case RandomsOp::OPCODE_ASINH:
		case RandomsOp::OPCODE_ACOSH:
		case RandomsOp::OPCODE_ATANH:
		case RandomsOp::OPCODE_HYPOT:
			std::cerr << "executeBinary does not support math function evaluation\n";
			break;
		case RandomsOp::OPCODE_SQRT:
		case RandomsOp::OPCODE_RAN:
		case RandomsOp::OPCODE_ASSIGN:
			std::cerr << "executeBinary does not support unary operators\n";
			break;
		}
		testref = reference;
	}

	// Execute a unary operator
	template<typename TestType>
	void executeUnary(RandomsOp opcode, double da, const TestType& nut, TestType& result, TestType& ref) {
		using std::sqrt;
		using std::exp;
		using std::exp2;
		using std::log;
		using std::log2;
		using std::log10;
		using std::sin;
		using std::cos;
		using std::tan;
		using std::asin;
		using std::acos;
		using std::atan;
		using std::sinh;
		using std::cosh;
		using std::tanh;
		using std::asinh;
		using std::acosh;
		using std::atanh;

		double reference = 0.0;
		result = 0;
		switch (opcode) {
 		case RandomsOp::OPCODE_ASSIGN:
			result = da;
			reference = da;
			break;
		case RandomsOp::OPCODE_SQRT:
			result = sqrt(nut);
			reference = std::sqrt(da);
			break;
		case RandomsOp::OPCODE_EXP:
			result = exp(nut);
			reference = std::exp(da);
			break;
		case RandomsOp::OPCODE_EXP2:
			result = exp2(nut);
			reference = std::exp2(da);
			break;
		case RandomsOp::OPCODE_LOG:
			result = log(nut);
			reference = std::log(da);
			break;
		case RandomsOp::OPCODE_LOG2:
			result = log2(nut);
			reference = std::log2(da);
			break;
		case RandomsOp::OPCODE_LOG10:
			result = log10(nut);
			reference = std::log10(da);
			break;
		case RandomsOp::OPCODE_SIN:
			result = sin(nut);
			reference = std::sin(da);
			break;
		case RandomsOp::OPCODE_COS:
			result = cos(nut);
			reference = std::cos(da);
			break;
		case RandomsOp::OPCODE_TAN:
			result = tan(nut);
			reference = std::tan(da);
			break;
		case RandomsOp::OPCODE_ASIN:
			result = asin(nut);
			reference = std::asin(da);
			break;
		case RandomsOp::OPCODE_ACOS:
			result = acos(nut);
			reference = std::acos(da);
			break;
		case RandomsOp::OPCODE_ATAN:
			result = atan(nut);
			reference = std::atan(da);
			break;
		case RandomsOp::OPCODE_SINH:
			result = sinh(nut);
			reference = std::sinh(da);
			break;
		case RandomsOp::OPCODE_COSH:
			result = cosh(nut);
			reference = std::cosh(da);
			break;
		case RandomsOp::OPCODE_TANH:
			result = tanh(nut);
			reference = std::tanh(da);
			break;
		case RandomsOp::OPCODE_ASINH:
			result = asinh(nut);
			reference = std::asinh(da);
			break;
		case RandomsOp::OPCODE_ACOSH:
			result = acosh(nut);
			reference = std::acosh(da);
			break;
		case RandomsOp::OPCODE_ATANH:
			result = atanh(nut);
			reference = std::atanh(da);
			break;
		case RandomsOp::OPCODE_NOP:
		case RandomsOp::OPCODE_ADD:
		case RandomsOp::OPCODE_SUB:
		case RandomsOp::OPCODE_MUL:
		case RandomsOp::OPCODE_DIV:
		case RandomsOp::OPCODE_IPA:
		case RandomsOp::OPCODE_IPS:
		case RandomsOp::OPCODE_IPM:
		case RandomsOp::OPCODE_IPD:
		case RandomsOp::OPCODE_POW:
		case RandomsOp::OPCODE_HYPOT:
		case RandomsOp::OPCODE_RAN:
		default:
			result = std::numeric_limits<float>::signaling_NaN();
			std::cerr << "Unsupported unary operator: operation ignored\n";
			break;
		}
		ref = reference;
	}

	// generate a random set of operands to test the binary operators for a number system configuration
	// Basic design is that we generate nrOfRandom posit values and store them in an operand array.
	// We will then execute the binary operator nrOfRandom combinations.
	template<typename TestType>
	int VerifyBinaryOperatorThroughRandoms(bool reportTestCases, RandomsOp opcode, size_t nrOfRandoms) {
		///std::cerr << typeid(TestType).name() << " : ";

		std::string operation_string;
		switch (opcode) {
		case RandomsOp::OPCODE_ADD:
			operation_string = "+";
			break;
		case RandomsOp::OPCODE_SUB:
			operation_string = "-";
			break;
		case RandomsOp::OPCODE_MUL:
			operation_string = "*";
			break;
		case RandomsOp::OPCODE_DIV:
			operation_string = "/";
			break;
		case RandomsOp::OPCODE_IPA:
			operation_string = "+=";
			break;
		case RandomsOp::OPCODE_IPS:
			operation_string = "-=";
			break;
		case RandomsOp::OPCODE_IPM:
			operation_string = "*=";
			break;
		case RandomsOp::OPCODE_IPD:
			operation_string = "/=";
			break;
		case RandomsOp::OPCODE_POW:
			operation_string = "pow";
			break;
		case RandomsOp::OPCODE_EXP:
		case RandomsOp::OPCODE_EXP2:
		case RandomsOp::OPCODE_LOG:
		case RandomsOp::OPCODE_LOG2:
		case RandomsOp::OPCODE_LOG10:
		case RandomsOp::OPCODE_SIN:
		case RandomsOp::OPCODE_COS:
		case RandomsOp::OPCODE_TAN:
		case RandomsOp::OPCODE_ASIN:
		case RandomsOp::OPCODE_ACOS:
		case RandomsOp::OPCODE_ATAN:
		case RandomsOp::OPCODE_SINH:
		case RandomsOp::OPCODE_COSH:
		case RandomsOp::OPCODE_TANH:
		case RandomsOp::OPCODE_ASINH:
		case RandomsOp::OPCODE_ACOSH:
		case RandomsOp::OPCODE_ATANH:
		case RandomsOp::OPCODE_HYPOT:
			std::cerr << "VerifyBinaryOperatorThroughRandoms does not support math function evaluation\n";
			return 1;
		case RandomsOp::OPCODE_NOP:
			std::cerr << "VerifyBinaryOperatorThroughRandoms NOP\n";
			break;
		case RandomsOp::OPCODE_RAN:
		case RandomsOp::OPCODE_ASSIGN:
		case RandomsOp::OPCODE_SQRT:
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
		for (unsigned i = 1; i < nrOfRandoms; i++) {
			TestType testa, testb, result{0}, ref{0};
			testa.setbits(distr(eng));
			testb.setbits(distr(eng));
			double da = double(testa);
			double db = double(testb);
			// in case you have numeric_limits<long double>::digits trouble... this will show that
			//std::cout << "sizeof da: " << sizeof(da) << " bits in significant " << (std::numeric_limits<long double>::digits - 1) << " value da " << da << " at index " << ia << " testa " << testa << std::endl;
			//std::cout << "sizeof db: " << sizeof(db) << " bits in significant " << (std::numeric_limits<long double>::digits - 1) << " value db " << db << " at index " << ia << " testa " << testb << std::endl;

			executeBinary(opcode, da, db, testa, testb, result, ref);
			// check the result
			if (result != ref) {
				// we can't properly test the NaN and Inf encodings as the transformation through the custom type
				// tends to be special cased. So check for these cases and ignore the failure
				switch (std::fpclassify(da)) {
				case FP_NAN:
				case FP_INFINITE:
				case FP_ZERO:
					continue;
				case FP_NORMAL:
				case FP_SUBNORMAL:
				default:
					break;
				}
				switch (std::fpclassify(db)) {
				case FP_NAN:
				case FP_INFINITE:
				case FP_ZERO:
					continue;
				case FP_NORMAL:
				case FP_SUBNORMAL:
				default:
					break;
				}
				//if (reportTestCases) std::cerr << "result : " << result << '\n';
				//if (reportTestCases) std::cerr << "ref    : " << ref << '\n';
				bool resultIsInf = result.isinf();
				bool refIsInf = ref.isinf();
				if (resultIsInf && refIsInf) {
					if (reportTestCases) std::cerr << "result and ref are both inf\n";
					// but ignore the failure
					continue;
				}
				else {
					if (reportTestCases) std::cerr << "result and/or ref are normal\n";
					TestType diff = result - ref;
					if (reportTestCases) std::cerr << "diff = " << to_binary(diff) << '\n';
				}
				++nrOfFailedTests;
				if (reportTestCases) ReportBinaryArithmeticError("FAIL", operation_string, testa, testb, result, ref);
			}
			else {
				//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", operation_string, testa, testb, result, ref);
			}
		}
		return nrOfFailedTests;
	}

	// generate a random set of operands to test the binary operators for a arithmetic type configuration
	// Basic design is that we generate nrOfRandom uint64_t values and store them in an operand array.
	// We will then execute the unary operator on .
	// provide 		double dminpos = double(minpos<nbits, es>(pminpos));
	template<typename TestType>
	int VerifyUnaryOperatorThroughRandoms(bool reportTestCases, RandomsOp opcode, size_t nrOfRandoms) {
		std::string operation_string;
		bool sqrtOperator = false;  // we need to filter negative values from the randoms
		switch (opcode) {
		default:
		case RandomsOp::OPCODE_NOP:
		case RandomsOp::OPCODE_POW:
		case RandomsOp::OPCODE_RAN:
			return 0;
		case RandomsOp::OPCODE_ASSIGN:
			break;
		case RandomsOp::OPCODE_ADD:
		case RandomsOp::OPCODE_SUB:
		case RandomsOp::OPCODE_MUL:
		case RandomsOp::OPCODE_DIV:
		case RandomsOp::OPCODE_IPA:
		case RandomsOp::OPCODE_IPS:
		case RandomsOp::OPCODE_IPM:
		case RandomsOp::OPCODE_IPD:
			std::cerr << "Unsupported binary operator, test cancelled\n";
			return 1;
		case RandomsOp::OPCODE_SQRT:
			operation_string = "sqrt";
			sqrtOperator = true;
			break;
		case RandomsOp::OPCODE_EXP:
			break;
		case RandomsOp::OPCODE_EXP2:
			break;
		case RandomsOp::OPCODE_LOG:
			break;
		case RandomsOp::OPCODE_LOG2:
			break;
		case RandomsOp::OPCODE_LOG10:
			break;
		case RandomsOp::OPCODE_SIN:
			break;
		case RandomsOp::OPCODE_COS:
			break;
		case RandomsOp::OPCODE_TAN:
			break;
		case RandomsOp::OPCODE_ASIN:
			break;
		case RandomsOp::OPCODE_ACOS:
			break;
		case RandomsOp::OPCODE_ATAN:
			break;
		case RandomsOp::OPCODE_SINH:
			break;
		case RandomsOp::OPCODE_COSH:
			break;
		case RandomsOp::OPCODE_TANH:
			break;
		case RandomsOp::OPCODE_ASINH:
			break;
		case RandomsOp::OPCODE_ACOSH:
			break;
		case RandomsOp::OPCODE_ATANH:
			break;
		case RandomsOp::OPCODE_HYPOT:
			break;
		}
		// generate random 64-bit strings and assign them to the arithmetic type
		std::random_device rd;     // get a random seed from the OS entropy device
		std::mt19937_64 eng(rd()); // use the 64-bit Mersenne Twister 19937 generator and seed it with entropy.
		// define the distribution, by default it goes from 0 to MAX(unsigned long long)
		std::uniform_int_distribution<unsigned long long> distr;
		int nrOfFailedTests = 0;
		for (unsigned i = 1; i < nrOfRandoms; i++) {
			TestType nut, result, ref;
			nut.setbits(distr(eng));  // nut: number system under test
			if (sqrtOperator && nut < 0) nut = -nut;
			double da = double(nut);

			executeUnary(opcode, da, nut, ref, result);

			if (result != ref) {
				if (result.isnan() && ref.isnan()) continue;
				++nrOfFailedTests;
				if (reportTestCases) ReportUnaryArithmeticError("FAIL", operation_string, nut, result, ref);
			}
			else {
				if (reportTestCases) ReportUnaryArithmeticSuccess<TestType>("PASS", operation_string, nut, result, ref);
			}
		}

		return nrOfFailedTests;
	}

	// generate a random set of operands to test the unary operators for an arithmetic type configuration
	// Basic design is that we generate nrOfRandom posit values and store them in an operand array.
	// We will then execute the binary operator nrOfRandom combinations.
	// provide 		double dminpos = double(minpos<nbits, es>(pminpos));
	template<typename TestType, bool throwArithmeticException = true>
	int VerifyElementaryFunctionThroughRandoms(bool reportTestCases, RandomsOp opcode, uint32_t nrOfRandoms) {
		std::string operation_string;
		bool positiveOnlyOperator = false;  // we need to filter negative values from the randoms
		switch (opcode) {
		default:
		case RandomsOp::OPCODE_NOP:
			return 0;
		case RandomsOp::OPCODE_ADD:
		case RandomsOp::OPCODE_SUB:
		case RandomsOp::OPCODE_MUL:
		case RandomsOp::OPCODE_DIV:
		case RandomsOp::OPCODE_IPA:
		case RandomsOp::OPCODE_IPS:
		case RandomsOp::OPCODE_IPM:
		case RandomsOp::OPCODE_IPD:
			std::cerr << "Unsupported binary operator, test cancelled\n";
			return 1;

			// elementary functions
		case RandomsOp::OPCODE_SQRT:
			operation_string = "sqrt";
			positiveOnlyOperator = true;
			break;
		case RandomsOp::OPCODE_EXP:
			operation_string = "exp";
			break;
		case RandomsOp::OPCODE_EXP2:
			operation_string = "exp2";
			break;
		case RandomsOp::OPCODE_LOG:
			operation_string = "log";
			positiveOnlyOperator = true;
			break;
		case RandomsOp::OPCODE_LOG2:
			operation_string = "log2";
			positiveOnlyOperator = true;
			break;
		case RandomsOp::OPCODE_LOG10:
			operation_string = "log10";
			positiveOnlyOperator = true;
			break;
		case RandomsOp::OPCODE_SIN:
			operation_string = "sin";
			break;
		case RandomsOp::OPCODE_COS:
			operation_string = "cos";
			break;
		case RandomsOp::OPCODE_TAN:
			operation_string = "tan";
			break;
		case RandomsOp::OPCODE_ASIN:
			operation_string = "asin";
			break;
		case RandomsOp::OPCODE_ACOS:
			operation_string = "acos";
			break;
		case RandomsOp::OPCODE_ATAN:
			operation_string = "atan";
			break;
		case RandomsOp::OPCODE_SINH:
			operation_string = "sinh";
			break;
		case RandomsOp::OPCODE_COSH:
			operation_string = "cosh";
			break;
		case RandomsOp::OPCODE_TANH:
			operation_string = "tanh";
			break;
		case RandomsOp::OPCODE_ASINH:
			operation_string = "asinh";
			break;
		case RandomsOp::OPCODE_ACOSH:
			operation_string = "acosh";
			break;
		case RandomsOp::OPCODE_ATANH:
			operation_string = "atanh";
			break;
			// two operand elementary functions
		case RandomsOp::OPCODE_POW:
			operation_string = "pow";
			break;
		case RandomsOp::OPCODE_HYPOT:
			operation_string = "hypot";
			break;
		}
		// generate the full state space set of valid posit values
		std::random_device rd;     // get a random seed from the OS entropy device, or whatever
		std::mt19937_64 eng(rd()); // use the 64-bit Mersenne Twister 19937 generator and seed it with entropy.
		// define the distribution, by default it goes from 0 to MAX(unsigned long long)
		std::uniform_int_distribution<unsigned long long> distr;
		int nrOfFailedTests = 0;
		for (unsigned i = 1; i < nrOfRandoms; i++) {
			TestType nut, result, ref;
			nut.setbits(distr(eng));
			if (positiveOnlyOperator && nut < 0) nut = -nut;
			double da = double(nut);
			// in case you have numeric_limits<long double>::digits trouble... this will show that
//			std::cout << "sizeof da: " << sizeof(da) << "bytes\n";
//			std::cout << "bits in significant " << (std::numeric_limits<long double>::digits - 1) << " value da " << da << " nut " << nut << std::endl;
			if constexpr (throwArithmeticException) {
				try {
					executeUnary(opcode, da, nut, result, ref);
				}
				catch (const universal_arithmetic_exception& err) {
					if (nut.isnan()) {
						if (reportTestCases) std::cerr << "Correctly caught arithmetic exception: " << err.what() << std::endl;
					}
					else {
						throw;  // rethrow
					}
				}
			}
			else {
				executeUnary(opcode, da, nut, ref, result);
			}
			if (result != ref) {
				std::cout << "result    : " << to_binary(result) << '\n';
				std::cout << "reference : " << to_binary(ref) << '\n';
				++nrOfFailedTests;
				if (reportTestCases) ReportUnaryArithmeticError("FAIL", operation_string, nut, result, ref);
			}
			else {
				//if (reportTestCases) ReportUnaryArithmeticSuccess("PASS", operation_string, nut, result, ref);
			}
		}

		return nrOfFailedTests;
	}

	template<typename TestType>
	int Compare(long double input, const TestType& testresult, const TestType& ptarget, const TestType& pref, bool reportTestCases) {
		int fail = 0;
		if (testresult != ptarget) {
			++fail;
			if (reportTestCases) {
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

}} // namespace sw::universal
