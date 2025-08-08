#pragma once
// performance/number_system.hpp : functions to aid in measuring arithmetic performance of custom number system types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>
#include <chrono>
#include <universal/native/ieee754.hpp>
#include <universal/utility/scientific.hpp>

namespace sw { namespace universal {

	// standardized structure to hold performance measurement results
	// 
	struct OperatorPerformance {
		OperatorPerformance() : intconvert(0), ieeeconvert(0), prefix(0), postfix(0), neg(0), add(0), sub(0), mul(0), div(0), sqrt(0) {}
		double intconvert;
		double ieeeconvert;
		double prefix;
		double postfix;
		double neg;
		double add;
		double sub;
		double mul;
		double div;
		double sqrt;
	};

	static constexpr int NR_TEST_CASES = 100000;
	static constexpr size_t FLOAT_TABLE_WIDTH = 15;

	template<typename Scalar>
	std::string ReportPerformance(const Scalar& number, OperatorPerformance &perf) {
		std::stringstream ostr;
		ostr << "Performance Report for type: " << typeid(number).name() << '\n'
			<< "Conversion int  : " << to_scientific(perf.intconvert) << "POPS\n"
			<< "Conversion ieee : " << to_scientific(perf.ieeeconvert) << "POPS\n"
			<< "Prefix          : " << to_scientific(perf.prefix) << "POPS\n"
			<< "Postfix         : " << to_scientific(perf.postfix) << "POPS\n"
			<< "Negation        : " << to_scientific(perf.neg) << "POPS\n"
			<< "Addition        : " << to_scientific(perf.add) << "POPS\n"
			<< "Subtraction     : " << to_scientific(perf.sub) << "POPS\n"
			<< "Multiplication  : " << to_scientific(perf.mul) << "POPS\n"
			<< "Division        : " << to_scientific(perf.div) << "POPS\n"
			<< "Square Root     : " << to_scientific(perf.sqrt) << "POPS\n"
			<< std::endl;
		return ostr.str();
	}

	// Integer conversion case for a posit configuration
	template<typename Scalar>
	int MeasureIntegerConversionPerformance(Scalar& a, int &positives, int &negatives) {
		positives = 0, negatives = 0;
		for (int i = -(NR_TEST_CASES >> 1); i < (NR_TEST_CASES >> 1); ++i) {
			a = i;
			a >= Scalar(0) ? positives++ : negatives++;
		}
		return positives + negatives;
	}


	// IEEE conversion case for a posit configuration
	template<typename Scalar>
	int MeasureIeeeConversionPerformance(Scalar& a, int &positives, int &negatives) {
		positives = 0, negatives = 0;
		for (int i = 1; i < NR_TEST_CASES; i++) {
			a = double(i);
			a >= Scalar(0) ? positives++ : negatives++;
		}
		return positives + negatives;
	}


	// measure performance of the postfix operator++
	template<typename Scalar>
	int MeasurePostfixPerformance(Scalar& a, int &positives, int &negatives)	{
		a = 1;
		positives = 0; negatives = 0;
		for (int i = 1; i < NR_TEST_CASES; i++) {
			a++;
			a >= Scalar(0) ? positives++ : negatives++;
		}
		return positives + negatives;
	}


	// measure performance of the prefix operator++
	template<typename Scalar>
	int MeasurePrefixPerformance(Scalar& a, int &positives, int &negatives) {
		a = 1;
		positives = 0; negatives = 0;
		for (int i = 1; i < NR_TEST_CASES; i++) {
			++a;
			a >= Scalar(0) ? positives++ : negatives++;
		}
		return positives + negatives;
	}


	// enumerate all negation cases for a posit configuration: executes within 10 sec till about nbits = 14
	template<typename Scalar>
	int MeasureNegationPerformance(Scalar& a, int &positives, int &negatives) {
		a = 1;
		positives = 0; negatives = 0;
		for (int i = 1; i < NR_TEST_CASES; i++) {
			a = -a;
			a >= Scalar(0) ? positives++ : negatives++;
		}
		return positives + negatives;
	}

	// enumerate all SQRT cases for a posit configuration: executes within 10 sec till about nbits = 14
	template<typename Scalar>
	int MeasureSqrtPerformance(Scalar& a, int &positives, int &negatives) {		
		using std::sqrt;
		using namespace sw::universal;
		positives = 0; negatives = 0;
		for (int i = 0; i < NR_TEST_CASES; i++) {
			a = i;
			Scalar root = sqrt(a);
			root >= Scalar(0) ? positives++ : negatives++;
		}
		return positives + negatives;
	}

