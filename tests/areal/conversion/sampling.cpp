// sampling.cpp: sampling comparison between different areal configurations
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if defined(_MSC_VER)
#pragma warning(disable : 4514)  // unreferenced function is removed
#pragma warning(disable : 4710)  // function is not inlined
#pragma warning(disable : 5045)  // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#endif
#include <iostream>
#include <iomanip>
// Configure the areal template environment
// first: enable general or specialized configurations
#define AREAL_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define AREAL_THROW_ARITHMETIC_EXCEPTION 0

#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>


template<size_t nbits, size_t es>
void GenerateArealComparisonTable(const std::string& tag) {
	using namespace sw::universal;
	constexpr size_t NR_VALUES = (size_t(1) << nbits);

	areal<nbits, es> a;
	std::string typeOfa = typeid(a).name();
	areal<nbits+1, es> b;
	std::string typeOfb = typeid(b).name();
	size_t columnWidth = 6 + std::max(typeOfa.length(), typeOfb.length());
	std::cout << tag << '\n' << std::setw(columnWidth) << typeOfb << "  |  " << std::setw(columnWidth) << typeOfa << '\n';

	// enumerate and compare the sampling of the real value line of the two types
	for (size_t i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		b.setbits(2*i);
		std::cout << std::setw(columnWidth - 11ull) << pretty_print(b) << ' ' << std::setw(10) << b << "  |  " << pretty_print(a) << ' ' << std::setw(10) << a << '\n';

		b.setbits(2 * i + 1);
		std::cout << std::setw(columnWidth - 11ull) << pretty_print(b) << ' ' << std::setw(10) << b << "  |  " << '\n';
	}
}

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	//bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "sampling of the real line: ";

#if MANUAL_TESTING

	GenerateArealComparisonTable<5, 2>(tag);

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else  // !MANUAL_TESTING

	std::cout << "Sampling of the reals by different areal configurations\n";



#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::areal_arithmetic_exception& err) {
	std::cerr << "Uncaught areal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::areal_internal_exception& err) {
	std::cerr << "Uncaught areal internal exception: " << err.what() << std::endl;
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


/*
  Value relationships between areal<nbits+1,es+1> and areal<nbits,es> we'll use for validation

  To generate:
  	GenerateFixedPointComparisonTable<4, 0>(std::string("-"));
	GenerateFixedPointComparisonTable<4, 1>(std::string("-"));
	GenerateFixedPointComparisonTable<4, 2>(std::string("-"));
	

 */
