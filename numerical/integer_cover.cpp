// integer_cover.cpp: covering the integers with a posit
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#define ALIASING_ALLOWED
#include "common.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

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
	bool bReportIndividualTestCases = true;
	std::string tag = "TwoSum failed: ";

	// preserve the existing ostream precision
	auto precision = cout.precision();
	cout << setprecision(12);

	cout << "Posit Integer Cover" << endl;

#if MANUAL_TEST

		cout << "posit<16,1>: 2^10 integer cover is : " << CalculateIntegerCover<16, 1,10>() << "%\n";
		cout << "posit<17,1>: 2^10 integer cover is : " << CalculateIntegerCover<17, 1, 10>() << "%\n";
		cout << "posit<18,1>: 2^10 integer cover is : " << CalculateIntegerCover<18, 1, 10>() << "%\n";
		cout << "posit<19,1>: 2^10 integer cover is : " << CalculateIntegerCover<19, 1, 10>() << "%\n";
		cout << "posit<20,1>: 2^10 integer cover is : " << CalculateIntegerCover<20, 1, 10>(true) << "%\n";
		cout << "posit<24,1>: 2^10 integer cover is : " << CalculateIntegerCover<24, 1, 10>(true) << "%\n";
		cout << "posit<28,1>: 2^10 integer cover is : " << CalculateIntegerCover<28, 1, 10>(true) << "%\n";
		cout << "posit<32,1>: 2^10 integer cover is : " << CalculateIntegerCover<32, 1, 10>(true) << "%\n";
		cout << "posit<32,2>: 2^10 integer cover is : " << CalculateIntegerCover<32, 2, 10>(true) << "%\n";
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
