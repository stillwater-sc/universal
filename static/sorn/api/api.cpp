// api.cpp: test suite runner to demonstrate public API of SORNs
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/number/sorn/sorn.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "sorn API demonstration";
	std::string test_tag    = "api";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	
	// headline
	std::cout << "------------------------\n" << "SORNuniversal playground\n" << "------------------------\n\n";

	// initialize SORN class
	using sornType = sw::universal::sorn<0, 4, 8>; // set up SORN datatype (linear)
	sornType s1 = 1; // assign value
	std::cout << "-- Length of sornType:\t\tsornBits: " << s1.sornBits << ", sornDT.size(): " << s1.sornDT.size() << '\n'; // print datatype length
	std::cout << s1.getConfig(); // print config parameters
	std::cout << s1.getDT(); // print datatype
	std::cout << "-- s1 has the value: " << s1 << '\n'; // print value

	// binary value handling
	std::bitset<s1.sornBits> s1Bits = s1.getBits(); // get binary value
	std::cout << "-- Binary value of s1: " << s1Bits << '\n'; // print binary value
	std::bitset<s1.sornBits> b1 = std::bitset<s1.sornBits>{ "00000001111000000000" };
	s1.setBits(b1); // set sorn value via binary input
	std::cout << "-- s1 set via binary input " << b1 << " has the value " << s1 << '\n'; // print value

	// arithmetics
	std::cout << "\n%% Arithmetic operations: \n\n";

	// addition + subtraction
	sornType s2 = 1.3; // test values
	sornType s3 = 0.7;
	sornType s4 = s2 + s3; // two operand addition
	sornType s5 = s1 - s3; // two operand subtraction
	double f1 = -0.5;
	sornType s6 = f1 + s2; // scalar addition
	sornType s7 = f1 - s2; // scalar subtraction
	std::cout << "-- Addition: \t\t" << s2 << " + " << s3 << " = " << s4 << '\n'; // print results
	std::cout << "-- Subtraction: \t" << s1 << " - " << s3 << " = " << s5 << '\n';
	std::cout << "-- Scalar Addition: \t" << f1 << " + " << s2 << " = " << s6 << '\n';
	std::cout << "-- Scalar Subtraction: \t" << f1 << " - " << s2 << " = " << s7 << "\n\n";

	// multiplication
	std::cout << "-- Multiplication: \n";
	sornType aPos = 0.6;
	sornType aNeg = -aPos;
	sornType aMid = aPos - aPos;
	sornType bPos = 1.6;
	sornType bNeg = -bPos;
	sornType bMid = bPos - bPos;

	sornType c1 = aPos * bPos; // case 1
	std::cout << "-- Case 1: \t" << aPos << " * " << bPos << " = " << c1 << '\n';
	sornType c2 = aPos * bMid; // case 2
	std::cout << "-- Case 2: \t" << aPos << " * " << bMid << " = " << c2 << '\n';
	sornType c3 = aPos * bNeg; // case 3
	std::cout << "-- Case 3: \t" << aPos << " * " << bNeg << " = " << c3 << '\n';
	sornType c4 = aMid * bPos; // case 4
	std::cout << "-- Case 4: \t" << aMid << " * " << bPos << " = " << c4 << '\n';
	sornType c5 = aMid * bMid; // case 5
	std::cout << "-- Case 5: \t" << aMid << " * " << bMid << " = " << c5 << '\n';
	sornType c6 = aMid * bNeg; // case 6
	std::cout << "-- Case 6: \t" << aMid << " * " << bNeg << " = " << c6 << '\n';
	sornType c7 = aNeg * bPos; // case 7
	std::cout << "-- Case 7: \t" << aNeg << " * " << bPos << " = " << c7 << '\n';
	sornType c8 = aNeg * bMid; // case 8
	std::cout << "-- Case 8: \t" << aNeg << " * " << bMid << " = " << c8 << '\n';
	sornType c9 = aNeg * bNeg; // case 9
	std::cout << "-- Case 9: \t" << aNeg << " * " << bNeg << " = " << c9 << "\n\n";

return EXIT_SUCCESS;

	sornType s8 = s2 * f1;
	std::cout << "-- Scalar Multiplication: \t" << s2 << " * " << f1 << " = " << s8 << std::endl;
	std::cout << '\n';

	// abs
	sornType s3Abs = s3.abs();
	std::cout << "-- abs( " << s3 << " ) = " << s3Abs << std::endl;
	sornType s7Abs = s7.abs();
	std::cout << "-- abs( " << s7 << " ) = " << s7Abs << std::endl;
	sornType s1Abs = s1.abs();
	std::cout << "-- abs( " << s1 << " ) = " << s1Abs << std::endl;
	sornType s5Abs = abs(s5);
	std::cout << "-- abs( " << s5 << " ) = " << s5Abs << std::endl;

	// hypot function
	sornType s9 = hypot(s1, s2);
	std::cout << "-- hypot(" << s1 << " , " << s2 << " ) = " << s9 << std::endl;

	std::cout << '\n';

	// log sorn DT
	using sornLogType = sw::universal::sorn<-2, 2, 1, 0, 1, 1, 1, 1>;
	sornLogType logVal = 0.001;
	std::cout << "-- Length of sornLogType:\tsornBits: " << logVal.sornBits << ", sornDT.size(): " << logVal.sornDT.size() << '\n';
	std::cout << logVal.getConfig();
	std::cout << logVal.getDT();
	std::cout << "-- logVal has the value: " << logVal << '\n';
	std::cout << '\n';


	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught unexpected universal internal exception: " << err.what() << std::endl;
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