	// measure performance of arithmetic addition
	template<typename Scalar>
	int MeasureAdditionPerformance(Scalar& a, int &positives, int &negatives) {
		a = 1;
		positives = 0; negatives = 0;
		for (int i = 0; i < NR_TEST_CASES; i++) {
			Scalar b( i );
			Scalar sum = a + b;
			sum >= Scalar(0) ? positives++ : negatives++;
		}
		return positives + negatives;
	}

	// measure performance of arithmetic subtraction
	template<typename Scalar>
	int MeasureSubtractionPerformance(Scalar& a, int &positives, int &negatives) {
		a = 1;
		positives = 0; negatives = 0;
		for (int i = 0; i < NR_TEST_CASES; i++) {
			Scalar b( i );
			Scalar diff = a - b;
			diff >= Scalar(0) ? positives++ : negatives++;
		}
		return positives + negatives;
	}

	// measure performance of arithmetic multiplication
	template<typename Scalar>
	int MeasureMultiplicationPerformance(Scalar& a, int &positives, int &negatives) {
		a = 1;
		positives = 0; negatives = 0;
		for (int i = 0; i < NR_TEST_CASES; i++) {
			Scalar b( i );
			Scalar mul = a * b;
			mul >= Scalar(0) ? positives++ : negatives++;
		}
		return positives + negatives;
	}

	// measure performance of arithmetic reciprocation
	template<typename Scalar>
	int MeasureReciprocationPerformance(Scalar& a, int &positives, int &negatives) {
		positives = 0; negatives = 0;
		for (size_t i = 1; i < NR_TEST_CASES; i++) {
			a.set_raw_bits(i);
			a = a.reciprocate();
			a >= Scalar(0) ? positives++ : negatives++;
		}
		return positives + negatives;
	}

	// measure performance of arithmetic division
	template<typename Scalar>
	int MeasureDivisionPerformance(Scalar& a, int &positives, int &negatives) {
		a = 1;
		positives = 0; negatives = 0;
		for (int i = 0; i < NR_TEST_CASES; i++) {
			Scalar b( i );
			Scalar div = a / b;
			div >= Scalar(0) ? positives++ : negatives++;
		}
		return positives + negatives;
	}

	//////////////////////////////////// RANDOMIZED TEST SUITE FOR BINARY OPERATORS ////////////////////////

	// for testing posit configs that are > 14-15, we need a more efficient approach.
	// One simple, brute force approach is to generate randoms.
	// A more white box approach is to focus on the testcases 
	// where something special happens in the posit arithmetic, such as rounding.

	// operation opcodes
	const int OPCODE_NOP = 0;
	const int OPCODE_ADD = 1;
	const int OPCODE_SUB = 2;
	const int OPCODE_MUL = 3;
	const int OPCODE_DIV = 4;
	const int OPCODE_RAN = 5;

	template<typename Scalar>
	void execute(int opcode, double da, double db, const Scalar& a, const Scalar& b, Scalar& reference, Scalar& result) {
		double oracle;
		switch (opcode) {
		default:
		case OPCODE_NOP:
			reference.setzero();
			result.setzero();
			return;
		case OPCODE_ADD:
			result = a + b;
			oracle = da + db;
			break;
		case OPCODE_SUB:
			result = a - b;
			oracle = da - db;
			break;
		case OPCODE_MUL:
			result = a * b;
			oracle = da * db;
			break;
		case OPCODE_DIV:
			result = a / b;
			oracle = da / db;
			break;
		}
		reference = oracle;
	}

