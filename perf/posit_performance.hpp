#pragma once
//  posit_performance.hpp : functions to aid in measuring arithmetic performance of posit types.
// Needs to be included after posit type is declared.
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>
#include <chrono>

namespace sw {
	namespace unum {

		// standardized structure to hold performance measurement results
		// 
		struct OperatorPerformance {
			OperatorPerformance() : convert(0), prefix(0), postfix(0), neg(0), add(0), sub(0), mul(0), div(0), sqrt(0) {}
			float convert;
			float prefix;
			float postfix;
			float neg;
			float add;
			float sub;
			float mul;
			float div;
			float sqrt;
		};

		static constexpr int NR_TEST_CASES = 100000;
		static constexpr unsigned FLOAT_TABLE_WIDTH = 15;

		template<size_t nbits, size_t es>
		void ReportPerformance(std::ostream& ostr, std::string header, OperatorPerformance &perf) {
			ostr << "Performance Report: " << header << '\n'
				<< "Conversion      : " << perf.convert << " POPS\n"
				<< "Prefix          : " << perf.prefix << " POPS\n"
				<< "Postfix         : " << perf.postfix << " POPS\n"
				<< "Negation        : " << perf.neg << " POPS\n"
				<< "Addition        : " << perf.add << " POPS\n"
				<< "Subtraction     : " << perf.sub << " POPS\n"
				<< "Multiplication  : " << perf.mul << " POPS\n"
				<< "Division        : " << perf.div << " POPS\n"
				<< "Square Root     : " << perf.sqrt << " POPS\n"
				<< std::endl;
		}


		// enumerate all conversion cases for a posit configuration
		template<size_t nbits, size_t es>
		int MeasureConversionPerformance(int &positives, int &negatives) {
			posit<nbits, es> p(0);

			positives = 0, negatives = 0;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				p = 0.031625;
				p >= 0 ? positives++ : negatives++;
			}
			return positives + negatives;
		}


