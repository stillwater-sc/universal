//  quire_accumulations.cpp : computational path experiments with quires
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// set to 1 if you want to generate hw test vectors
#define HARDWARE_QA_OUTPUT 0

// type definitions for the important types, posit<> and quire<>
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit1/posit1.hpp>
//#include <universal/traits/posit_traits.hpp>
//#include <universal/number/posit1/quire.hpp>
//#include <universal/number/posit1/fdp.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/quire_test_suite.hpp>
#include <universal/utility/convert_to.hpp>

// if you want to enable ISSUE_45
//#define ISSUE_45_DEBUG
#ifdef ISSUE_45_DEBUG
// forward reference
template<unsigned nbits, unsigned es, unsigned capacity> void Issue45_2();
#endif

template<unsigned nbits, unsigned es>
void PrintTestVector(std::ostream& ostr, const std::vector< sw::universal::posit<nbits,es> >& pv) {
	std::for_each (begin(pv), end(pv), [&ostr](const sw::universal::posit<nbits,es>& p){
		ostr << p << std::endl;
	});
}

template<unsigned nbits, unsigned es, unsigned capacity>
int GenerateQuireAccumulationTestCase(bool reportTestCases, unsigned nrOfElements, const sw::universal::posit<nbits,es>& seed) {
	using namespace sw::universal;
	int nrOfFailedTestCases = 0;
	std::stringstream ss;
	ss << "quire<" << nbits << "," << es << "," << capacity << ">";
	std::vector< sw::universal::posit<nbits, es> > t = GenerateVectorForZeroValueFDP(nrOfElements, seed);
	nrOfFailedTestCases += ReportTestResult(ValidateQuireAccumulation<nbits, es, capacity>(reportTestCases, t), ss.str(), "accumulation");
	return nrOfFailedTestCases;
}

// initialize a vector
template<typename Vector, typename Scalar>
void init(Vector& x, const Scalar& value) {
	for (unsigned i = 0; i < x.size(); ++i) x[i] = value;
}

// regular dot product
template<typename Vector>
typename Vector::value_type dot(const Vector& a, const Vector& b) {
	typename Vector::value_type sum{ 0 };
	if (size(a) != size(b)) {
		std::cerr << "vectors are not the same size\n";
		return sum;
	}
	for (unsigned i = 0; i < size(a); ++i) {
		sum += a[i] * b[i];
	}
	return sum;
}

template<unsigned nbits, unsigned es, unsigned nrElements = 16>
int ValidateExactDotProduct() {
	using namespace sw::universal;
	int nrOfFailures = 0;
	using Scalar = posit<nbits, es>;
	using Vector = std::vector<Scalar>;
	Scalar maxpos;
	maxpos.maxpos();
	Vector pv = GenerateVectorForZeroValueFDP(nrElements, maxpos);
	Vector ones(nrElements);

	{
		init(ones, Scalar(1));

		Scalar result = fdp(ones, pv);
		std::cout << "exact FDP test yields   = " << float(result) << '\n';

		if (!result.iszero()) ++nrOfFailures;
	}

	{
		using FVector = std::vector<float>;
		FVector fv;
		for_each(begin(pv), end(pv), [&fv](const Scalar& p) {
			fv.push_back(float(p));
		});
		FVector fones;
		for_each(begin(pv), end(pv), [&fones](const Scalar& p) {
			fones.push_back(float(p));
		});
		float result = dot(fones, fv);
		std::cout << "regular DOT test yields = " << result << "\n\n";
	}

	return nrOfFailures;
}

int ValidateQuireMagnitudeComparison() {
	using namespace sw::universal;

	quire<16, 1, 2> q;
	internal::value<20> v;
	v = 0xAAAA;
	q += v;
	v = 0xAAAB;
	std::cout << "quire: " << q << '\n';
	std::cout << "value: " << v.get_fixed_point() << " " << to_triple(v) << '\n';
	std::cout << (q < v ? "correct" : "incorrect") << '\n';
	std::cout << (q > v ? "incorrect" : "correct") << '\n';
	v = 0xAAAA;
	std::cout << "value: " << v.get_fixed_point() << " " << to_triple(v) << '\n';
	std::cout << (q == v ? "correct" : "incorrect") << '\n';
	return 0;
}