	// generate a random set of operands to test the binary operators for an arbitrary number type
	// Basic design is that we generate nrOfRandom values and store them in an operand array.
	// We will then execute the binary operator nrOfRandom combinations.
	template<typename Scalar>
	int MeasureArithmeticPerformance(const std::string& tag, bool bReportIndividualTestCases, int opcode, uint32_t nrOfRandoms) {
		const size_t SIZE_STATE_SPACE = nrOfRandoms;
		int nrOfFailedTests = 0;
		Scalar a, b, result, reference;

		std::string operation_string;
		switch (opcode) {
		default:
		case OPCODE_NOP:
			operation_string = "nop";
			break;
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
		}
		// generate the full state space set of valid posit values
		std::random_device rd;     //Get a random seed from the OS entropy device, or whatever
		std::mt19937_64 eng(rd()); //Use the 64-bit Mersenne Twister 19937 generator and seed it with entropy.
								   //Define the distribution, by default it goes from 0 to MAX(unsigned long long)
		std::uniform_int_distribution<unsigned long long> distr;
		std::vector<long double> operand_values(SIZE_STATE_SPACE);
		for (uint32_t i = 0; i < SIZE_STATE_SPACE; i++) {
			result = (distr(eng)); 
			operand_values[i] = (long double)(result);
		}
		for (unsigned i = 1; i < nrOfRandoms; i++) {
			unsigned ia = std::rand() % SIZE_STATE_SPACE; // random indices for picking operands to test
			long double da = operand_values[ia];
			a = da;
			unsigned ib = std::rand() % SIZE_STATE_SPACE;
			long double db = operand_values[ib];
			b = db;
			// in case you have numeric_limits<long double>::digits trouble... this will show that
			//std::cout << "sizeof da: " << sizeof(da) << " bits in significant " << (std::numeric_limits<long double>::digits - 1) << " value da " << da << " at index " << ia << " pa " << pa << std::endl;
			//std::cout << "sizeof db: " << sizeof(db) << " bits in significant " << (std::numeric_limits<long double>::digits - 1) << " value db " << db << " at index " << ia << " pa " << pb << std::endl;
			execute(opcode, da, db, a, b, reference, result);
			if (result != reference) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases) ReportBinaryArithmeticErrorInBinary("FAIL", operation_string, a, b, reference, result);
			}
			else {
				//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccessInBinary("PASS", operation_string, a, b, reference, result);
			}
		}

		return nrOfFailedTests;
	}

	// run and measure performance tests and generate an operator performance report
	// The number argument is just for ADL specialization
	template<typename Scalar>
	void GeneratePerformanceReport(Scalar& number, OperatorPerformance &report) {
		using namespace std;
		using namespace std::chrono;
		int positives, negatives;

		steady_clock::time_point begin, end;
		duration<double> time_span;
		double elapsed;

		begin = steady_clock::now();
		    MeasureIntegerConversionPerformance(number, positives, negatives);
		end = steady_clock::now();
		time_span = duration_cast<duration<double>>(end - begin);
		elapsed = time_span.count();
		report.intconvert = (double(positives) + double(negatives)) / elapsed;

		begin = steady_clock::now();
			MeasureIeeeConversionPerformance(number, positives, negatives);
		end = steady_clock::now();
		time_span = duration_cast<duration<double>>(end - begin);
		elapsed = time_span.count();
		report.ieeeconvert = (double(positives) + double(negatives)) / elapsed;

		begin = steady_clock::now();
		    MeasurePrefixPerformance(number, positives, negatives);
		end = steady_clock::now();
		time_span = duration_cast<duration<double>>(end - begin);
		elapsed = time_span.count();
		report.prefix = (double(positives) + double(negatives)) / elapsed;

		begin = steady_clock::now();
		    MeasurePostfixPerformance(number, positives, negatives);
		end = steady_clock::now();
		time_span = duration_cast<duration<double>>(end - begin);
		elapsed = time_span.count();
		report.postfix = (double(positives) + double(negatives)) / elapsed;

		begin = steady_clock::now();
		    MeasureNegationPerformance(number, positives, negatives);
		end = steady_clock::now();
		time_span = duration_cast<duration<double>>(end - begin);
		elapsed = time_span.count();
		report.neg = (double(positives) + double(negatives)) / elapsed;

		begin = steady_clock::now();
		    MeasureSqrtPerformance(number, positives, negatives);
		end = steady_clock::now();
		time_span = duration_cast<duration<double>>(end - begin);
		elapsed = time_span.count();
		report.sqrt = (double(positives) + double(negatives)) / elapsed;

		begin = steady_clock::now();
		    MeasureAdditionPerformance(number, positives, negatives);
		end = steady_clock::now();
		time_span = duration_cast<duration<double>>(end - begin);
		elapsed = time_span.count();
		report.add = (double(positives) + double(negatives)) / elapsed;

		begin = steady_clock::now();
		    MeasureSubtractionPerformance(number, positives, negatives);
		end = steady_clock::now();
		time_span = duration_cast<duration<double>>(end - begin);
		elapsed = time_span.count();
		report.sub = (double(positives) + double(negatives)) / elapsed;

		begin = steady_clock::now();
		    MeasureMultiplicationPerformance(number, positives, negatives);
		end = steady_clock::now();
		time_span = duration_cast<duration<double>>(end - begin);
		elapsed = time_span.count();
		report.mul = (double(positives) + double(negatives)) / elapsed;

		begin = steady_clock::now();
		    MeasureDivisionPerformance(number, positives, negatives);
		end = steady_clock::now();
		time_span = duration_cast<duration<double>>(end - begin);
		elapsed = time_span.count();
		report.div = (double(positives) + double(negatives)) / elapsed;
	}

}} // namespace sw::universal
