// kahan_sum.cpp: Kahan summation evaluation of posit number systems
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include "common.hpp"
// pull in the posit number system
#include <universal/number/posit/posit.hpp>

constexpr size_t COLUMN_WIDTH = 25;

/*
floating point arithmetic:
 - integers are represented exactly
 - float(x - y) = x - y when x/2 <= y <= 2x: 
        difference is represented exactly when two numbers are less than 2x of each other
 - float(2x)    = 2x barring overflow
 - float(x/2)   = x/2 barring underflow

TwoSum denotes an algorithm introduced by Knuth 
in "The Art of Computer Programming", vol 2, Seminumerical Algorithms.

Given two floating point values a and b, 
generate a rounded sum s and a remainder r, such that
    s = RoundToNearest(a + b), and
    a + b = s + r

*/


template<typename Vector>
std::pair<typename Vector::value_type, typename Vector::value_type> KahanSummation(const Vector& data) {
	using Scalar = typename Vector::value_type;
	Scalar a{ 0 };
	Scalar r{ 0 };
	for (auto b : data) {
		Scalar y = b - r;
//		std::cout << "y : " << y << std::endl;
		Scalar t = a + y;
//		std::cout << "t : " << t << std::endl;
		r = (t - a) - y;  // (sum + y - sum) - y
//		std::cout << "r : " << r << std::endl;
		a = t;
//		std::cout << "a : " << a << std::endl;
	}
	return std::make_pair(a, r);
}

template<typename Vector>
typename Vector::value_type GenerateData(Vector& data, size_t nrElements) {
	using Scalar = typename Vector::value_type;
	Scalar v = Scalar(1.0) / Scalar(nrElements);
	data.clear();
	Scalar naiveSum{ 0 };
	for (size_t i = 0; i < nrElements; ++i) {
		data.push_back(v);
		naiveSum += v;
	}
	return naiveSum;
}

template<typename Scalar>
void GenerateTest(std::ostream& ostr, size_t N) {
	using namespace std;
	std::vector<Scalar> data;
	auto naiveSum = GenerateData(data, N);
	auto p = KahanSummation(data);
	auto oldprecision = ostr.precision();
	auto target = std::numeric_limits<Scalar>::max_digits10;
	ostr << setprecision(target)
		<< setw(COLUMN_WIDTH+15)
		<< typeid(Scalar).name()
		<< ", "
		<< setw(COLUMN_WIDTH)
		<< naiveSum
		<< ", "
		<< setw(COLUMN_WIDTH)
		<< p.first
		<< ", "
		<< setw(COLUMN_WIDTH)
		<< p.second 
		<< setw(oldprecision)
		<< endl;
}

#define MANUAL_TEST 1

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	// print detailed bit-level computational intermediate results
	// bool verbose = false;

// 	int nrOfFailedTestCases = 0;

	// preserve the existing ostream precision
	auto precision = std::cout.precision();
	std::cout << setprecision(12);

	std::cout << "Kahan summation comparison" << endl;

#if MANUAL_TEST

/*
Kahan summation comparison
									type,                 Naive Sum,                 Kahan Sum,            Residual Error
								   float,               0.999999344,                         1,            2.23517418e-08
								  double,        1.0000000000000007,                         1,   -2.0816681711721685e-17
			 class sw::universal::posit<32,2>,                1.00000007,                         1,           -9.31322575e-10
			 class sw::universal::posit<64,3>,                         1,                         1,   8.67361737988403547e-19
>>>> a floating point value that is perfectly representable
								   float,                         1,                         1,                         0
								  double,                         1,                         1,                         0
			 class sw::universal::posit<32,2>,                         1,                         1,                         0
			 class sw::universal::posit<64,3>,                         1,                         1,                         0
*/

	cout << setw(COLUMN_WIDTH+15) << "type" << ", " 
		<< setw(COLUMN_WIDTH) << "Naive Sum" << ", " 
		<< setw(COLUMN_WIDTH) << "Kahan Sum" << ", " 
		<< setw(COLUMN_WIDTH) << "Residual Error" << std::endl;

	{
		constexpr size_t N = 100;
		GenerateTest<float>(cout, N);
		GenerateTest<double>(cout, N);
		GenerateTest<sw::universal::posit<32, 2>>(cout, N);
		GenerateTest<sw::universal::posit<64, 3>>(cout, N);
	}

	cout << ">>>> a floating point value that is perfectly representable\n";
	{
		constexpr size_t N = 65536;
		GenerateTest<float>(cout, N);
		GenerateTest<double>(cout, N);
		GenerateTest<sw::universal::posit<32, 2>>(cout, N);
		GenerateTest<sw::universal::posit<64, 3>>(cout, N);
	}

#else
	// nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<2, 0>(tag, bReportIndividualTestCases), "posit<2,0>", "twoSum");

#endif // MANUAL_TEST

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
