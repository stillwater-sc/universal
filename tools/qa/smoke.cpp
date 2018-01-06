// smoke.cpp: generate smoke tests
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"
#include <chrono>
#include <ctime>

#include <posit>
#include "../tests/posit_test_helpers.hpp"

using namespace std;

/*
smoke tests focus on the boundary cases of posit arithmetic.

There are four regions where the number of exponent bits vary
 */

namespace sw {
	namespace qa {
		template<size_t nbits, size_t es>
		int Compare(double input, const sw::unum::posit<nbits, es>& presult, double reference, bool bReportIndividualTestCases) {
			int fail = 0;
			double result = presult.to_double();
			if (fabs(result - reference) > 0.000000001) {
				fail++;
				if (bReportIndividualTestCases)	ReportConversionError("FAIL", "=", input, reference, presult);
			}
			else {
				if (bReportIndividualTestCases) ReportConversionSuccess("PASS", "=", input, reference, presult);
			}
			return fail;
		}


		template<size_t nbits, size_t es>
		int SmokeTestConversion(std::string tag, bool bReportIndividualTestCases) {
			// we are going to generate a test set that consists of all edge case posit configs and their midpoints
			// we do this by enumerating a posit that is 1-bit larger than the test posit configuration

			// there are four quadrants, each with two endpoints
			// south-east  -> [minpos -   1.0)
			// north-east  -> (1.0    -   maxpos)
			// north-west  -> [-maxpos - -1.0)
			// south-west  -> (-1.0    - -minpos)

			// on the minpos/maxpos side there are 2* 2^(es+1) patterns that carry special rounding behavior
			// es = 0:   0, minpos                           ->  2 special cases
			// es = 1:   0, minpos, 2 exponent configs       ->  4 special cases
			// es = 2:   0, minpos, 2, 4 exponent configs    ->  8 special cases
			// es = 3:   0/minpos, 2, 4, 8 exponent configs  -> 16 special cases
			// es = 4:   0/minpos, 2, 4, 8, 16 exp configs   -> 32 special cases
			// -> 2^(es+1) special cases
			constexpr size_t cases = size_t(1) << (es + 2);
			int64_t test_patterns[cases];
			for (int64_t i = 0; i < cases; i++) {
				test_patterns[i] = i;
			}
			// generate those patterns
			const int64_t NR_TEST_CASES = cases;
			const int64_t HALF = cases + 1;
			sw::unum::posit<nbits + 1, es> pref, pprev, pnext;

			// execute the test
			int nrOfFailedTests = 0;
			double minpos = sw::unum::minpos_value<nbits + 1, es>();
			double eps;
			double da, input;
			sw::unum::posit<nbits, es> pa;
			for (int64_t i = 0; i < NR_TEST_CASES; i++) {
				pref.set_raw_bits(i);
				cout << pref << endl;

				da = pref.to_double();
				if (i == 0) {
					eps = minpos / 2.0;
				}
				else {
					eps = da > 0 ? da * 1.0e-6 : da * -1.0e-6;
				}
				if (i % 2) {
					if (i == 1) {
						// special case of projecting to +minpos
						// even the -delta goes to +minpos
						input = da - eps;
						pa = input;
						pnext.set_raw_bits(i + 1);
						nrOfFailedTests += sw::qa::Compare(input, pa, pnext.to_double(), bReportIndividualTestCases);
						input = da + eps;
						pa = input;
						nrOfFailedTests += sw::qa::Compare(input, pa, pnext.to_double(), bReportIndividualTestCases);

					}
					else if (i == HALF - 1) {
						// special case of projecting to +maxpos
						input = da - eps;
						pa = input;
						pprev.set_raw_bits(HALF - 2);
						nrOfFailedTests += sw::qa::Compare(input, pa, pprev.to_double(), bReportIndividualTestCases);
					}
					else if (i == HALF + 1) {
						// special case of projecting to -maxpos
						input = da - eps;
						pa = input;
						pprev.set_raw_bits(HALF + 2);
						nrOfFailedTests += sw::qa::Compare(input, pa, pprev.to_double(), bReportIndividualTestCases);
					}
					else if (i == NR_TEST_CASES - 1) {
						// special case of projecting to -minpos
						// even the +delta goes to -minpos
						input = da - eps;
						pa = input;
						pprev.set_raw_bits(i - 1);
						nrOfFailedTests += sw::qa::Compare(input, pa, pprev.to_double(), bReportIndividualTestCases);
						input = da + eps;
						pa = input;
						nrOfFailedTests += sw::qa::Compare(input, pa, pprev.to_double(), bReportIndividualTestCases);
					}
					else {
						// for odd values, we are between posit values, so we create the round-up and round-down cases
						// round-down
						input = da - eps;
						pa = input;
						pprev.set_raw_bits(i - 1);
						nrOfFailedTests += sw::qa::Compare(input, pa, pprev.to_double(), bReportIndividualTestCases);
						// round-up
						input = da + eps;
						pa = input;
						pnext.set_raw_bits(i + 1);
						nrOfFailedTests += sw::qa::Compare(input, pa, pnext.to_double(), bReportIndividualTestCases);
					}
				}
				else {
					// for the even values, we generate the round-to-actual cases
					if (i == 0) {
						// special case of projecting to +minpos
						input = da + eps;
						pa = input;
						pnext.set_raw_bits(i + 2);
						nrOfFailedTests += sw::qa::Compare(input, pa, pnext.to_double(), bReportIndividualTestCases);
					}
					else if (i == NR_TEST_CASES - 2) {
						// special case of projecting to -minpos
						input = da - eps;
						pa = input;
						pprev.set_raw_bits(NR_TEST_CASES - 2);
						nrOfFailedTests += sw::qa::Compare(input, pa, pprev.to_double(), bReportIndividualTestCases);
					}
					else {
						// round-up
						input = da - eps;
						pa = input;
						nrOfFailedTests += sw::qa::Compare(input, pa, da, bReportIndividualTestCases);
						// round-down
						input = da + eps;
						pa = input;
						nrOfFailedTests += sw::qa::Compare(input, pa, da, bReportIndividualTestCases);
					}
				}
			}
			return nrOfFailedTests;
		}

