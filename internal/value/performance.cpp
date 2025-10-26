// performance.cpp: functional tests of the value type API
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// as we are using the raw include, we need to 
// setup the conditional compilation flags
#define VALUE_THROW_ARITHMETIC_EXCEPTION 0
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/internal/value/value.hpp>
#include <universal/performance/number_system.hpp>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::internal;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	// Performance benchmarks for value class
	std::cout << "\nPerformance benchmarks for value<> class\n";
	std::cout << (bReportIndividualTestCases ? " " : "not ") << "reporting individual testcases\n";

#if MANUAL_TESTING

	std::cout << "single precision float\n";
	{
		constexpr unsigned fbits = 22;
		value<fbits> number{ 1 };
		OperatorPerformance perfReport;
		GeneratePerformanceReport(number, perfReport);
		auto report = ReportPerformance(number, perfReport);
		std::cout << report << '\n';
	}

#else

#if REGRESSION_LEVEL_1
	std::cout << "half precision float\n";
	{
		constexpr unsigned fbits = 10;
		value<fbits> number{ 1 };
		OperatorPerformance perfReport;
		GeneratePerformanceReport(number, perfReport);
		auto report = ReportPerformance(number, perfReport);
		std::cout << report << '\n';
	}
#endif

#if REGRESSION_LEVEL_2
	std::cout << "single precision float\n";
	{
		constexpr unsigned fbits = 22;
		value<fbits> number{ 1 };
		OperatorPerformance perfReport;
		GeneratePerformanceReport(number, perfReport);
		auto report = ReportPerformance(number, perfReport);
		std::cout << report << '\n';
	}
#endif

#if REGRESSION_LEVEL_3
	std::cout << "double precision float\n";
	{
		constexpr unsigned fbits = 53;
		value<fbits> number{ 1 };
		OperatorPerformance perfReport;
		GeneratePerformanceReport(number, perfReport);
		auto report = ReportPerformance(number, perfReport);
		std::cout << report << '\n';
	}

	std::cout << "extended precision float\n";
	{
		constexpr unsigned fbits = 64;
		value<fbits> number{ 1 };
		OperatorPerformance perfReport;
		GeneratePerformanceReport(number, perfReport);
		auto report = ReportPerformance(number, perfReport);
		std::cout << report << '\n';
	}
#endif

#if REGRESSION_LEVEL_4
	std::cout << "quad precision float\n";
	{
		constexpr unsigned fbits = 112;
		value<fbits> number{ 1 };
		OperatorPerformance perfReport;
		GeneratePerformanceReport(number, perfReport);
		auto report = ReportPerformance(number, perfReport);
		std::cout << report << '\n';
	}
#endif

#endif // MANUAL_TESTING

	std::cout << (nrOfFailedTestCases == 0 ? "PASS" : "FAIL");
	std::cout.flush();

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

