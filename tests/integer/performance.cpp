//  performance.cpp : performance benchmarking for abitrary precision integers
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
// configure the integer arithmetic class
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/integer/integer.hpp>
#include <universal/integer/numeric_limits.hpp>
// is representable
#include <universal/functions/isrepresentable.hpp>
// test helpers, such as, ReportTestResults
#include "../utils/test_helpers.hpp"

/*
   The goal of the arbitrary integers is to provide a constrained big integer type
   that enables fast computation with exceptions for overflow, so that the type
   can be used for forward error analysis studies.
*/

namespace sw {
namespace unum {


}
}

#include <chrono>
template<size_t nbits>
void ShiftPerformanceTest() {
	using namespace std;
	using namespace std::chrono;

	constexpr uint64_t NR_OPS = 1000000;

	integer<nbits> a = 0xFFFFFFFF;
	steady_clock::time_point begin = steady_clock::now();
	for (uint64_t i = 0; i < NR_OPS; ++i) {
		a >>= 8;
		a <<= 8;
	}
	steady_clock::time_point end = steady_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(end - begin);;
	double elapsed = time_span.count();
	cout << "performance is " << double(NR_OPS) / elapsed << " integer<" << nbits << "> shifts/sec" << endl;
}

// do we need to fix the performance of the shift operator?
void TestShiftOperatorPerformance() {
	using namespace std;

	cout << endl << "TestShiftOperatorPerformance" << endl;

	ShiftPerformanceTest<16>();
	ShiftPerformanceTest<32>();
	ShiftPerformanceTest<64>();
	ShiftPerformanceTest<128>();
	ShiftPerformanceTest<1024>();
	/*
	performance of the serial implementation of the shift operators
		performance is 1.99374e+07 integer<16> shifts / sec
		performance is 8.44852e+06 integer<32> shifts / sec
		performance is 3.85375e+06 integer<64> shifts / sec
		performance is 1.77301e+06 integer<128> shifts / sec
		performance is 219793 integer<1024> shifts / sec
	*/
}

template<size_t nbits>
void ArithmeticPerformanceTest() {
	using namespace std;
	using namespace std::chrono;

	constexpr uint64_t NR_OPS = 1000000;

	steady_clock::time_point begin, end;
	duration<double> time_span;
	double elapsed;

	integer<nbits> a, b, c, d;
	for (int i = 0; i < int(a.nrBytes); ++i) {
		a.setbyte(i, rand());
		b.setbyte(i, rand());
	}
	begin = steady_clock::now();
	for (uint64_t i = 0; i < NR_OPS; ++i) {
		c = a + b;
		a = c - b;
	}
	end = steady_clock::now();
	time_span = duration_cast<duration<double>>(end - begin);;
	 elapsed = time_span.count();
	cout << "performance is " << double(NR_OPS) / elapsed << " integer<" << nbits << "> additions/subtractions" << endl;

	begin = steady_clock::now();
	for (uint64_t i = 0; i < NR_OPS; ++i) {
		c = a * b;
		c.clear();
		d = c;
	}
	end = steady_clock::now();
	time_span = duration_cast<duration<double>>(end - begin);;
	elapsed = time_span.count();
	cout << "performance is " << double(NR_OPS) / elapsed << " integer<" << nbits << "> multiplications" << endl;

	begin = steady_clock::now();
	for (uint64_t i = 0; i < NR_OPS; ++i) {
		c = a / b;
		c.clear();
		d = c;
	}
	end = steady_clock::now();
	time_span = duration_cast<duration<double>>(end - begin);;
	elapsed = time_span.count();
	cout << "performance is " << double(NR_OPS) / elapsed << " integer<" << nbits << "> divisions" << endl;
}

void TestArithmeticOperatorPerformance() {
	using namespace std;

	cout << endl << "TestShiftOperatorPerformance" << endl;

	ArithmeticPerformanceTest<16>();
	ArithmeticPerformanceTest<32>();
	ArithmeticPerformanceTest<64>();
	ArithmeticPerformanceTest<128>();
//	ArithmeticPerformanceTest<1024>();
	/*
		TestShiftOperatorPerformance
		performance is 1.01249e+08 integer<16> additions/subtractions
		performance is 1.45226e+06 integer<16> multiplications
		performance is 3.05808e+07 integer<16> divisions
		performance is 6.75147e+07 integer<32> additions/subtractions
		performance is 366806 integer<32> multiplications
		performance is 1.93706e+06 integer<32> divisions
		performance is 2.11016e+07 integer<64> additions/subtractions
		performance is 93139 integer<64> multiplications
		performance is 4.24692e+07 integer<64> divisions
		performance is 1.29312e+07 integer<128> additions/subtractions
		performance is 23545.5 integer<128> multiplications
		performance is 543714 integer<128> divisions
		performance is 2.06385e+06 integer<1024> additions/subtractions
		performance is 407.244 integer<1024> multiplications
		performance is 2.58264e+06 integer<1024> divisions
	*/
}

// enumerate a couple ratios to test representability
void ReproducibilityTestSuite() {
	for (int i = 0; i < 30; i += 3) {
		for (int j = 0; j < 70; j += 7) {
			sw::unum::reportRepresentability(i, j);
		}
	}
}


#define MANUAL_TESTING 0
#define STRESS_TESTING 0

std::string convert_to_string(const std::vector<char>& v) {
	std::stringstream ss;
	for (std::vector<char>::const_reverse_iterator rit = v.rbegin(); rit != v.rend(); ++rit) {
		ss << (int)*rit;
	}
	return ss.str();
}

int main()
try {
	using namespace std;
	using namespace sw::unum;

	std::string tag = "Integer operator performance benchmarking";

#if MANUAL_TESTING

	TestShiftOperatorPerformance();
	TestArithmeticOperatorPerformance();
	ReproducibilityTestSuite();

	cout << "done" << endl;

	return EXIT_SUCCESS;
#else
	std::cout << tag << std::endl;

	int nrOfFailedTestCases = 0;
	   
	TestArithmeticOperatorPerformance();

#if STRESS_TESTING

#endif // STRESS_TESTING
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
