//  quires.cpp : test suite for quires
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"

// type definitions for the important types, posit<> and quire<>
#include "../../posit/posit.hpp"
#include "../../posit/posit_manipulators.hpp"    // pretty_print
#include "../../posit/quire.hpp"
// test support functions
#include "../tests/quire_test_helpers.hpp"


template<size_t nbits, size_t es, size_t capacity>
void GenerateTestCase(int input, const sw::unum::quire<nbits, es, capacity>& reference, const sw::unum::quire<nbits, es, capacity>& qresult) {

	std::cout << std::endl;
}

template<size_t nbits, size_t es>
void Issue45() {
	using ScalarType = sw::unum::posit<nbits, es>;
	using magnitude = sw::unum::posit<nbits, es>;
	using positX = sw::unum::posit<nbits, es>;
	using quireX = sw::unum::quire<nbits, es, 10>;
	using valueX = sw::unum::value<2*(nbits - 2 - es)>;

	constexpr int n = 64;
	std::vector<positX> Acoefficients(n);
	for (int i = 0; i < n; ++i) {
		Acoefficients[i] = sw::unum::minpos<nbits, es>();
	}
	std::vector<positX> xcoefficients(n);
	for (int i = 0; i < n; ++i) {
		xcoefficients[i] = 1.0f;
	}
	std::vector<positX> ycoefficients(n);
	//const LocalOrdinalType* Arowoffsets = &A.row_offsets[0];
	const ScalarType* Acoefs = &Acoefficients[0];
	const ScalarType* xcoefs = &xcoefficients[0];
	ScalarType* ycoefs = &ycoefficients[0];
	ScalarType beta = 0;

	magnitude result;
	quireX resultAsQuire;
	valueX resultValueX(0.0f);
	resultAsQuire = resultValueX;

	for (int row = 0; row < 1; ++row) {
		quireX sum;
		valueX sumVal(0.0f);
		sum = sumVal;

		for (int i = 0; i < n; ++i) {
			valueX addend = sw::unum::quire_mul(Acoefficients[i], xcoefficients[i]);
			sum += addend;
			std::cout << components(addend) << "\n" << sum << std::endl;
		}
		positX tempSum;
		tempSum.convert(sum.to_value());
		ycoefs[row] = tempSum;
		valueX resultValueX = sw::unum::quire_mul(xcoefs[row], tempSum);
		resultAsQuire += resultValueX;

	}
	result.convert(resultAsQuire.to_value());
	std::cout << "result: " << result << std::endl;
}