template<unsigned nbits, unsigned es, unsigned capacity = 2>
int ValidateSignMagnitudeTransitions() {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;
	std::cout << "Quire configuration: quire<" << nbits << ", " << es << ", " << capacity << ">" << std::endl;

	// moving through the four quadrants of a sign/magnitude adder/subtractor
	posit<nbits, es> min1, min2, min3, min4;
	min1.minpos();                              // ...0001
	min2 = min1; min2++;                        // ...0010
	min3 = min2; min3++;                        // ...0011
	min4 = min3; min4++;                        // ...0100
	posit<nbits, es> max1, max2, max3, max4;
	max1.maxpos();                              // 01..111
	max2 = max1; --max2;                        // 01..110
	max3 = max2; --max3;                        // 01..101
	max4 = max3; --max4;                        // 01..100

	std::cout << '\n';
	std::cout << "Posit range extremes:\n";
	std::cout << "min1 = minpos  " << min1.get() << " " << min1 << '\n';
	std::cout << "min2           " << min2.get() << " " << min2 << '\n';
	std::cout << "min3           " << min3.get() << " " << min3 << '\n';
	std::cout << "min4           " << min4.get() << " " << min4 << '\n';
	std::cout << "..." << '\n';
	std::cout << "max4           " << max4.get() << " " << max4 << '\n';
	std::cout << "max3           " << max3.get() << " " << max3 << '\n';
	std::cout << "max2           " << max2.get() << " " << max2 << '\n';
	std::cout << "max1 = maxpos  " << max1.get() << " " << max1 << '\n';

	std::cout << '\n';

	std::cout << "Quire experiments: sign/magnitude transitions at the range extremes\n";

	posit<nbits, es> one{ 1.0f };
	quire<nbits, es, capacity> q;
	internal::value<2 * (nbits - 2 - es)> addend;

		// show the relative positions of maxpos^2, maxpos, minpos, minpos^2
	q = addend = quire_mul(max1, max1);
	std::cout << q << " q == maxpos^2         = " << to_triple(addend) << '\n';
	q = addend = quire_mul(max1, one);  // indicative that the quire 'sits' behind the ALU.
	std::cout << q << " q == maxpos           = " << to_triple(addend) << '\n';
	q = addend = quire_mul(min1, one);  // indicative that the quire 'sits' behind the ALU.
	std::cout << q << " q == minpos           = " << to_triple(addend) << '\n';
	q = addend = quire_mul(min1, min1);
	std::cout << q << " q == minpos^2         = " << to_triple(addend) << '\n';

	// reset to zero
	q.clear();
	std::cout << q << "                                               <-- start at zero\n";
	// start in the positive, SE quadrant with minpos^2
	q += addend = quire_mul(min1, min1);
	std::cout << q << " q += minpos^2  addend = " << to_triple(addend) << '\n';
	// move to the negative SW quadrant by adding negative value that is bigger
	q += addend = quire_mul(min2, -min2);
	std::cout << q << " q += min2^2    addend = " << to_triple(addend) << '\n';
	// remove minpos^2 from the quire by subtracting it
	q -= addend = quire_mul(min1, min1);
	std::cout << q << " q -= minpos^2  addend = " << to_triple(addend) << '\n';
	// move back into posit, SE quadrant by adding the next bigger product
	q += addend = quire_mul(min3, min3);
	std::cout << q << " q += min3^2    addend = " << to_triple(addend) << '\n';
	// remove the min2^2 from the quire by subtracting it
	q -= addend = quire_mul(min2, min2);
	std::cout << q << " q -= min2^2    addend = " << to_triple(addend) << '\n';
	// add a -maxpos^2, to flip it again
	q += addend = quire_mul(max1, -max1);
	std::cout << q << " q += -maxpos^2 addend = " << to_triple(addend) << '\n';
	// subtract min3^2 to propagate the carry
	q -= addend = quire_mul(min3, min3);
	std::cout << q << " q -= min3^2    addend = " << to_triple(addend) << '\n';
	// remove min2^2 remenants
	q += addend = quire_mul(min2, min2);
	std::cout << q << " q += min2^2    addend = " << to_triple(addend) << '\n';
	q += addend = quire_mul(min2, min2);
	std::cout << q << " q += min2^2    addend = " << to_triple(addend) << '\n';
	// borrow propagate
	q += addend = quire_mul(min1, min1);
	std::cout << q << " q += minpos^2  addend = " << to_triple(addend) << '\n';
	// flip the max3 bit
	q += addend = quire_mul(max3, max3);
	std::cout << q << " q += max3^2    addend = " << to_triple(addend) << '\n';
	// add maxpos^2 to be left with max3^2
	q += addend = quire_mul(max1, max1);
	std::cout << q << " q += maxpos^2  addend = " << to_triple(addend) << '\n';
	// subtract max2^2 to flip the sign again
	q -= addend = quire_mul(max2, max2);
	std::cout << q << " q -= max2^2    addend = " << to_triple(addend) << '\n';
	// remove the max3^2 remenants
	q -= addend = quire_mul(max3, max3);
	std::cout << q << " q -= max3^2    addend = " << to_triple(addend) << '\n';
	// remove the minpos^2 bits
	q -= addend = quire_mul(min1, min1);
	std::cout << q << " q -= minpos^2  addend = " << to_triple(addend) << '\n';
	// add maxpos^2 to be left with max2^2 and flipped back to positive quadrant
	q += addend = quire_mul(max1, max1);
	std::cout << q << " q += maxpos^2  addend = " << to_triple(addend) << '\n';
	// add max2^2 to remove its remenants
	q += addend = quire_mul(max2, max2);
	std::cout << q << " q += max2^2    addend = " << to_triple(addend) << '\n';
	// subtract minpos^2 to propagate the borrow across the quire
	q -= addend = quire_mul(min1, min1);
	std::cout << q << " q -= minpos^2  addend = " << to_triple(addend) << '\n';
	// subtract maxpos^2 to flip the sign and be left with minpos^2
	q -= addend = quire_mul(max1, max1);
	std::cout << q << " q -= maxpos^2  addend = " << to_triple(addend) << '\n';
	// add minpos^2 to get to zero
	q += addend = quire_mul(min1, min1);
	std::cout << q << " q += minpos^2  addend = " << to_triple(addend) << '\n';
	// subtract minpos^2 to go negative
	q += addend = -quire_mul(min1, min1);
	std::cout << q << " q += -minpos^2 addend = " << to_triple(addend) << '\n';
	// add minpos^2 to get to zero
	q += addend = quire_mul(min1, min1);
	std::cout << q << " q += minpos^2  addend = " << to_triple(addend) << " <-- back to zero\n";

	return nrOfFailedTestCases;
}

