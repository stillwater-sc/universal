// performance.cpp: functional tests of the value type API
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/bitblock/bitblock.hpp>
#include <universal/value/value.hpp>
#include <universal/performance/number_system.hpp>

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	// Performance benchmarks for value class
	cout << endl << "Performance benchmarks for value<> class" << endl;
	cout << (bReportIndividualTestCases ? " " : "not ") << "reporting individual testcases" << endl;

#if MANUAL_TESTING
	cout << "half precision float\n";
	{
		constexpr size_t fbits = 10;
		value<fbits> number{ 1 };
		OperatorPerformance perfReport;
		GeneratePerformanceReport(number, perfReport);
		auto report = ReportPerformance(number, perfReport);
		cout << report << endl;
	}

	cout << "single precision float\n";
	{
		constexpr size_t fbits = 22;
		value<fbits> number{ 1 };
		OperatorPerformance perfReport;
		GeneratePerformanceReport(number, perfReport);
		auto report = ReportPerformance(number, perfReport);
		cout << report << endl;
	}

	cout << "double precision float\n";
	{
		constexpr size_t fbits = 53;
		value<fbits> number{ 1 };
		OperatorPerformance perfReport;
		GeneratePerformanceReport(number, perfReport);
		auto report = ReportPerformance(number, perfReport);
		cout << report << endl;
	}

	cout << "extended precision float\n";
	{
		constexpr size_t fbits = 64;
		value<fbits> number{ 1 };
		OperatorPerformance perfReport;
		GeneratePerformanceReport(number, perfReport);
		auto report = ReportPerformance(number, perfReport);
		cout << report << endl;
	}

	cout << "quad precision float\n";
	{
		constexpr size_t fbits = 112;
		value<fbits> number{ 1 };
		OperatorPerformance perfReport;
		GeneratePerformanceReport(number, perfReport);
		auto report = ReportPerformance(number, perfReport);
		cout << report << endl;
	}

#else

	cout << "TBD" << endl;

#endif // MANUAL_TESTING

	if (nrOfFailedTestCases > 0) cout << "FAIL"; else cout << "PASS";

	cout.flush();
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