/*

taking 5.05447e-05 += quire_mul(-0.0165405, 0.000999451) (which equals -1.65314e-05)
(-,-16,00010101010110100000000000)
1: 000000000000000000000000000000_000000000000000000000000000000000000000000000000000000000.00000000000000100100000100000001100000000000000000000000
Row = 266, i = 5338, tempValue after += 3.43323e-05

taking 3.43323e-05 += quire_mul(-0.00828552, 0.000999451) (which equals -8.28097e-06)
(-,-17,00010101110111010000000000)
1: 000000000000000000000000000000_000000000000000000000000000000000000000000000000000000000.00000000000000011011011000010011000000000000000000000000
Row = 266, i = 5339, tempValue after += 2.6226e-05

----------------------------------------------------------------------------------------------------------------------

taking 2.6226e-05 += quire_mul(-0.016571, 0.000999451) (which equals -1.65619e-05)
(-,-16,00010101110111010000000000)
-1: 111111111111111111111111111111_111111111111111111111111111111111111111111111111111111111.11111111111111110101111111001010000000000000000000000000
Row = 266, i = 5340, tempValue after += -2.68435e+08
----------------------------------------------------------------------------------------------------------------------

Row = 266, i = 5341, tempValue = -2.68435e+08
taking -2.68435e+08 += quire_mul(-0.00828552, 0.000999451) (which equals -8.28097e-06)
(-,-17,00010101110111010000000000)
-1: 111111111111111111111111111111_111111111111111111111111111111111111111111111111111111111.11111111111111111110101010111000100000000000000000000000
*/
template<size_t nbits, size_t es, size_t capacity = 30>
void Issue45_2() {
	using namespace std;
	using namespace sw::unum;

	constexpr size_t mbits = 2 * (nbits - 2 - es);
	sw::unum::quire<nbits, es, capacity> q, q_base;
	sw::unum::value<mbits> unrounded, q_value;
	sw::unum::bitblock<mbits> fraction;

	//  quire_bits = "+:000000000000000000000000000000_000000000000000000000000000000000000000000000000000000000.00000000000000100100000100000001100000000000000000000000";
	//	quire_bits = "+:000000000000000000000000000000_000000000000000000000000000000000000000000000000000000000.00000000000000011011011000010011000000000000000000000000";
	//	quire_bits = "-:111111111111111111111111111111_111111111111111111111111111111111111111111111111111111111.11111111111111110101111111001010000000000000000000000000";
	//	quire_bits = "-:111111111111111111111111111111_111111111111111111111111111111111111111111111111111111111.11111111111111111110101010111000100000000000000000000000";

	fraction.load_bits("00010101010110100000000000");
	//unrounded.set(true, -16, fraction, false, false, false);  // (-,-16,00010101010110100000000000)

	std::string quire_bits;
	quire_bits = "+:000000000000000000000000000000_000000000000000000000000000000000000000000000000000000000.00000000000000100100000100000001100000000000000000000000";
	q.load_bits(quire_bits);
	//std::cout << quire_bits << std::endl;
	//std::cout << q << std::endl;
	fraction.load_bits("00010101110111010000000000");
	unrounded.set(true, -17, fraction, false, false, false);  // (-, -17, 00010101110111010000000000)
	q += unrounded; q_base += unrounded;
	std::cout << q_base << " <--- q_base" << std::endl;
	std::cout << q << std::endl;
	quire_bits = "+:000000000000000000000000000000_000000000000000000000000000000000000000000000000000000000.00000000000000011011011000010011000000000000000000000000";
	std::cout << quire_bits << " <--- debug reference" << std::endl;

	q_base.clear();
	fraction.load_bits("00010101110111010000000000");
	unrounded.set(true, -16, fraction, false, false, false);  // (-,-16,00010101110111010000000000)

	q += unrounded; q_base += unrounded;
	std::cout << q_base << " <--- q_base" << std::endl;
	std::cout << q << std::endl;
	quire_bits = "-:111111111111111111111111111111_111111111111111111111111111111111111111111111111111111111.11111111111111110101111111001010000000000000000000000000";
	std::cout << quire_bits << " <--- debug reference" << std::endl;

	q_base.clear();
	fraction.load_bits("00010101110111010000000000");
	unrounded.set(true, -17, fraction, false, false, false);  // (-,-17,00010101110111010000000000)
	q += unrounded; q_base += unrounded;
	std::cout << q_base << " <--- q_base" << std::endl;
	std::cout << q << std::endl;
	quire_bits = "-:111111111111111111111111111111_111111111111111111111111111111111111111111111111111111111.11111111111111111110101010111000100000000000000000000000";
	std::cout << quire_bits << " <--- debug reference" << std::endl;


	quire_bits = "+:000000000000000000000000000000_000000000000000000000000000000000000000000000000000000000.00000000000000011011011000010011000000000000000000000000";
	quire_bits = "-:000000000000000000000000000000_000000000000000000000000000000000000000000000000000000000.00000000000000010001010111011101000000000000000000000000";

	std::cout << std::endl << std::endl;
	q_base.clear();
	quire_bits = "+:000000000000000000000000000000_000000000000000000000000000000000000000000000000000000000.00000000000000001000000000010000000000000000000000000000";
	q.load_bits(quire_bits);
	std::cout << q << " <---- starting value" << std::endl;
	fraction.load_bits("00000000000111000000000000");
	unrounded.set(true, -17, fraction, false, false, false);  // (-, -17, 00000000000111000000000000)
	q += unrounded; q_base += unrounded;
	std::cout << q_base << " <--- q_base" << std::endl;
	std::cout << q << std::endl;

	std::cout << std::endl << std::endl;
	q_base.clear();
	quire_bits = "+:000000000000000000000000000000_000000000000000000000000000000000000000000000000000000000.01000000000000000000000000000000000000000000000000000000";
	q.load_bits(quire_bits);
	std::cout << q << " <---- starting value" << std::endl;
	fraction.load_bits("11000000000000000000000000");
	unrounded.set(true, -3, fraction, false, false, false); 
	q += unrounded; q_base += unrounded;
	std::cout << q_base << " <--- q_base" << std::endl;
	std::cout << q << std::endl;

	std::cout << std::endl << std::endl;
	q_base.clear();
	quire_bits = "+:000000000000000000000000000000_000000000000000000000000000000000000000000000000000000000.00000010000000000000000000000000000000000000000000000000";
	q.load_bits(quire_bits);
	std::cout << q << " <---- starting value" << std::endl;
	fraction.load_bits("11000000000000000000000000");
	unrounded.set(true, -8, fraction, false, false, false);
	q += unrounded; q_base += unrounded;
	std::cout << q_base << " <--- q_base" << std::endl;
	std::cout << q << std::endl;

	std::cout << std::endl << std::endl;
	q_base.clear();
	quire_bits = "+:000000000000000000000000000000_000000000000000000000000000000000000000000000000000000000.00000000000100000000000000000000000000000000000000000000";
	q.load_bits(quire_bits);
	std::cout << q << " <---- starting value" << std::endl;
	fraction.load_bits("11000000000000000000000000");
	unrounded.set(true, -13, fraction, false, false, false);
	q += unrounded; q_base += unrounded;
	std::cout << q_base << " <--- q_base" << std::endl;
	std::cout << q << std::endl;

	std::cout << std::endl << std::endl;
	q_base.clear();
	quire_bits = "+:000000000000000000000000000000_000000000000000000000000000000000000000000000000000000000.00000000000000001000000000000000000000000000000000000000";
	q.load_bits(quire_bits);
	std::cout << q << " <---- starting value" << std::endl;
	fraction.load_bits("11000000000000000000000000");
	unrounded.set(true, -18, fraction, false, false, false);
	q += unrounded; q_base += unrounded;
	std::cout << q_base << " <--- q_base" << std::endl;
	std::cout << q << std::endl;

	std::cout << std::endl << std::endl;
	q_base.clear();
	quire_bits = "+:000000000000000000000000000000_000000000000000000000000000000000000000000000000000000000.00000000000000000000010000000000000000000000000000000000";
	q.load_bits(quire_bits);
	std::cout << q << " <---- starting value" << std::endl;
	fraction.load_bits("11000000000000000000000000");
	unrounded.set(true, -23, fraction, false, false, false);
	q += unrounded; q_base += unrounded;
	std::cout << q_base << " <--- q_base" << std::endl;
	std::cout << q << std::endl;

	std::cout << std::endl << std::endl;
	q_base.clear();
	quire_bits = "+:000000000000000000000000000000_000000000000000000000000000000000000000000000000000000000.00000000000000000000000000100000000000000000000000000000";
	q.load_bits(quire_bits);
	std::cout << q << " <---- starting value" << std::endl;
	fraction.load_bits("11000000000000000000000000");
	unrounded.set(true, -28, fraction, false, false, false);
	q += unrounded; q_base += unrounded;
	std::cout << q_base << " <--- q_base" << std::endl;
	std::cout << q << std::endl;

	std::cout << std::endl << std::endl;
	q_base.clear();
	quire_bits = "+:000000000000000000000000000000_000000000000000000000000000000000000000000000000000000000.00000000000000000000000000010000000000000000000000000000";
	q.load_bits(quire_bits);
	std::cout << q << " <---- starting value" << std::endl;
	fraction.load_bits("11000000000000000000000000");
	unrounded.set(true, -29, fraction, false, false, false);
	q += unrounded; q_base += unrounded;
	std::cout << q_base << " <--- q_base" << std::endl;
	std::cout << q << std::endl;

	std::cout << std::endl << std::endl;
	q_base.clear();
	quire_bits = "+:000000000000000000000000000000_000000000000000000000000000000000000000000000000000000000.00000000000000000000000000001000000000000000000000000000";
	q.load_bits(quire_bits);
	std::cout << q << " <---- starting value" << std::endl;
	fraction.load_bits("11000000000000000000000000");
	unrounded.set(true, -30, fraction, false, false, false);
	q += unrounded; q_base += unrounded;
	std::cout << q_base << " <--- q_base" << std::endl;
	std::cout << q << std::endl;

	{
		// inefficient as we are copying a whole quire just to reset the sign bit, but we are leveraging the comparison logic
		//quire<nbits, es, capacity> absq = abs(*this);
		constexpr size_t qbits = (size_t(1) << es) * (4 * nbits - 8) + capacity;
		constexpr size_t fbits = nbits - 3 - es;
		//value<qbits> absq = abs(q);
		quire <nbits, es, capacity> absq = abs(q);
		value<mbits> absv = abs(unrounded);
		if (absq < absv) {
			cout << "q < v" << endl;
		}
		else if (absq > absv) {
			cout << "q > v" << endl;
		}
		else {
			cout << "q == v" << endl;
		}
	}

}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	using namespace std;
	using namespace sw::unum;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Quire Accumulation failed";

