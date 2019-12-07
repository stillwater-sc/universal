// integer_cover.cpp: covering the integers with a posit
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include "common.hpp"
// test helpers, such as, ReportTestResults
#include "../tests/utils/test_helpers.hpp"
#include "../tests/utils/posit_test_helpers.hpp"

/*
When using a discretization scheme, for example, an Analog-to-Digital converter, 
we have a set of integers that can be turned into fractions by normalizing to the
state space of possible samples. These sets can be projected to different ranges.
For example, a ADC channel generates values from 0 to (2^width - 1), and they can
represent a value range of -2^width-1,...,-1,0,1,... 2^width-1 -1.

We are interested to see how well a posit configuration can capture these
integer values, and how well they can capture the fractions when mapping to 
different regions.
*/

template<size_t nbits, size_t es, size_t adc_width>
double CalculateIntegerCover(bool verbose = false) {
	sw::unum::posit<nbits, es> pLevel;// , pFraction;

	constexpr unsigned long nrSamples = (unsigned long)1 << adc_width;
	unsigned long cover = 0;
	for (unsigned long level = 0; level < nrSamples; ++level) {
		pLevel = level;
		//pFraction = level / (double)nrSamples;
		unsigned long rounded = (unsigned long)pLevel;
		if (rounded == level) {
			++cover;
		}
		else {
			if (verbose) std::cout << "level = " << level << " rounded to " << rounded << "\n";
		}
	}
	return 100.0 * (double)cover / nrSamples;
}

#define MANUAL_TEST 1

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	// print detailed bit-level computational intermediate results
	// bool verbose = false;

	int nrOfFailedTestCases = 0;
	// bool bReportIndividualTestCases = true;
	std::string tag = "TwoSum failed: ";

	// preserve the existing ostream precision
	auto precision = cout.precision();
	cout << setprecision(12);

	cout << "Posit Integer Cover" << endl;

#if MANUAL_TEST
	cout << "8-bit ADC sample coverage\n";
	cout << "posit<12,0>: 2^8 integer cover is : " << CalculateIntegerCover<12, 0, 8>() << "%\n";
	cout << "posit<12,1>: 2^8 integer cover is : " << CalculateIntegerCover<12, 1, 8>() << "%\n";
	cout << "posit<12,2>: 2^8 integer cover is : " << CalculateIntegerCover<12, 2, 8>() << "%\n";

	cout << "posit<13,0>: 2^8 integer cover is : " << CalculateIntegerCover<13, 0, 8>() << "%\n";
	cout << "posit<13,1>: 2^8 integer cover is : " << CalculateIntegerCover<13, 1, 8>() << "%\n";
	cout << "posit<13,2>: 2^8 integer cover is : " << CalculateIntegerCover<13, 2, 8>() << "%\n";

	cout << "posit<14,0>: 2^8 integer cover is : " << CalculateIntegerCover<14, 0, 8>() << "%\n";
	cout << "posit<14,1>: 2^8 integer cover is : " << CalculateIntegerCover<14, 1, 8>() << "%\n";
	cout << "posit<14,1>: 2^8 integer cover is : " << CalculateIntegerCover<14, 2, 8>() << "%\n";

	cout << "posit<15,0>: 2^8 integer cover is : " << CalculateIntegerCover<15, 0, 8>() << "%\n";
	cout << "posit<15,1>: 2^8 integer cover is : " << CalculateIntegerCover<15, 1, 8>() << "%\n";	
	cout << "posit<15,2>: 2^8 integer cover is : " << CalculateIntegerCover<15, 2, 8>() << "%\n";

	cout << "posit<16,0>: 2^8 integer cover is : " << CalculateIntegerCover<16, 0, 8>() << "%\n";
	cout << "posit<16,1>: 2^8 integer cover is : " << CalculateIntegerCover<16, 1, 8>() << "%\n";

	cout << "10-bit ADC sample coverage\n";
	cout << "posit<16,1>: 2^10 integer cover is : " << CalculateIntegerCover<16, 1, 10>() << "%\n";
	cout << "posit<17,1>: 2^10 integer cover is : " << CalculateIntegerCover<17, 1, 10>() << "%\n";
	cout << "posit<18,1>: 2^12 integer cover is : " << CalculateIntegerCover<18, 1, 10>() << "%\n";
	cout << "posit<15,2>: 2^10 integer cover is : " << CalculateIntegerCover<15, 2, 10>() << "%\n";
	cout << "posit<16,2>: 2^10 integer cover is : " << CalculateIntegerCover<16, 2, 10>() << "%\n";
	cout << "posit<17,2>: 2^10 integer cover is : " << CalculateIntegerCover<17, 2, 10>() << "%\n";
	cout << "posit<18,2>: 2^10 integer cover is : " << CalculateIntegerCover<18, 2, 10>() << "%\n";

	cout << "12-bit ADC sample coverage\n";
	cout << "posit<18,1>: 2^12 integer cover is : " << CalculateIntegerCover<18, 1, 12>() << "%\n";
	cout << "posit<19,1>: 2^12 integer cover is : " << CalculateIntegerCover<19, 1, 12>() << "%\n";
	cout << "posit<20,1>: 2^12 integer cover is : " << CalculateIntegerCover<20, 1, 12>() << "%\n";

	cout << "14-bit ADC sample coverage\n";
	cout << "posit<20,2>: 2^14 integer cover is : " << CalculateIntegerCover<20, 2, 14>() << "%\n";
	cout << "posit<24,1>: 2^14 integer cover is : " << CalculateIntegerCover<24, 1, 14>() << "%\n";
	cout << "posit<28,1>: 2^14 integer cover is : " << CalculateIntegerCover<28, 1, 14>() << "%\n";

	cout << "16-bit ADC sample coverage\n";
	cout << "posit<20,1>: 2^16 integer cover is : " << CalculateIntegerCover<20, 1, 16>() << "%\n";
	cout << "posit<24,1>: 2^16 integer cover is : " << CalculateIntegerCover<24, 1, 16>() << "%\n";
	cout << "posit<28,1>: 2^16 integer cover is : " << CalculateIntegerCover<28, 1, 16>() << "%\n";
	cout << "posit<32,1>: 2^16 integer cover is : " << CalculateIntegerCover<32, 1, 16>() << "%\n";
	cout << "posit<32,2>: 2^16 integer cover is : " << CalculateIntegerCover<32, 2, 16>() << "%\n";
#else

#endif // MANUAL_TEST

	// restore the previous ostream precision
	cout << setprecision(precision);

	return nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