/*
Benchmarked 12/13/2020
Processor : Intel Core i7-9850H CPU @2.60GHz
Memory    : 16GB
OS        : 64-bit, x64-based processor
Performance benchmarks for value<> class
 reporting individual testcases
half precision float
Performance Report for type: class sw::universal::value<10>
Conversion int  : 104 MPOPS
Conversion ieee :  31 MPOPS
Prefix          :   9 MPOPS
Postfix         :   9 MPOPS
Negation        : 196 MPOPS
Addition        :  10 MPOPS
Subtraction     :   8 MPOPS
Multiplication  :   8 MPOPS
Division        :   1 MPOPS
Square Root     :  10 MPOPS


single precision float
Performance Report for type: class sw::universal::value<22>
Conversion int  :  18 MPOPS
Conversion ieee :  14 MPOPS
Prefix          :   4 MPOPS
Postfix         :   4 MPOPS
Negation        : 212 MPOPS
Addition        :   3 MPOPS
Subtraction     :   2 MPOPS
Multiplication  :   2 MPOPS
Division        : 313 KPOPS
Square Root     :   4 MPOPS


double precision float
Performance Report for type: class sw::universal::value<53>
Conversion int  :   9 MPOPS
Conversion ieee :  10 MPOPS
Prefix          :   2 MPOPS
Postfix         :   2 MPOPS
Negation        : 219 MPOPS
Addition        :   1 MPOPS
Subtraction     :   1 MPOPS
Multiplication  :   1 MPOPS
Division        :  79 KPOPS
Square Root     :   2 MPOPS


extended precision float
Performance Report for type: class sw::universal::value<64>
Conversion int  :   9 MPOPS
Conversion ieee :  10 MPOPS
Prefix          :   1 MPOPS
Postfix         :   1 MPOPS
Negation        : 228 MPOPS
Addition        :   1 MPOPS
Subtraction     :   1 MPOPS
Multiplication  :   1 MPOPS
Division        :  58 KPOPS
Square Root     :   2 MPOPS


quad precision float
Performance Report for type: class sw::universal::value<112>
Conversion int  :   8 MPOPS
Conversion ieee :  10 MPOPS
Prefix          :   1 MPOPS
Postfix         :   1 MPOPS
Negation        : 284 MPOPS
Addition        :   1 MPOPS
Subtraction     : 918 KPOPS
Multiplication  : 690 KPOPS
Division        :  22 KPOPS
Square Root     :   2 MPOPS
 */

/*
Benchmarked  9/16/2021 
Processor : AMD Ryzen
Memory    : 32GB
OS        : 64-bit, x64-based processor

Performance benchmarks for value<> class
 reporting individual testcases
half precision float
Performance Report for type: class sw::universal::internal::value<10>
Conversion int  :  70 MPOPS
Conversion ieee :  25 MPOPS
Prefix          :   8 MPOPS
Postfix         :   7 MPOPS
Negation        : 168 MPOPS
Addition        :   9 MPOPS
Subtraction     :   6 MPOPS
Multiplication  :   6 MPOPS
Division        : 980 KPOPS
Square Root     :   7 MPOPS


single precision float
Performance Report for type: class sw::universal::internal::value<22>
Conversion int  :  14 MPOPS
Conversion ieee :   9 MPOPS
Prefix          :   3 MPOPS
Postfix         :   3 MPOPS
Negation        : 222 MPOPS
Addition        :   2 MPOPS
Subtraction     :   2 MPOPS
Multiplication  :   1 MPOPS
Division        : 258 KPOPS
Square Root     :   3 MPOPS


double precision float
Performance Report for type: class sw::universal::internal::value<53>
Conversion int  :   9 MPOPS
Conversion ieee :   6 MPOPS
Prefix          :   1 MPOPS
Postfix         :   1 MPOPS
Negation        : 209 MPOPS
Addition        :   1 MPOPS
Subtraction     :   1 MPOPS
Multiplication  :   1 MPOPS
Division        :  59 KPOPS
Square Root     :   2 MPOPS


extended precision float
Performance Report for type: class sw::universal::internal::value<64>
Conversion int  :   5 MPOPS
Conversion ieee :   7 MPOPS
Prefix          :   1 MPOPS
Postfix         :   1 MPOPS
Negation        : 219 MPOPS
Addition        :   1 MPOPS
Subtraction     :   1 MPOPS
Multiplication  : 878 KPOPS
Division        :  54 KPOPS
Square Root     :   2 MPOPS


quad precision float
Performance Report for type: class sw::universal::internal::value<112>
Conversion int  :   6 MPOPS
Conversion ieee :   7 MPOPS
Prefix          : 859 KPOPS
Postfix         : 876 KPOPS
Negation        : 265 MPOPS
Addition        : 821 KPOPS
Subtraction     : 679 KPOPS
Multiplication  : 564 KPOPS
Division        :  21 KPOPS
Square Root     :   2 MPOPS


PASS
 */