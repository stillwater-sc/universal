// mod_conversion.cpp: functional tests for fixed-point modulo conversions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 0

// minimum set of include files to reflect source code dependencies
#include <universal/fixpnt/fixed_point.hpp>
#include <universal/fixpnt/manipulators.hpp>
#include <universal/fixpnt/math_functions.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/fixpnt_test_suite.hpp>

// generate specific test case that you can trace with the trace conditions in fixed_point.hpp
// for most bugs they are traceable with _trace_conversion and _trace_add
template<size_t nbits, size_t rbits, typename Ty>
void GenerateTestCase(Ty _a, Ty _b) {
	Ty ref;
	sw::universal::fixpnt<nbits, rbits> a, b, cref, result;
	a = _a;
	b = _b;
	result = a + b;
	ref = _a + _b;
	cref = ref;
	std::streamsize oldPrecision = std::cout.precision();
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << _a << " + " << std::setw(nbits) << _b << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << a << " + " << b << " = " << result << " (reference: " << cref << ")   " ;
	std::cout << (cref == result ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::dec << std::setprecision(oldPrecision);
}


template<size_t nbits, size_t rbits>
void GenerateFixedPointComparisonTable(std::string& tag) {
	using namespace std;
	using namespace sw::universal;
	constexpr size_t NR_VALUES = (size_t(1) << nbits);
	fixpnt<nbits, rbits> fp;
	fixpnt<nbits+1, rbits+1> fpnext;
	cout << "  fixpnt<" << nbits + 1 << "," << rbits + 1 << ">      |    fixpnt<" << nbits << ", " << rbits << ">" << endl;
	for (size_t i = 0; i < NR_VALUES; ++i) {
		fp.set_raw_bits(i);
		fpnext.set_raw_bits(2*i);
		cout << to_binary(fpnext) << ' ' << setw(10) << fpnext << "  |  " << to_binary(fp) << ' ' << setw(15) << fp << endl;
		fpnext.set_raw_bits(2 * i + 1);
		cout << to_binary(fpnext) << ' ' << setw(10) << fpnext << "  |  " << endl;
	}
}

/*
void GenerateFixedPointRangeTable() {
	using namespace std;
	using namespace sw::universal;
	cout << "fixpnt<4,#> ranges\n";
	ReportFixedPointRanges<4, 0, Modulo>(cout);
	ReportFixedPointRanges<4, 1, Modulo>(cout);
	ReportFixedPointRanges<4, 2, Modulo>(cout);
	ReportFixedPointRanges<4, 3, Modulo>(cout);
	ReportFixedPointRanges<4, 4, Modulo>(cout);
	cout << "fixpnt<6,#> ranges\n";
	ReportFixedPointRanges<6, 0, Modulo>(cout);
	ReportFixedPointRanges<6, 1, Modulo>(cout);
	ReportFixedPointRanges<6, 2, Modulo>(cout);
	ReportFixedPointRanges<6, 3, Modulo>(cout);
	ReportFixedPointRanges<6, 4, Modulo>(cout);
	ReportFixedPointRanges<6, 5, Modulo>(cout);
	ReportFixedPointRanges<6, 6, Modulo>(cout);
	cout << "fixpnt<8,#> ranges\n";
	ReportFixedPointRanges<8, 0, Modulo>(cout);
	ReportFixedPointRanges<8, 1, Modulo>(cout);
	ReportFixedPointRanges<8, 2, Modulo>(cout);
	ReportFixedPointRanges<8, 3, Modulo>(cout);
	ReportFixedPointRanges<8, 4, Modulo>(cout);
	ReportFixedPointRanges<8, 5, Modulo>(cout);
	ReportFixedPointRanges<8, 6, Modulo>(cout);
	ReportFixedPointRanges<8, 7, Modulo>(cout);
	ReportFixedPointRanges<8, 8, Modulo>(cout);
	cout << "fixpnt<10,#> ranges\n";
	ReportFixedPointRanges<10, 0, Modulo>(cout);
	ReportFixedPointRanges<10, 1, Modulo>(cout);
	ReportFixedPointRanges<10, 2, Modulo>(cout);
	ReportFixedPointRanges<10, 3, Modulo>(cout);
	ReportFixedPointRanges<10, 4, Modulo>(cout);
	ReportFixedPointRanges<10, 5, Modulo>(cout);
	ReportFixedPointRanges<10, 6, Modulo>(cout);
	ReportFixedPointRanges<10, 7, Modulo>(cout);
	ReportFixedPointRanges<10, 8, Modulo>(cout);
	ReportFixedPointRanges<10, 9, Modulo>(cout);
	ReportFixedPointRanges<10, 10, Modulo>(cout);
	cout << "fixpnt<12,#> ranges\n";
	ReportFixedPointRanges<12, 0, Modulo>(cout);
	ReportFixedPointRanges<12, 1, Modulo>(cout);
	ReportFixedPointRanges<12, 2, Modulo>(cout);
	ReportFixedPointRanges<12, 3, Modulo>(cout);
	ReportFixedPointRanges<12, 4, Modulo>(cout);
	ReportFixedPointRanges<12, 5, Modulo>(cout);
	ReportFixedPointRanges<12, 6, Modulo>(cout);
	ReportFixedPointRanges<12, 7, Modulo>(cout);
	ReportFixedPointRanges<12, 8, Modulo>(cout);
	ReportFixedPointRanges<12, 9, Modulo>(cout);
	ReportFixedPointRanges<12, 10, Modulo>(cout);
	ReportFixedPointRanges<12, 11, Modulo>(cout);
	ReportFixedPointRanges<12, 12, Modulo>(cout);
	cout << "fixpnt<14,#> ranges\n";
	ReportFixedPointRanges<14, 0, Modulo>(cout);
	ReportFixedPointRanges<14, 1, Modulo>(cout);
	ReportFixedPointRanges<14, 2, Modulo>(cout);
	ReportFixedPointRanges<14, 3, Modulo>(cout);
	ReportFixedPointRanges<14, 4, Modulo>(cout);
	ReportFixedPointRanges<14, 5, Modulo>(cout);
	ReportFixedPointRanges<14, 6, Modulo>(cout);
	ReportFixedPointRanges<14, 7, Modulo>(cout);
	ReportFixedPointRanges<14, 8, Modulo>(cout);
	ReportFixedPointRanges<14, 9, Modulo>(cout);
	ReportFixedPointRanges<14, 10, Modulo>(cout);
	ReportFixedPointRanges<14, 11, Modulo>(cout);
	ReportFixedPointRanges<14, 12, Modulo>(cout);
	ReportFixedPointRanges<14, 13, Modulo>(cout);
	ReportFixedPointRanges<14, 14, Modulo>(cout);
	cout << "fixpnt<16,#> ranges\n";
	ReportFixedPointRanges<16, 0, Modulo>(cout);
	ReportFixedPointRanges<16, 1, Modulo>(cout);
	ReportFixedPointRanges<16, 2, Modulo>(cout);
	ReportFixedPointRanges<16, 3, Modulo>(cout);
	ReportFixedPointRanges<16, 4, Modulo>(cout);
	ReportFixedPointRanges<16, 5, Modulo>(cout);
	ReportFixedPointRanges<16, 6, Modulo>(cout);
	ReportFixedPointRanges<16, 7, Modulo>(cout);
	ReportFixedPointRanges<16, 8, Modulo>(cout);
	ReportFixedPointRanges<16, 9, Modulo>(cout);
	ReportFixedPointRanges<16, 10, Modulo>(cout);
	ReportFixedPointRanges<16, 11, Modulo>(cout);
	ReportFixedPointRanges<16, 12, Modulo>(cout);
	ReportFixedPointRanges<16, 13, Modulo>(cout);
	ReportFixedPointRanges<16, 14, Modulo>(cout);
	ReportFixedPointRanges<16, 15, Modulo>(cout);
	ReportFixedPointRanges<16, 16, Modulo>(cout);
	cout << "fixpnt<20,#> ranges\n";
	ReportFixedPointRanges<20, 0, Modulo>(cout);
	ReportFixedPointRanges<20, 1, Modulo>(cout);
	ReportFixedPointRanges<20, 2, Modulo>(cout);
	ReportFixedPointRanges<20, 3, Modulo>(cout);
	ReportFixedPointRanges<20, 4, Modulo>(cout);
	ReportFixedPointRanges<20, 5, Modulo>(cout);
	ReportFixedPointRanges<20, 6, Modulo>(cout);
	ReportFixedPointRanges<20, 7, Modulo>(cout);
	ReportFixedPointRanges<20, 8, Modulo>(cout);
	ReportFixedPointRanges<20, 9, Modulo>(cout);
	ReportFixedPointRanges<20, 10, Modulo>(cout);
	ReportFixedPointRanges<20, 11, Modulo>(cout);
	ReportFixedPointRanges<20, 12, Modulo>(cout);
	ReportFixedPointRanges<20, 13, Modulo>(cout);
	ReportFixedPointRanges<20, 14, Modulo>(cout);
	ReportFixedPointRanges<20, 15, Modulo>(cout);
	ReportFixedPointRanges<20, 16, Modulo>(cout);
	ReportFixedPointRanges<20, 17, Modulo>(cout);
	ReportFixedPointRanges<20, 18, Modulo>(cout);
	ReportFixedPointRanges<20, 19, Modulo>(cout);
	ReportFixedPointRanges<20, 20, Modulo>(cout);
}
*/
// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "conversion: ";

#if MANUAL_TESTING

	ReportFixedPointRanges<12, 0, Modulo>(cout);

	//ReportFixedPointRanges<12, 1, Modulo>(cout);
	//GenerateFixedPointValues<12, 1>();
	
	//GenerateFixedPointRangeTable();

	//cout << "quire<512,240>\n";
	//ReportFixedPointRanges<512, 240, Modulo>(cout);

	GenerateFixedPointValues<4, 2>();

	/*
	GenerateFixedPointValues<4, 0>();
	GenerateFixedPointValues<4, 1>();
	GenerateFixedPointValues<4, 2>();
	GenerateFixedPointValues<4, 3>();
	GenerateFixedPointValues<4, 4>();

	GenerateFixedPointValues<5, 4>();
	GenerateFixedPointValues<5, 5>();

	GenerateFixedPointValues<8, 8>();

	GenerateFixedPointValues<16, 8>();
    */

	nrOfFailedTestCases = ReportTestResult(VerifyConversion<4, 4>(tag, bReportIndividualTestCases), tag, "fixpnt<4,4>");

	nrOfFailedTestCases = ReportTestResult(VerifyConversion<8, 8>(tag, bReportIndividualTestCases), tag, "fixpnt<8,8>");

	nrOfFailedTestCases = ReportTestResult(VerifyConversion<12, 1>(tag, bReportIndividualTestCases), tag, "fixpnt<12,1>");

#if STRESS_TESTING

	// manual exhaustive test

#endif

#else  // !MANUAL_TESTING

	cout << "Fixed-point conversion validation" << endl;

	nrOfFailedTestCases = ReportTestResult(VerifyConversion<4, 0, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<4,0,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<4, 1, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<4,1,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<4, 2, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<4,2,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<4, 3, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<4,3,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<4, 4, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<4,4,Modulo,uint8_t>");

	nrOfFailedTestCases = ReportTestResult(VerifyConversion<8, 0, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<8,0,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<8, 1, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<8,1,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<8, 2, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<8,2,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<8, 3, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<8,3,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<8, 4, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<8,4,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<8, 5, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<8,5,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<8, 6, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<8,6,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<8, 7, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<8,7,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<8, 8, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<8,8,Modulo,uint8_t>");

	nrOfFailedTestCases = ReportTestResult(VerifyConversion<12, 0, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<12,0,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<12, 1, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<12,1,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<12, 2, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<12,2,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<12, 3, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<12,3,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<12, 4, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<12,4,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<12, 6, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<12,6,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<12, 8, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<12,8,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<12, 10, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<12,10,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<12, 12, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<12,12,Modulo,uint8_t>");

	nrOfFailedTestCases = ReportTestResult(VerifyConversion<16, 0, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<16,0,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<16, 1, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<16,1,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<16, 2, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<16,2,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<16, 3, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<16,3,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<16, 4, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<16,4,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<16, 8, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<16,8,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<16, 12, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<16,12,Modulo,uint8_t>");
	nrOfFailedTestCases = ReportTestResult(VerifyConversion<16, 16, Modulo, uint8_t>(tag, bReportIndividualTestCases), tag, "fixpnt<16,16,Modulo,uint8_t>");

#if STRESS_TESTING

#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught fixpnt internal exception: " << err.what() << std::endl;
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
  Value relationships between fixpnt<nbits+1,rbits+1> and fixpnt<nbits,rbits> we'll use for validation

  To generate:
  	GenerateFixedPointComparisonTable<4, 0>(std::string("-"));
	GenerateFixedPointComparisonTable<4, 1>(std::string("-"));
	GenerateFixedPointComparisonTable<4, 2>(std::string("-"));
	

  fixpnt<5,1>      |    fixpnt<4, 0>
0000.0          0  |  0000.          0
0000.1        0.5  |
0001.0        1.0  |  0001.        1.0
0001.1        1.5  |
0010.0        2.0  |  0010.        2.0
0010.1        2.5  |
0011.0        3.0  |  0011.        3.0
0011.1        3.5  |
0100.0        4.0  |  0100.        4.0
0100.1        4.5  |
0101.0        5.0  |  0101.        5.0
0101.1        5.5  |
0110.0        6.0  |  0110.        6.0
0110.1        6.5  |
0111.0        7.0  |  0111.        7.0
0111.1        7.5  |
1000.0       -8.0  |  1000.       -8.0
1000.1       -7.5  |
1001.0       -7.0  |  1001.       -7.0
1001.1       -6.5  |
1010.0       -6.0  |  1010.       -6.0
1010.1       -5.5  |
1011.0       -5.0  |  1011.       -5.0
1011.1       -4.5  |
1100.0       -4.0  |  1100.       -4.0
1100.1       -3.5  |
1101.0       -3.0  |  1101.       -3.0
1101.1       -2.5  |
1110.0       -2.0  |  1110.       -2.0
1110.1       -1.5  |
1111.0       -1.0  |  1111.       -1.0
1111.1       -0.5  |



  fixpnt<5,2>      |    fixpnt<4, 1>
000.00          0  |  000.0          0
000.01       0.25  |
000.10       0.50  |  000.1        0.5
000.11       0.75  |
001.00        1.0  |  001.0        1.0
001.01       1.25  |
001.10       1.50  |  001.1        1.5
001.11       1.75  |
010.00        2.0  |  010.0        2.0
010.01       2.25  |
010.10       2.50  |  010.1        2.5
010.11       2.75  |
011.00        3.0  |  011.0        3.0
011.01       3.25  |
011.10       3.50  |  011.1        3.5
011.11       3.75  |
100.00       -4.0  |  100.0       -4.0
100.01      -3.75  |
100.10      -3.50  |  100.1       -3.5
100.11      -3.25  |
101.00       -3.0  |  101.0       -3.0
101.01      -2.75  |
101.10      -2.50  |  101.1       -2.5
101.11      -2.25  |
110.00       -2.0  |  110.0       -2.0
110.01      -1.75  |
110.10      -1.50  |  110.1       -1.5
110.11      -1.25  |
111.00       -1.0  |  111.0       -1.0
111.01      -0.75  |
111.10      -0.50  |  111.1       -0.5
111.11      -0.25  |



  fixpnt<5,3>      |    fixpnt<4, 2>
00.000          0  |  00.00          0
00.001      0.125  |
00.010      0.250  |  00.01       0.25
00.011      0.375  |
00.100      0.500  |  00.10       0.50
00.101      0.625  |
00.110      0.750  |  00.11       0.75
00.111      0.875  |
01.000        1.0  |  01.00        1.0
01.001      1.125  |
01.010      1.250  |  01.01       1.25
01.011      1.375  |
01.100      1.500  |  01.10       1.50
01.101      1.625  |
01.110      1.750  |  01.11       1.75
01.111      1.875  |
10.000       -2.0  |  10.00       -2.0
10.001     -1.875  |
10.010     -1.750  |  10.01      -1.75
10.011     -1.625  |
10.100     -1.500  |  10.10      -1.50
10.101     -1.375  |
10.110     -1.250  |  10.11      -1.25
10.111     -1.125  |
11.000       -1.0  |  11.00       -1.0
11.001     -0.875  |
11.010     -0.750  |  11.01      -0.75
11.011     -0.625  |
11.100     -0.500  |  11.10      -0.50
11.101     -0.375  |
11.110     -0.250  |  11.11      -0.25
11.111     -0.125  |
 */