template<unsigned nbits, unsigned es, unsigned capacity = 2>
int ValidateCarryPropagation() {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	constexpr unsigned mbits = 2 * (nbits - 2 - es);
	quire<nbits, es, capacity> q;
	posit<nbits, es> mp(SpecificValue::minpos);
	internal::value<mbits> minpos_square = quire_mul(mp, mp);
	constexpr unsigned NR_INCREMENTS_TO_OVERFLOW = (unsigned(1) << (q.qbits+1));
	for (unsigned i = 0; i < NR_INCREMENTS_TO_OVERFLOW; ++i) {
		q += minpos_square;
	}
	std::cout << q << '\n';
	nrOfFailedTests = q.iszero() ? 0 : 1;

	return nrOfFailedTests;
}

template<unsigned nbits, unsigned es, unsigned capacity = 2>
int ValidateBorrowPropagation() {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	constexpr unsigned mbits = 2 * (nbits - 2 - es);
	quire<nbits, es, capacity> q;
	posit<nbits, es> mp(SpecificValue::minpos);
	internal::value<mbits> minpos_square = quire_mul(mp, mp);
	q -= minpos_square;
	std::cout << q << '\n';
	constexpr unsigned NR_DECREMENTS_TO_OVERFLOW = (unsigned(1) << (q.qbits + 1));
	for (unsigned i = 0; i < NR_DECREMENTS_TO_OVERFLOW-1; ++i) {
		q -= minpos_square;
	}
	std::cout << q << '\n';
	nrOfFailedTests = q.iszero() ? 0 : 1;

	return nrOfFailedTests;
}

