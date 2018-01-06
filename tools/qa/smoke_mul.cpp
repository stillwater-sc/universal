// smoke_mul.cpp: generate smoke tests for multiplication
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

#include <limits>

namespace sw {
	namespace qa {

#if 0
			// there are four quadrants, each with two endpoints
			// south-east  -> [minpos -   1.0)
			// north-east  -> (1.0    -   maxpos)
			// north-west  -> [-maxpos - -1.0)
			// south-west  -> (-1.0    - -minpos)

			// on each minpos/maxpos side there are 2^(es+1) patterns that carry special rounding behavior
			// es = 0:   0/minpos                            ->  2 special cases
			// es = 1:   0/minpos, 2 exponent configs        ->  4 special cases
			// es = 2:   0/minpos, 2, 4 exponent configs     ->  8 special cases
			// es = 3:   0/minpos, 2, 4, 8 exponent configs  -> 16 special cases
			// es = 4:   0/minpos, 2, 4, 8, 16 exp configs   -> 32 special cases
			// -> 2^(es+1) special cases
			//
			// plus the region around 1 that puts the most pressure on the conversion algorithm's precision
			// --1, 1, and 1++, so three extra cases per half.
			// Because we need to recognize the -minpos case, which happens to be all 1's, and is the last
			// test case in exhaustive testing, we need to have that test case end up in the last entry
			// of the test case array.
			constexpr size_t single_quadrant_cases = size_t(1) << (es + 2);
			constexpr size_t cases_around_plusminus_one = 6;
			constexpr size_t cases = cases_around_plusminus_one + 4 * single_quadrant_cases;
			// generate the special patterns
			uint64_t test_patterns[cases];
			// first patterns around +/- 1
			std::bitset<nbits+1> raw_bits;
			sw::unum::posit<nbits+1, es> p;  // need to generate them in the context of the posit that is nbits+1
			p = 1.0f; raw_bits = p.get(); cout << "raw bits for 1.0: " << raw_bits << " ull " << raw_bits.to_ullong() << endl;
			test_patterns[1] = raw_bits.to_ullong();
			p--; raw_bits = p.get(); cout << "raw bits for 1.0-eps: " << raw_bits << " ull " << raw_bits.to_ullong() << endl;
			test_patterns[0] = raw_bits.to_ullong();
			p = 1.0f;
			p++; raw_bits = p.get(); cout << "raw bits for 1.0+eps: " << raw_bits << " ull " << raw_bits.to_ullong() << endl;
			test_patterns[2] = raw_bits.to_ullong();
			p = -1.0f; raw_bits = p.get();
			test_patterns[4] = raw_bits.to_ullong();
			p--; raw_bits = p.get();
			test_patterns[3] = raw_bits.to_ullong();
			p = -1.0f;
			p++; raw_bits = p.get();
			test_patterns[5] = raw_bits.to_ullong();
			for (int64_t i = 0; i < single_quadrant_cases; i++) {
				test_patterns[i + cases_around_plusminus_one] = i;
			}
#if 0
			cout << "Generated test patterns" << endl;
			for (int i = 0; i < single_quadrant_cases + cases_around_plusminus_one; i++) {
				cout << "[" << setw(3) << i << "] = " << test_patterns[i] << endl;
			}
#endif
			const int64_t NR_TEST_CASES = cases_around_plusminus_one + single_quadrant_cases;
			const int64_t HALF = cases + 1;
			sw::unum::posit<nbits + 1, es> pref, pprev, pnext;
#endif

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
	typedef std::numeric_limits< double > dbl;
	cout << "double max digits " << dbl::max_digits10 << endl;

	cout << "Generating smoke tests for multiplication" << endl;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	float upper_limit = int64_t(1) << 17;
	using namespace std::chrono;
	steady_clock::time_point t1 = steady_clock::now();
	nrOfFailedTestCases = sw::qa::SmokeTestMultiplication<4,1>("smoke testing", bReportIndividualTestCases);
	steady_clock::time_point t2 = steady_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	double elapsed = time_span.count();
	std::cout << "It took " << elapsed << " seconds." << std::endl;
	std::cout << "Performance " << (uint32_t)(upper_limit / (1000 * elapsed)) << " Ksamples/s" << std::endl;
	std::cout << std::endl;

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
