// large_lcm.cpp: calculating a least common multiple of a very large set
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <typeinfo>
#include <chrono>
// include the number system we want to use, and configure overflow exceptions so we can capture failures
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/integer/integer>

template<size_t nbits, typename BlockType>
void MeasureLCM(const std::vector<sw::universal::integer<nbits, BlockType>>& v) {
	using namespace std;
	chrono::steady_clock::time_point begin, end;
	begin = chrono::steady_clock::now();
	using Integer = sw::universal::integer<nbits, BlockType>;
	Integer least_common_multple = lcm(v);
	end = chrono::steady_clock::now();
	using TimeReal = float;
	chrono::duration<TimeReal> time_span = chrono::duration_cast<chrono::duration<TimeReal >> (end - begin);
	TimeReal elapsed = time_span.count();

	cout << "In " << elapsed << " seconds calculated LCM of " << v.size() << " elements of type " << typeid(Integer).name()
		<< " to be\n" << least_common_multple << endl;
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;
	   	
	{
		constexpr size_t nbits = 512;
		using Integer = integer<nbits, uint32_t>;
		Integer factor;

		// use random_device to generate a seed for Mersenne twister engine
		random_device rd{};
		mt19937 engine{ rd() };
		uniform_real_distribution<long double> dist{0.0, 1000000000000.0l };

		vector<Integer> v;
		for (int i = 0; i < 10; ++i) {
			factor = dist(engine);
			if (factor.iseven()) ++factor;
			// cout << factor << endl;
			v.push_back(factor);
		}
		try {
			MeasureLCM(v);
		}
		catch (const integer_overflow& e) {
			cerr << e.what() << endl;
			cerr << typeid(Integer).name() << " has insufficient dynamic range to capture the least common multiple\n";
			ofstream out;
			out.open("lcm_dataset_1.txt");
			for (size_t i = 0; i < v.size(); ++i) {
				out << v[i] << endl;
			}
			out.close();
		}
	}

	{
		constexpr size_t nbits = 1024;
		using Integer = integer<nbits, uint32_t>;
		Integer factor;

		random_device rd{};
		mt19937 engine{ rd() };
		uniform_real_distribution<long double> dist{0.0, 1000000.0 };

		vector<Integer> v;
		for (int i = 0; i < 100; ++i) {
			factor = dist(engine);
			if (factor.iseven()) ++factor;
			v.push_back(factor);
		}
		try {
			MeasureLCM(v);
		}
		catch (const integer_overflow& e) {
			cerr << e.what() << endl;
			cerr << typeid(Integer).name() << " has insufficient dynamic range to capture the least common multiple\n";
			ofstream out;
			out.open("lcm_dataset_2.txt");
			for (size_t i = 0; i < v.size(); ++i) {
				out << v[i] << endl;
			}
			out.close();
		}
	}

	{
		constexpr size_t nbits = 2048;
		using Integer = integer<nbits, uint32_t>;
		Integer factor;

		random_device rd{};
		mt19937 engine{ rd() };
		uniform_real_distribution<long double> dist{0.0, 1000.0 };

		vector<Integer> v;
		for (int i = 0; i < 1000; ++i) {
			factor = dist(engine);
			if (factor.iseven()) ++factor;
			v.push_back(factor);
		}
		try {
			MeasureLCM(v);
		}
		catch (const integer_overflow& e) {
			cerr << e.what() << endl;
			cerr << typeid(Integer).name() << " has insufficient dynamic range to capture the least common multiple\n";
			ofstream out;
			out.open("lcm_dataset_3.txt");
			for (size_t i = 0; i < v.size(); ++i) {
				out << v[i] << endl;
			}
			out.close();
		}
	}

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
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