		// measure performance of the postfix operator++
		template<size_t nbits, size_t es>
		int MeasurePostfixPerformance(int &positives, int &negatives)	{
			posit<nbits, es> p(0);

			positives = 0; negatives = 0;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				p++;
				p >= 0 ? positives++ : negatives++;
			}
			return positives + negatives;
		}


		// measure performance of the prefix operator++
		template<size_t nbits, size_t es>
		int MeasurePrefixPerformance(int &positives, int &negatives) {
			posit<nbits, es> p(0);

			positives = 0; negatives = 0;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				++p;
				p >= 0 ? positives++ : negatives++;
			}
			return positives + negatives;
		}


		// enumerate all negation cases for a posit configuration: executes within 10 sec till about nbits = 14
		template<size_t nbits, size_t es>
		int MeasureNegationPerformance(int &positives, int &negatives) {
			posit<nbits, es> pa(0);

			positives = 0; negatives = 0;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				pa = -pa;
				pa >= 0 ? positives++ : negatives++;
			}
			return positives + negatives;
		}

		// enumerate all SQRT cases for a posit configuration: executes within 10 sec till about nbits = 14
		template<size_t nbits, size_t es>
		int MeasureSqrtPerformance(int &positives, int &negatives) {		
			posit<nbits, es> pa, psqrt;

			positives = 0; negatives = 0;
			for (int i = 0; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				psqrt = sw::unum::sqrt(pa);
				psqrt >= 0 ? positives++ : negatives++;
			}
			return positives + negatives;
		}

		// measure performance of arithmetic addition
		template<size_t nbits, size_t es>
		int MeasureAdditionPerformance(int &positives, int &negatives) {
			posit<nbits, es> pa(1.0), pb, psum;

			positives = 0; negatives = 0;
			for (int i = 0; i < NR_TEST_CASES; i++) {
				pb.set_raw_bits(i);
				psum = pa + pb;
				psum >= 0 ? positives++ : negatives++;
			}
			return positives + negatives;
		}

		// measure performance of arithmetic subtraction
		template<size_t nbits, size_t es>
		int MeasureSubtractionPerformance(int &positives, int &negatives) {
			posit<nbits, es> pa(1.0), pb, pdif;

			positives = 0; negatives = 0;
			for (int i = 0; i < NR_TEST_CASES; i++) {
				pb.set_raw_bits(i);
				pdif = pa - pb;
				pdif >= 0 ? positives++ : negatives++;
			}
			return positives + negatives;
		}

		// measure performance of arithmetic multiplication
		template<size_t nbits, size_t es>
		int MeasureMultiplicationPerformance(int &positives, int &negatives) {
			posit<nbits, es> pa(1.0), pb, pmul;

			positives = 0; negatives = 0;
			for (int i = 0; i < NR_TEST_CASES; i++) {
				pb.set_raw_bits(i);
				pmul = pa * pb;
				pmul >= 0 ? positives++ : negatives++;
			}
			return positives + negatives;
		}

		// measure performance of arithmetic reciprocation
		template<size_t nbits, size_t es>
		int MeasureReciprocationPerformance(int &positives, int &negatives) {
			posit<nbits, es> pa(0); ++pa; // minpos

			positives = 0; negatives = 0;
			for (int i = 0; i < NR_TEST_CASES; i++) {
				pa = pa.reciprocate();
				pa >= 0 ? positives++ : negatives++;
			}
			return positives + negatives;
		}

		// measure performance of arithmetic division
		template<size_t nbits, size_t es>
		int MeasureDivisionPerformance(int &positives, int &negatives) {
			posit<nbits, es> pa(1.0), pb, pdiv;

			positives = 0; negatives = 0;
			for (int i = 0; i < NR_TEST_CASES; i++) {
				pb.set_raw_bits(i);
				pdiv = pa / pb;
				pdiv >= 0 ? positives++ : negatives++;
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

		template<size_t nbits, size_t es>
		void execute(int opcode, double da, double db, const posit<nbits, es>& pa, const posit<nbits, es>& pb, posit<nbits, es>& preference, posit<nbits, es>& presult) {
			double reference;
			switch (opcode) {
			default:
			case OPCODE_NOP:
				preference.setToZero();
				presult.setToZero();
				return;
			case OPCODE_ADD:
				presult = pa + pb;
				reference = da + db;
				break;
			case OPCODE_SUB:
				presult = pa - pb;
				reference = da - db;
				break;
			case OPCODE_MUL:
				presult = pa * pb;
				reference = da * db;
				break;
			case OPCODE_DIV:
				presult = pa / pb;
				reference = da / db;
				break;
			}
			preference = reference;
		}

		// generate a random set of operands to test the binary operators for a posit configuration
		// Basic design is that we generate nrOfRandom posit values and store them in an operand array.
		// We will then execute the binary operator nrOfRandom combinations.
		template<size_t nbits, size_t es>
		int MeasureArithmeticPerformance(std::string tag, bool bReportIndividualTestCases, int opcode, uint32_t nrOfRandoms) {
			const size_t SIZE_STATE_SPACE = nrOfRandoms;
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, pb, presult, preference;

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
#ifdef POSIT_USE_LONG_DOUBLE
			std::vector<long double> operand_values(SIZE_STATE_SPACE);
			for (uint32_t i = 0; i < SIZE_STATE_SPACE; i++) {
				presult.set_raw_bits(distr(eng));  // take the bottom nbits bits as posit encoding
				operand_values[i] = (long double)(presult);
			}
			long double da, db;
#else // USE DOUBLE
			std::vector<double> operand_values(SIZE_STATE_SPACE);
			for (uint32_t i = 0; i < SIZE_STATE_SPACE; i++) {
				presult.set_raw_bits(distr(eng));  // take the bottom nbits bits as posit encoding
				operand_values[i] = double(presult);
			}
			double da, db;
#endif // POSIT_USE_LONG_DOUBLE
			unsigned ia, ib;  // random indices for picking operands to test
			for (unsigned i = 1; i < nrOfRandoms; i++) {
				ia = std::rand() % SIZE_STATE_SPACE;
				da = operand_values[ia];
				pa = da;
				ib = std::rand() % SIZE_STATE_SPACE;
				db = operand_values[ib];
				pb = db;
				// in case you have numeric_limits<long double>::digits trouble... this will show that
				//std::cout << "sizeof da: " << sizeof(da) << " bits in significant " << (std::numeric_limits<long double>::digits - 1) << " value da " << da << " at index " << ia << " pa " << pa << std::endl;
				//std::cout << "sizeof db: " << sizeof(db) << " bits in significant " << (std::numeric_limits<long double>::digits - 1) << " value db " << db << " at index " << ia << " pa " << pb << std::endl;
				execute(opcode, da, db, pa, pb, preference, presult);
				if (presult != preference) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases) ReportBinaryArithmeticErrorInBinary("FAIL", operation_string, pa, pb, preference, presult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccessInBinary("PASS", operation_string, pa, pb, preference, presult);
				}
			}

			return nrOfFailedTests;
		}


		template<size_t nbits, size_t es>
		void GeneratePerformanceReport(OperatorPerformance &report) {
			using namespace std;
			using namespace std::chrono;
			int positives, negatives;

			steady_clock::time_point begin, end;
			duration<double> time_span;
			double elapsed;

			begin = steady_clock::now();
			    MeasureConversionPerformance<nbits, es>(positives, negatives);
			end = steady_clock::now();
			time_span = duration_cast<duration<double>>(end - begin);
			elapsed = time_span.count();
			report.convert = float((positives + negatives) / elapsed);

			begin = steady_clock::now();
			    MeasurePrefixPerformance<nbits, es>(positives, negatives);
			end = steady_clock::now();
			 time_span = duration_cast<duration<double>>(end - begin);
			elapsed = time_span.count();
			report.prefix = float((positives + negatives) / elapsed);

			begin = steady_clock::now();
			    MeasurePostfixPerformance<nbits, es>(positives, negatives);
			end = steady_clock::now();
			time_span = duration_cast<duration<double>>(end - begin);
			elapsed = time_span.count();
			report.postfix = float((positives + negatives) / elapsed);

			begin = steady_clock::now();
			    MeasureNegationPerformance<nbits, es>(positives, negatives);
			end = steady_clock::now();
			time_span = duration_cast<duration<double>>(end - begin);
			elapsed = time_span.count();
			report.neg = float((positives + negatives) / elapsed);

			begin = steady_clock::now();
			    MeasureSqrtPerformance<nbits, es>(positives, negatives);
			end = steady_clock::now();
			time_span = duration_cast<duration<double>>(end - begin);
			elapsed = time_span.count();
			report.sqrt = float((positives + negatives) / elapsed);

			begin = steady_clock::now();
			    MeasureAdditionPerformance<nbits, es>(positives, negatives);
			end = steady_clock::now();
			time_span = duration_cast<duration<double>>(end - begin);
			elapsed = time_span.count();
			report.add = float((positives + negatives) / elapsed);

			begin = steady_clock::now();
			    MeasureSubtractionPerformance<nbits, es>(positives, negatives);
			end = steady_clock::now();
			time_span = duration_cast<duration<double>>(end - begin);
			elapsed = time_span.count();
			report.sub = float((positives + negatives) / elapsed);

			begin = steady_clock::now();
			    MeasureMultiplicationPerformance<nbits, es>(positives, negatives);
			end = steady_clock::now();
			time_span = duration_cast<duration<double>>(end - begin);
			elapsed = time_span.count();
			report.mul = float((positives + negatives) / elapsed);

			begin = steady_clock::now();
			    MeasureDivisionPerformance<nbits, es>(positives, negatives);
			end = steady_clock::now();
			time_span = duration_cast<duration<double>>(end - begin);
			elapsed = time_span.count();
			report.div = float((positives + negatives) / elapsed);
		}
	} // namespace unum

} // namespace sw

