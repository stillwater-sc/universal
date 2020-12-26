// linear_cover.cpp: covering a linear range with a posit
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include "common.hpp"
// pull in the posit number system
#include <universal/posit/posit>
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
double CalculateLinearCover(bool verbose = false) {
	sw::universal::posit<nbits, es> pLevel;// , pFraction;

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

// Generate a linear sample space within the posit encoding
template<size_t nbits, size_t es>
void GenerateLinearSamples() {
	// the linear range is going to be multiples of epsilon() around 1.0
	using namespace std;
	using namespace sw::universal;
	using Posit = posit<nbits, es>;

	Posit epsilon = numeric_limits<Posit>::epsilon();
	cout << "epsilon is " << epsilon << " " << color_print(epsilon) << endl;

	Posit p = 1;
	//while (!p.isnar()) {
	int count = 0;
	while (count < 40) {
		cout << color_print(p) << " : " << p << endl;
		p += epsilon; ++count;
	}
}

// Test a linear sample space within the posit encoding
template<size_t nbits, size_t es>
void TestLinearSamples() {
	// the linear range is going to be multiples of epsilon() around 1.0
	using namespace std;
	using namespace sw::universal;
	using Posit = posit<nbits, es>;

	Posit epsilon = numeric_limits<Posit>::epsilon();
	cout << "epsilon is " << epsilon << " " << color_print(epsilon) << endl;

	constexpr size_t NR_VALUES = (1 << nbits);
	Posit p;
	for (size_t i = 0; i < NR_VALUES; ++i) {
		p.set_raw_bits(i);
		cout << color_print(p) << " : " << setw(10) << p << " : ";
		Posit multiple = p / epsilon;
		if (floor(double(multiple)) == double(multiple)) {
			//if (multiple.isinteger()) {
			cout << multiple << " sample value : " << multiple * epsilon << endl;
		}
		else {
			cout << " : not a multiple of epsilon" << endl;
		}
	}
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	// preserve the existing ostream precision
	auto precision = cout.precision();
	cout << setprecision(12);

	cout << "Posit Linear Cover" << endl;

	GenerateLinearSamples<8, 0>();

/*
	cout << "8-bit ADC sample coverage\n";
	cout << "posit<12,0>: 2^8 integer cover is : " << CalculateLinearCover<12, 0, 8>() << "%\n";
	cout << "posit<12,1>: 2^8 integer cover is : " << CalculateLinearCover<12, 1, 8>() << "%\n";
	cout << "posit<12,2>: 2^8 integer cover is : " << CalculateLinearCover<12, 2, 8>() << "%\n";

	cout << "posit<13,0>: 2^8 integer cover is : " << CalculateLinearCover<13, 0, 8>() << "%\n";
	cout << "posit<13,1>: 2^8 integer cover is : " << CalculateLinearCover<13, 1, 8>() << "%\n";
	cout << "posit<13,2>: 2^8 integer cover is : " << CalculateLinearCover<13, 2, 8>() << "%\n";

	cout << "posit<14,0>: 2^8 integer cover is : " << CalculateLinearCover<14, 0, 8>() << "%\n";
	cout << "posit<14,1>: 2^8 integer cover is : " << CalculateLinearCover<14, 1, 8>() << "%\n";
	cout << "posit<14,1>: 2^8 integer cover is : " << CalculateLinearCover<14, 2, 8>() << "%\n";

	cout << "posit<15,0>: 2^8 integer cover is : " << CalculateLinearCover<15, 0, 8>() << "%\n";
	cout << "posit<15,1>: 2^8 integer cover is : " << CalculateLinearCover<15, 1, 8>() << "%\n";	
	cout << "posit<15,2>: 2^8 integer cover is : " << CalculateLinearCover<15, 2, 8>() << "%\n";

	cout << "posit<16,0>: 2^8 integer cover is : " << CalculateLinearCover<16, 0, 8>() << "%\n";
	cout << "posit<16,1>: 2^8 integer cover is : " << CalculateLinearCover<16, 1, 8>() << "%\n";

	cout << "10-bit ADC sample coverage\n";
	cout << "posit<16,1>: 2^10 integer cover is : " << CalculateLinearCover<16, 1, 10>() << "%\n";
	cout << "posit<17,1>: 2^10 integer cover is : " << CalculateLinearCover<17, 1, 10>() << "%\n";
	cout << "posit<18,1>: 2^12 integer cover is : " << CalculateLinearCover<18, 1, 10>() << "%\n";
	cout << "posit<15,2>: 2^10 integer cover is : " << CalculateLinearCover<15, 2, 10>() << "%\n";
	cout << "posit<16,2>: 2^10 integer cover is : " << CalculateLinearCover<16, 2, 10>() << "%\n";
	cout << "posit<17,2>: 2^10 integer cover is : " << CalculateLinearCover<17, 2, 10>() << "%\n";
	cout << "posit<18,2>: 2^10 integer cover is : " << CalculateLinearCover<18, 2, 10>() << "%\n";

	cout << "12-bit ADC sample coverage\n";
	cout << "posit<18,1>: 2^12 integer cover is : " << CalculateLinearCover<18, 1, 12>() << "%\n";
	cout << "posit<19,1>: 2^12 integer cover is : " << CalculateLinearCover<19, 1, 12>() << "%\n";
	cout << "posit<20,1>: 2^12 integer cover is : " << CalculateLinearCover<20, 1, 12>() << "%\n";

	cout << "14-bit ADC sample coverage\n";
	cout << "posit<20,2>: 2^14 integer cover is : " << CalculateLinearCover<20, 2, 14>() << "%\n";
	cout << "posit<24,1>: 2^14 integer cover is : " << CalculateLinearCover<24, 1, 14>() << "%\n";
	cout << "posit<28,1>: 2^14 integer cover is : " << CalculateLinearCover<28, 1, 14>() << "%\n";

	cout << "16-bit ADC sample coverage\n";
	cout << "posit<20,1>: 2^16 integer cover is : " << CalculateLinearCover<20, 1, 16>() << "%\n";
	cout << "posit<24,1>: 2^16 integer cover is : " << CalculateLinearCover<24, 1, 16>() << "%\n";
	cout << "posit<28,1>: 2^16 integer cover is : " << CalculateLinearCover<28, 1, 16>() << "%\n";
	cout << "posit<32,1>: 2^16 integer cover is : " << CalculateLinearCover<32, 1, 16>() << "%\n";
	cout << "posit<32,2>: 2^16 integer cover is : " << CalculateLinearCover<32, 2, 16>() << "%\n";
*/

	// restore the previous ostream precision
	cout << setprecision(precision);

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
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