		template<size_t nbits, size_t es>
		int SmokeTestMultiplication(std::string tag, bool bReportIndividualTestCases) {
			int nrOfFailedTests = 0;
			const size_t NR_POSITS = (unsigned(1) << nbits);

			sw::unum::posit<nbits, es> pa, pb, pmul, pref;
			double da, db;
			for (int i = 0; i < NR_POSITS; i++) {
				pa.set_raw_bits(i);
				da = pa.to_double();
				for (int j = 0; j < NR_POSITS; j++) {
					pb.set_raw_bits(j);
					db = pb.to_double();
					pmul = pa * pb;
					pref = da * db;
					if (fabs(pmul.to_double() - pref.to_double()) > 0.000000001) {
						if (bReportIndividualTestCases) ReportBinaryArithmeticError("FAIL", "*", pa, pb, pref, pmul);
						nrOfFailedTests++;
					}
					else {
						if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "*", pa, pb, pref, pmul);
					}
				}
			}
			return nrOfFailedTests;
		}

	}
}


int main(int argc, char** argv)
try {

	if (argc != 2) {
		cerr << "Generate smoke tests." << endl;
		cerr << "Usage: smoke operator" << endl;
		return EXIT_SUCCESS;   // signal successful completion for ctest
	}
	string operation = argv[1];
	cout << "Generating smoke tests for operation -" << operation << "-" << endl;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	float upper_limit = int64_t(1) << 17;
	using namespace std::chrono;
	steady_clock::time_point t1 = steady_clock::now();
	nrOfFailedTestCases = sw::qa::SmokeTestConversion<5, 2>("smoke testing", bReportIndividualTestCases);
	steady_clock::time_point t2 = steady_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	double elapsed = time_span.count();
	std::cout << "It took " << elapsed << " seconds." << std::endl;
	std::cout << "Performance " << (uint32_t)(upper_limit / (1000 * elapsed)) << " Ksamples/s" << std::endl;
	std::cout << std::endl;




	//nrOfFailedTestCases = sw::qa::SmokeTestMultiplication<5, 2>("smoke testing", bReportIndividualTestCases);

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
