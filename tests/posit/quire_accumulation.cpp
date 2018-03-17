//  quire_accumulations.cpp : computational path experiments with quires
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"

// set to 1 if you want to generate hw test vectors
#define HARDWARE_QA_OUTPUT 0

// type definitions for the important types, posit<> and quire<>
#include "../../posit/posit.hpp"
#include "../../posit/quire.hpp"
// test support functions
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"
#include "../tests/quire_test_helpers.hpp"

template<size_t nbits, size_t es>
void PrintTestVector(std::ostream& ostr, const std::vector< sw::unum::posit<nbits,es> >& pv) {
	for (typename std::vector< posit<nbits,es> >::const_iterator it = pv.begin(); it != pv.end(); it++) {
		ostr << *it << std::endl;
	}
}

template<size_t nbits, size_t es, size_t capacity>
int GenerateQuireAccumulationTestCase(bool bReportIndividualTestCases, size_t nrOfElements, const sw::unum::posit<nbits,es>& seed) {
	int nrOfFailedTestCases = 0;
	std::stringstream ss;
	ss << "quire<" << nbits << "," << es << "," << capacity << ">";
	std::vector< sw::unum::posit<nbits, es> > t = GenerateVectorForZeroValueFDP(nrOfElements, seed);
	nrOfFailedTestCases += ReportTestResult(sw::unum::ValidateQuireAccumulation<nbits, es, capacity>(bReportIndividualTestCases, t), ss.str(), "accumulation");
	return nrOfFailedTestCases;
}

int ValidateQuireMagnitudeComparison() {
	using namespace std;
	using namespace sw::unum;

	quire<16, 1, 2> q = 0xAAAA;
	value<20> v;
	v = 0xAAAB;
	cout << "quire: " << q << endl;
	cout << "value: " << v.get_fixed_point() << " " << components(v) << endl;
	cout << (q < v ? "correct" : "incorrect") << endl;
	cout << (q > v ? "incorrect" : "correct") << endl;
	v = 0xAAAA;
	cout << "value: " << v.get_fixed_point() << " " << components(v) << endl;
	cout << (q == v ? "correct" : "incorrect") << endl;
	return 0;
}

