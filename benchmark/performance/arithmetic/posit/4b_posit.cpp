// posit_8b.cpp: performance characterization of standard posit<8,0> configuration
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable fast specialized posit<4,0>
//#define POSIT_FAST_SPECIALIZATION
#define POSIT_FAST_POSIT_4_0 1
// enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <universal/performance/number_system.hpp>

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	constexpr size_t nbits = 4;
	constexpr size_t es = 0;

#if defined(POSIT_FAST_POSIT_4_0)
	std::cout << "Fast specialization posit<4,0> configuration performance tests\n";
#else
	std::cout << "Reference posit<4,0> configuration performance tests\n";
#endif
	posit<nbits, es> number;
	OperatorPerformance perfReport;
	GeneratePerformanceReport(number, perfReport);
	std::cout << ReportPerformance(number, perfReport);
	std::cout << std::endl;
	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
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
