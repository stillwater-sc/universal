// sum_of_integers.cpp: evaluation of a sequence of additions in the posit number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/number/posit/posit.hpp>

template<typename Scalar>
Scalar NaiveSumOfIntegers(long long lowerbound = 0, long long upperbound = 10000) {
	Scalar sum = 0;
	for (long long i = lowerbound; i < upperbound; ++i) {
		sum += Scalar(i);
	}
	return sum;
}

// to see how different Universal number systems fair on the sum of integers, set STRESS_TESTING to 1
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	// print detailed bit-level computational intermediate results
	// since we are summing integers, the max upperbound is pow(10,9), which is a billion

	// preserve the existing ostream precision
	auto precision = std::cout.precision();
	std::cout << std::setprecision(20);

	std::cout << "SumOfIntegers using 64-bit int\n";
	for (int i = 1; i < 10; ++i) {
		std::cout << std::setw(3) << i << " 0 - " << pow(10,i) << " : " << NaiveSumOfIntegers<long long>(0, (long long)pow(10,i)) << '\n';
	}

	std::cout << "SumOfIntegers using IEEE single precision float\n";
	for (int i = 1; i < 10; ++i) {
		std::cout << std::setw(3) << i << " 0 - " << pow(10, i) << " : " << NaiveSumOfIntegers<float>(0, (long long)pow(10, i)) << '\n';
	}

	std::cout << "SumOfIntegers using IEEE double precision float\n";
	for (int i = 1; i < 10; ++i) {
		std::cout << std::setw(3) << i << " 0 - " << pow(10, i) << " : " << NaiveSumOfIntegers<double>(0, (long long)pow(10, i)) << '\n';
	}

#if STRESS_TESTING
	constexpr size_t es = 2;
	using Posit32 = posit<32, es>;
	using Posit56 = posit<56, es>;
	using Posit64 = posit<64, es>;
	using Integer64 = integer<64, uint64_t>;
	using Integer80 = integer<80, uint32_t>;
	using Integer96 = integer<96, uint32_t>;

	std::cout << "SumOfIntegers using 64-bit Universal integer\n";
	for (int i = 1; i < 10; ++i) {
		std::cout << std::setw(3) << i << " 0 - " << pow(10, i) << " : " << NaiveSumOfIntegers<Integer64>(0, (long)pow(10, i)) << '\n';
	}

	std::cout << "SumOfIntegers using 80-bit Universal integer\n";
	for (int i = 1; i < 10; ++i) {
		std::cout << std::setw(3) << i << " 0 - " << pow(10, i) << " : " << NaiveSumOfIntegers<Integer80>(0, (long)pow(10, i)) << '\n';
	}

	std::cout << "SumOfIntegers using 96-bit Universal integer\n";
	for (int i = 1; i < 10; ++i) {
		std::cout << std::setw(3) << i << " 0 - " << pow(10, i) << " : " << NaiveSumOfIntegers<Integer96>(0, (long)pow(10, i)) << '\n';
	}

	std::cout << "SumOfIntegers using posit<32,2>\n";
	for (int i = 1; i < 6; ++i) {
		std::cout << std::setw(3) << i << " 0 - " << pow(10, i) << " : " << NaiveSumOfIntegers<Posit32>(0, (long)pow(10, i)) << '\n';
	}

	std::cout << "SumOfIntegers using posit<56,2>\n";
	for (int i = 1; i < 6; ++i) {
		std::cout << std::setw(3) << i << " 0 - " << pow(10, i) << " : " << NaiveSumOfIntegers<Posit56>(0, (long)pow(10, i)) << '\n';
	}

	std::cout << "SumOfIntegers using posit<64,2>\n";
	for (int i = 1; i < 6; ++i) {
		std::cout << std::setw(3) << i << " 0 - " << pow(10, i) << " : " << NaiveSumOfIntegers<Posit64>(0, (long)pow(10, i)) << '\n';
	}
#endif

	// restore the previous ostream precision
	std::cout << std::setprecision(precision);

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