template<unsigned nbits, unsigned es, unsigned capacity = 2>
int ValidateQuireAccumulation(bool reportTestCases) {
	int nrOfFailedTests = 0;

	return nrOfFailedTests;
}

// one of test to check that the quire can deal with 0
void TestCaseForProperZeroHandling() {
	using namespace sw::universal;

	quire<8, 1, 2> q;
	posit<8, 1> minpos;
	minpos.minpos();
	q += quire_mul(minpos, minpos);
	internal::value<3> v3 = q.to_value().round_to<3>();
	internal::value<5> v5 = q.to_value().round_to<5>();
	internal::value<7> v7 = q.to_value().round_to<7>();
	std::cout << to_triple(v3) << '\n';
	std::cout << to_triple(v5) << '\n';
	std::cout << to_triple(v7) << '\n';

	// test correct handling of 0
	q = 1;
	std::cout << q << '\n';
	posit<8, 1> one = 1;
	posit<8, 1> aThird = 0.3333333333333333333333333333333333333333333;
	internal::value< posit<8, 1>::mbits > mul = quire_mul(aThird, -one);
	std::cout << to_triple(mul) << '\n';
	q += quire_mul(aThird, -one);
	std::cout << q << '\n';
	internal::value<8> result = q.to_value().round_to<8>();
	std::cout << result << " " << to_triple(result) << '\n';
}

#define MANUAL_TESTING 1

int main()
try {
	using namespace sw::universal;

	bool reportTestCases = false;
	int nrOfFailedTestCases = 0;

	std::cout << "Quire experiments\n";

	std::string tag = "Quire Accumulation failed";

#if MANUAL_TESTING
	std::cout << "Quire load/store and add/subtract\n";
	posit<16, 1> p(1);
	quire<16, 1> q1(p);
	quire<16, 1> q2 = q1;
	std::cout << q2 << '\n';
	q2 += p;
	std::cout << q2 << '\n';
	q2 -= q1;
	std::cout << q2 << '\n';
	q2 -= p;
	std::cout << q2 << '\n';
	q2 -= p;
	std::cout << q2 << '\n';

	nrOfFailedTestCases += ValidateExactDotProduct<16, 1>();

	nrOfFailedTestCases += ValidateSignMagnitudeTransitions<8, 1>();

	nrOfFailedTestCases += ValidateSignMagnitudeTransitions<16, 1>();
	
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 1, 2>(reportTestCases, 16, posit<8, 1>(SpecificValue::minpos));
	
	std::cout << "Carry Propagation\n";
	nrOfFailedTestCases += ReportTestResult(ValidateCarryPropagation<4, 1>(), "carry propagation", "increment");
	std::cout << "Borrow Propagation\n";
	nrOfFailedTestCases += ReportTestResult(ValidateBorrowPropagation<4, 1>(), "borrow propagation", "increment");

#ifdef ISSUE_45_DEBUG
	{	
		Issue45_2<16, 1, 30>();
	}
#endif

#else

	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 0, 2>(reportTestCases, 16, posit<8, 0>(SpecificValue::minpos));
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 1, 2>(reportTestCases, 16, posit<8, 1>(SpecificValue::minpos));
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 2, 2>(reportTestCases, 16, posit<8, 2>(SpecificValue::minpos));
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 0, 5>(reportTestCases, 16, posit<8, 0>(SpecificValue::maxpos));
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 1, 5>(reportTestCases, 16, posit<8, 1>(SpecificValue::maxpos));
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<8, 2, 5>(reportTestCases, 16, posit<8, 2>(SpecificValue::maxpos));

	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<16, 0, 2>(reportTestCases, 256, posit<16, 0>(SpecificValue::minpos));
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<16, 1, 2>(reportTestCases, 256, posit<16, 1>(SpecificValue::minpos));
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<16, 2, 2>(reportTestCases, 256, posit<16, 2>(SpecificValue::minpos));
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<16, 0, 5>(reportTestCases, 16, posit<16, 0>(SpecificValue::maxpos));
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<16, 1, 5>(reportTestCases, 16, posit<16, 1>(SpecificValue::maxpos));
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<16, 2, 5>(reportTestCases, 16, posit<16, 2>(SpecificValue::maxpos));

	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<24, 0, 2>(reportTestCases, 4096, posit<24, 0>(SpecificValue::minpos));
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<24, 1, 2>(reportTestCases, 4096, posit<24, 1>(SpecificValue::minpos));
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<24, 2, 2>(reportTestCases, 4096, posit<24, 2>(SpecificValue::minpos));

	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<32, 0, 2>(reportTestCases, 65536, posit<32, 0>(SpecificValue::minpos));
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<32, 1, 2>(reportTestCases, 65536, posit<32, 1>(SpecificValue::minpos));
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<32, 2, 2>(reportTestCases, 65536, posit<32, 2>(SpecificValue::minpos));
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<32, 0, 5>(reportTestCases, 16, posit<32, 0>(SpecificValue::maxpos));
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<32, 1, 5>(reportTestCases, 16, posit<32, 1>(SpecificValue::maxpos));
	nrOfFailedTestCases += GenerateQuireAccumulationTestCase<32, 2, 5>(reportTestCases, 16, posit<32, 2>(SpecificValue::maxpos));

#endif // MANUAL_TESTING
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}