#if MANUAL_TESTING


	{
		float v = 2.6226e-05f;
		sw::unum::quire<16, 1, 2> q;
		sw::unum::posit<16, 1> p1, p2, argA, argB;

		p1 = v;
		q = p1.to_value();
		convert(q.to_value(), p2);
		argA = -0.016571;
		argB = 0.000999451;
		float diff = v - float(p1);
		std::cout << "diff       = " << setprecision(17) << diff << std::endl;

		std::cout << "quire      = " << q << std::endl;
		std::cout << "v as posit = " << pretty_print(p1) << std::endl;
		std::cout << "q as posit = " << p2 << std::endl;
		q += quire_mul(argA, argB);
		std::cout << "quire      = " << q << std::endl;
		convert(q.to_value(), p2);
		std::cout << "q as posit = " << p2 << std::endl;
	}


	const size_t nbits = 4;
	const size_t es = 1;
	const size_t capacity = 2; // for testing the accumulation capacity of the quire can be small
	const size_t fbits = 5;

	//GenerateUnsignedIntAssignments<nbits, es, capacity>();
	//GenerateSignedIntAssignments<nbits, es, capacity>();
	//GenerateUnsignedIntAssignments<8, 2, capacity>();

	GenerateValueAssignments<nbits, es, capacity, fbits>();

	std::cout << endl;
	std::cout << "Nothing prohibiting us from creating quires for float and double arithmetic" << std::endl;
	float f = 1.555555555555e-10f;
	value<23> vf(f);
	quire<10, 2, 2> fquire;
	fquire += vf;
	std::cout << "float:  " << setw(15) << f << " " << fquire << std::endl;

	double d = 1.555555555555e16;
	value<52> vd(d);
	quire<10, 2, 2> dquire;
	dquire += vd;
	std::cout << "double: " << setw(15) << d << " " << dquire << std::endl;

	/* pattern to use posits with a quire
	posit<10, 2> p = 1.555555555555e16;
	quire<10, 2, 2> pquire(p.convert_to_scientific_notation());
	std::cout << "posit:  " << setw(15) << d << " " << dquire << std::endl;
	*/

	std::cout << std::endl;
	// nbits = 4, es = 1, capacity = 2
	//  17 16   15 14 13 12 11 10  9  8    7  6  5  4  3  2  1  0
	// [ 0  0    0  0  0  0  0  0  0  0    0  0  0  0  0  0  0  0 ]
	quire<nbits, es, capacity> q;
	value<5> maxpos, maxpos_squared, minpos, minpos_squared;
	long double dmax = sw::unum::maxpos_value<nbits, es>();
	maxpos = dmax;
	maxpos_squared = dmax*dmax;
	std::cout << "maxpos * maxpos = " << components(maxpos_squared) << std::endl;
	long double dmin = sw::unum::minpos_value<nbits, es>();
	minpos = dmin;
	minpos_squared = dmin*dmin;
	std::cout << "minpos * minpos = " << components(minpos_squared) << std::endl;
	value<5> c(maxpos_squared);

	std::cout << "Add/Subtract propagating carry/borrows to and from capacity segment" << std::endl;
	q.clear();
	value<5> v(64);
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << " <- entering capacity bits" << std::endl;
	q += c;		std::cout << q << " <- adding maxpos^2" << std::endl;
	q += c;     std::cout << q << " <- flipping another capacity bit" << std::endl;
	q += -c;	std::cout << q << " <- subtracting maxpos^2" << std::endl;
	q += -c;	std::cout << q << " <- subtracting maxpos^2" << std::endl;
	q += -v;	std::cout << q << " <- removing the capacity bit" << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << " <- should be zero" << std::endl;

	std::cout << "Add/Subtract propagating carry/borrows across lower/upper accumulators" << std::endl;
	q.clear();
	v = 0.5;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << " <- should be zero" << std::endl;

	std::cout << "Add/Subtract propagating carry/borrows across lower/upper accumulators" << std::endl;
	q.clear();  // equivalent to q = 0, but more articulate/informative
	v = 3.875 + 0.0625; std::cout << "v " << components(v) << std::endl; // the input value is 11.1111 so hidden + 5 fraction bits
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << " <- should be zero" << std::endl;


	std::cout << std::endl;

#if 0
	{
		Issue45_2<16, 1, 30>();
	}
#endif

#else

	std::cout << "Quire validation" << std::endl;
	TestQuireAccumulationResult(ValidateQuireAccumulation<8,0,5>(), "quire<8,0,5>");

#ifdef STRESS_TESTING


#endif // STRESS_TESTING


#endif // MANUAL_TESTING
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