template<size_t nbits, size_t es, size_t capacity = 2>
int ValidateSignMagnitudeTransitions() {
	int nrOfFailedTestCases = 0;
	cout << "Quire configuration: quire<" << nbits << ", " << es << ", " << capacity << ">" << endl;

	// moving through the four quadrants of a sign/magnitue adder/subtractor
	posit<nbits, es> minpos, min2, min3, min4;
	minpos = sw::unum::minpos<nbits, es>();     // ...0001
	min2 = minpos; min2++;                  // ...0010
	min3 = minpos; min3++; min3++;          // ...0011
	min4 = minpos; min4++; min4++; min4++;  // ...0100
	posit<nbits, es> maxpos, max2, max3, max4;
	maxpos = maxpos_value<nbits, es>();     // 01..111
	max2 = maxpos; --max2;                  // 01..110
	max3 = max2; --max3;                    // 01..101
	max4 = max3; --max4;                    // 01..100

	cout << endl;
	cout << "Posit range extremes:" << endl;
	cout << "minpos         " << minpos.get() << " " << minpos << endl;
	cout << "min2           " << min2.get() << " " << min2 << endl;
	cout << "min3           " << min3.get() << " " << min3 << endl;
	cout << "min4           " << min4.get() << " " << min4 << endl;
	cout << "..." << endl;
	cout << "max4           " << max4.get() << " " << max4 << endl;
	cout << "max3           " << max3.get() << " " << max3 << endl;
	cout << "max2           " << max2.get() << " " << max2 << endl;
	cout << "maxpos         " << maxpos.get() << " " << maxpos << endl;

	cout << endl;

	cout << "Quire experiments: sign/magnitude transitions at the range extremes" << endl;

	quire<nbits, es, capacity> q;
	cout << q << "                                               <-- start at zero" << endl;
	// start in the positive, SE quadrant with minpos^2
	q += quire_mul(minpos, minpos);
	cout << q << " q += minpos^2  minpos = " << minpos.get() << " " << components(minpos.to_value()) << endl;
	// move to the negative SW quadrant by adding negative value that is bigger
	q += quire_mul(min2, -min2);
	cout << q << " q += min2^2    min2   = " << min2.get() << " " << components(min2.to_value()) << endl;
	// remove minpos^2 from the quire by subtracting it
	q -= quire_mul(minpos, minpos);
	cout << q << " q -= minpos^2  minpos = " << minpos.get() << " " << components(minpos.to_value()) << endl;
	// move back into posit, SE quadrant by adding the next bigger product
	q += quire_mul(min3, min3);
	cout << q << " q += min3^2    min3   = " << min3.get() << " " << components(min3.to_value()) << endl;
	// remove the min2^2 from the quire by subtracting it
	q -= quire_mul(min2, min2);
	cout << q << " q -= min2^2    min2   = " << min2.get() << " " << components(min2.to_value()) << endl;
	// add a -maxpos^2, to flip it again
	q += quire_mul(maxpos, -maxpos);
	cout << q << " q += -maxpos^2 maxpos = " << maxpos.get() << " " << components(maxpos.to_value()) << endl;
	// subtract min3^2 to propagate the carry
	q -= quire_mul(min3, min3);
	cout << q << " q -= min3^2    min3   = " << min3.get() << " " << components(min3.to_value()) << endl;
	// remove min2^2 remenants
	q += quire_mul(min2, min2);
	cout << q << " q += min2^2    min2   = " << min2.get() << " " << components(min2.to_value()) << endl;
	q += quire_mul(min2, min2);
	cout << q << " q += min2^2    min2   = " << min2.get() << " " << components(min2.to_value()) << endl;
	// borrow propagate
	q += quire_mul(minpos, minpos);
	cout << q << " q += minpos^2  minpos = " << minpos.get() << " " << components(minpos.to_value()) << endl;
	// flip the max3 bit
	q += quire_mul(max3, max3);
	cout << q << " q += max3^2    max3   = " << max3.get() << " " << components(max3.to_value()) << endl;
	// add maxpos^2 to be left with max3^2
	q += quire_mul(maxpos, maxpos);
	cout << q << " q += maxpos^2  maxpos = " << maxpos.get() << " " << components(maxpos.to_value()) << endl;
	// subtract max2^2 to flip the sign again
	q -= quire_mul(max2, max2);
	cout << q << " q -= max2^2    max2   = " << max2.get() << " " << components(max2.to_value()) << endl;
	// remove the max3^2 remenants
	q -= quire_mul(max3, max3);
	cout << q << " q -= max3^2    max3   = " << max3.get() << " " << components(max3.to_value()) << endl;
	// remove the minpos^2 bits
	q -= quire_mul(minpos, minpos);
	cout << q << " q -= minpos^2  minpos = " << minpos.get() << " " << components(minpos.to_value()) << endl;
	// add maxpos^2 to be left with max2^2 and flipped back to positive quadrant
	q += quire_mul(maxpos, maxpos);
	cout << q << " q += maxpos^2  maxpos = " << maxpos.get() << " " << components(maxpos.to_value()) << endl;
	// add max2^2 to remove its remenants
	q += quire_mul(max2, max2);
	cout << q << " q += max2^2    max2   = " << max2.get() << " " << components(max2.to_value()) << endl;
	// subtract minpos^2 to propagate the borrow across the quire
	q -= quire_mul(minpos, minpos);
	cout << q << " q -= minpos^2  minpos = " << minpos.get() << " " << components(minpos.to_value()) << endl;
	// subtract maxpos^2 to flip the sign and be left with minpos^2
	q -= quire_mul(maxpos, maxpos);
	cout << q << " q -= maxpos^2  maxpos = " << maxpos.get() << " " << components(maxpos.to_value()) << endl;
	// add minpos^2 to get to zero
	q += quire_mul(minpos, minpos);
	cout << q << " q += minpos^2  minpos = " << minpos.get() << " " << components(minpos.to_value()) << " <-- back to zero" << endl;

	return nrOfFailedTestCases;
}

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::unum;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	cout << "Quire experiments" << endl;

	std::string tag = "Quire Accumulation failed";