////////////////////////////////////////////////////////////////////////
// specific debug scenarios of note
//
// use forward reference to bring them up to the main body
// template<unsigned nbits, unsigned es, unsigned capacity> void Issue45();
// template<unsigned nbits, unsigned es, unsigned capacity> void Issue45_2();

// test case for github issue #45
template<unsigned nbits, unsigned es>
void Issue45() {
	using ScalarType = sw::universal::posit<nbits, es>;
	using magnitude = sw::universal::posit<nbits, es>;
	using positX = sw::universal::posit<nbits, es>;
	using quireX = sw::universal::quire<nbits, es, 10>;
	using valueX = sw::universal::value<2 * (nbits - 2 - es)>;

	constexpr int n = 64;
	std::vector<positX> Acoefficients(n);
	for (int i = 0; i < n; ++i) {
		Acoefficients[i] = positX(sw::universal::SpecificValue::minpos);
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
			valueX addend = sw::universal::quire_mul(Acoefficients[i], xcoefficients[i]);
			sum += addend;
			std::cout << components(addend) << "\n" << sum << std::endl;
		}
		positX tempSum;
		tempSum.convert(sum.to_value());
		ycoefs[row] = tempSum;
		valueX resultValueX = sw::universal::quire_mul(xcoefs[row], tempSum);
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

// step by step testing to find where the failure occurred
template<unsigned nbits, unsigned es, unsigned capacity = 30>
void Issue45_2() {
	using namespace sw::universal;

	std::cout << "Debug of issue #45\n";

	constexpr unsigned mbits = 2 * (nbits - 2 - es);
	sw::universal::quire<nbits, es, capacity> q, q_base;
	sw::universal::value<mbits> unrounded, q_value;
	sw::universal::bitblock<mbits> fraction;

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
		constexpr unsigned mbits = 2 * (nbits - 2 - es);
		sw::universal::quire<nbits, es, capacity> q, q_base;

		// inefficient as we are copying a whole quire just to reset the sign bit, but we are leveraging the comparison logic
		//quire<nbits, es, capacity> absq = abs(*this);
		//constexpr unsigned qbits = (unsigned(1) << es) * (4 * nbits - 8) + capacity;
		//constexpr unsigned fbits = nbits - 3 - es;
		//value<qbits> absq = abs(q);
		quire <nbits, es, capacity> absq = abs(q);
		internal::value<mbits> absv = abs(unrounded);
		if (absq < absv) {
			std::cout << "q < v\n";
		}
		else if (absq > absv) {
			std::cout << "q > v\n";
		}
		else {
			std::cout << "q == v\n";
		}
	}
}
