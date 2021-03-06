// algo.cpp: tests to explore different implementations of the arithmetic operators
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>

// minimum set of include files to reflect source code dependencies
#include <universal/native/ieee754.hpp>
#define VALUE_THROW_ARITHMETIC_EXCEPTION 0
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/internal/value/value.hpp>
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
// #include <universal/verification/test_reporters.hpp>


// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	if (argc > 1) std::cout << argv[0] << std::endl; 
	
//	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug

	{
		internal::value<8> a,b;
		a = 1.0f;
		b = 1.0f;
		cout << to_triple(a) << " : " << a << '\n';
		cout << to_triple(b) << " : " << b << '\n';
		// add is adding 3 bits to the mantissa to 
		// have all rounding bits available after alignment
		internal::value<13> c;
		internal::module_add<8,12>(a, b, c);
		cout << to_triple(c) << " : " << c << '\n';
	}
	{
		blocktriple<8> a,b;
		a = 1.0f;
		b = 1.0f;
		cout << to_triple(a) << " : " << a << '\n';
		cout << to_triple(b) << " : " << b << '\n';
		// with blocktriple we hide the internals
		// and present an unrounded external interface
		// But how could you do that if the guard/round/sticky
		// bits are required for proper rounding?
		blocktriple<9> c;
		module_add(a, b, c);
		cout << to_triple(c) << " : " << c << '\n';
	}




#if STRESS_TESTING

#endif

#else

	cout << "block addition validation" << endl;


#if STRESS_TESTING



#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