#if MANUAL_TESTING
	std::vector< posit<16, 1> > t;

//	t = GenerateVectorForZeroValueFDP(16, maxpos<16,1>());
//	PrintTestVector(cout, t);

#if 0
	quire<8, 1, 2> q;
	posit<8, 1> minpos = minpos_value<8, 1>();
	q += quire_mul(minpos, minpos);
	value<3> v3 = q.to_value().round_to<3>();
	value<5> v5 = q.to_value().round_to<5>();
	value<7> v7 = q.to_value().round_to<7>();
	cout << components(v3) << endl;
	cout << components(v5) << endl;
	cout << components(v7) << endl;

	// test correct handling of 0
	quire<8, 1, 2> q = 1;
	cout << q << endl;
	posit<8, 1> one = 1;
	posit<8, 1> aThird = 0.3333333333333333333333333333333333333333333;
	value< posit<8, 1>::mbits > mul = quire_mul(aThird, -one);
	cout << components(mul) << endl;
	q += quire_mul(aThird, -one);
	cout << q << endl;
	value<8> result = q.to_value().round_to<8>();
	cout << result << " " << components(result) << endl;
#endif

	nrOfFailedTestCases += ValidateSignMagnitudeTransitions<8, 1>();
	
	//nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 1, 2>(bReportIndividualTestCases, 16, minpos<8, 1>());

#else

	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 0, 2>(bReportIndividualTestCases, 16, minpos<8, 0>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 1, 2>(bReportIndividualTestCases, 16, minpos<8, 1>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 2, 2>(bReportIndividualTestCases, 16, minpos<8, 2>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 0, 5>(bReportIndividualTestCases, 16, maxpos<8, 0>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 1, 5>(bReportIndividualTestCases, 16, maxpos<8, 1>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 2, 5>(bReportIndividualTestCases, 16, maxpos<8, 2>());

	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<16, 0, 2>(bReportIndividualTestCases, 256, minpos<16, 0>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<16, 1, 2>(bReportIndividualTestCases, 256, minpos<16, 1>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<16, 2, 2>(bReportIndividualTestCases, 256, minpos<16, 2>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<16, 0, 5>(bReportIndividualTestCases, 16, maxpos<16, 0>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<16, 1, 5>(bReportIndividualTestCases, 16, maxpos<16, 1>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<16, 2, 5>(bReportIndividualTestCases, 16, maxpos<16, 2>());

	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<24, 0, 2>(bReportIndividualTestCases, 4096, minpos<24, 0>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<24, 1, 2>(bReportIndividualTestCases, 4096, minpos<24, 1>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<24, 2, 2>(bReportIndividualTestCases, 4096, minpos<24, 2>());

	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<32, 0, 2>(bReportIndividualTestCases, 65536, minpos<32, 0>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<32, 1, 2>(bReportIndividualTestCases, 65536, minpos<32, 1>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<32, 2, 2>(bReportIndividualTestCases, 65536, minpos<32, 2>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<32, 0, 5>(bReportIndividualTestCases, 16, maxpos<32, 0>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<32, 1, 5>(bReportIndividualTestCases, 16, maxpos<32, 1>());
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<32, 2, 5>(bReportIndividualTestCases, 16, maxpos<32, 2>());

#ifdef STRESS_TESTING


#endif // STRESS_TESTING


#endif // MANUAL_TESTING
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
