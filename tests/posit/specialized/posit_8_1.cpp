// posit_8_1.cpp: Functionality tests for posit<8,1>
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the posit template environment
// first: enable fast specialized posit<8,1>
#define POSIT_FAST_POSIT_8_1 1
// second: enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/posit/posit>
#include "../../test_helpers.hpp"
#include "../../posit_test_helpers.hpp"
#include "../../posit_math_helpers.hpp"

/*
Standard posits with nbits = 8 have no exponent bits, i.e. es = 0.
*/
template<size_t nbits, size_t es>
void placeholder(sw::unum::posit<nbits, es>& a) {
	using namespace std;
	using namespace sw::unum;
	bool		     	 _sign;
	regime<nbits, es>    _regime;
	exponent<nbits, es>  _exponent;
	fraction<5>          _fraction;
	bitblock<nbits>		 _raw_bits;
	decode(a.get(), _sign, _regime, _exponent, _fraction);
	double s = (_sign ? -1.0 : 1.0);
	double r = _regime.value();
	double e = _exponent.value();
	double f = (1.0 + _fraction.value());
	cout << "fraction = " << _fraction << " faction value = " << f << endl;
}

void GenerateValues() {
	using namespace std;
	using namespace sw::unum;
	posit<8, 1> a;
	for (unsigned int i = 0; i < 256; ++i) {
		a.set_raw_bits(i);
		cout << hex << i << " " << dec << a << endl;
	}
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	// const size_t RND_TEST_CASES = 0;  // no randoms, 8-bit posits can be done exhaustively

	const size_t nbits = 8;
	const size_t es = 1;

	GenerateValues();
	posit<nbits, es> a = 15;
	cout << a << endl;
	return 0;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;
	std::string tag = " posit<8,1>";

#if POSIT_FAST_POSIT_8_1
	cout << "Fast specialization posit<8,1> configuration tests" << endl;
#else
	cout << "Standard posit<8,1> configuration tests" << endl;
#endif

	posit<nbits, es> p;
	cout << dynamic_range(p) << endl;

	// special cases
	p = 0;
	if (!p.iszero()) ++nrOfFailedTestCases;
	p = NAN;
	if (!p.isnar()) ++nrOfFailedTestCases;
	p = INFINITY;
	if (!p.isnar()) ++nrOfFailedTestCases;

	// logic tests
	cout << "Logic operator tests " << endl;
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicEqual             <nbits, es>(), tag, "    ==         (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicNotEqual          <nbits, es>(), tag, "    !=         (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicLessThan          <nbits, es>(), tag, "    <          (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicLessOrEqualThan   <nbits, es>(), tag, "    <=         (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicGreaterThan       <nbits, es>(), tag, "    >          (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidatePositLogicGreaterOrEqualThan<nbits, es>(), tag, "    >=         (native)  ");

	// conversion tests
	cout << "Assignment/conversion tests " << endl;
	nrOfFailedTestCases += ReportTestResult( ValidateIntegerConversion<nbits, es>(tag, bReportIndividualTestCases), tag, "integer assign (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateConversion       <nbits, es>(tag, bReportIndividualTestCases), tag, "float assign   (native)  ");

	// arithmetic tests
	cout << "Arithmetic tests " << endl;
	nrOfFailedTestCases += ReportTestResult( ValidateAddition         <nbits, es>(tag, bReportIndividualTestCases), tag, "add            (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateSubtraction      <nbits, es>(tag, bReportIndividualTestCases), tag, "subtract       (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateMultiplication   <nbits, es>(tag, bReportIndividualTestCases), tag, "multiply       (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateDivision         <nbits, es>(tag, bReportIndividualTestCases), tag, "divide         (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateNegation         <nbits, es>(tag, bReportIndividualTestCases), tag, "negate         (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateReciprocation    <nbits, es>(tag, bReportIndividualTestCases), tag, "reciprocate    (native)  ");

	// elementary function tests
	cout << "Elementary function tests " << endl;
//	nrOfFailedTestCases += ReportTestResult( ValidateSqrt             <nbits, es>(tag, bReportIndividualTestCases), tag, "sqrt           (native)  ");
	nrOfFailedTestCases += ReportTestResult( ValidateExp              <nbits, es>(tag, bReportIndividualTestCases), tag, "exp                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateExp2             <nbits, es>(tag, bReportIndividualTestCases), tag, "exp2                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateLog              <nbits, es>(tag, bReportIndividualTestCases), tag, "log                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateLog2             <nbits, es>(tag, bReportIndividualTestCases), tag, "log2                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateLog10            <nbits, es>(tag, bReportIndividualTestCases), tag, "log10                    ");
	nrOfFailedTestCases += ReportTestResult( ValidateSine             <nbits, es>(tag, bReportIndividualTestCases), tag, "sin                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateCosine           <nbits, es>(tag, bReportIndividualTestCases), tag, "cos                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateTangent          <nbits, es>(tag, bReportIndividualTestCases), tag, "tan                      ");
	nrOfFailedTestCases += ReportTestResult( ValidateAtan             <nbits, es>(tag, bReportIndividualTestCases), tag, "atan                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateAsin             <nbits, es>(tag, bReportIndividualTestCases), tag, "asin                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateAcos             <nbits, es>(tag, bReportIndividualTestCases), tag, "acos                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateSinh             <nbits, es>(tag, bReportIndividualTestCases), tag, "sinh                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateCosh             <nbits, es>(tag, bReportIndividualTestCases), tag, "cosh                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateTanh             <nbits, es>(tag, bReportIndividualTestCases), tag, "tanh                     ");
	nrOfFailedTestCases += ReportTestResult( ValidateAtanh            <nbits, es>(tag, bReportIndividualTestCases), tag, "atanh                    ");
	nrOfFailedTestCases += ReportTestResult( ValidateAcosh            <nbits, es>(tag, bReportIndividualTestCases), tag, "acosh                    ");
	nrOfFailedTestCases += ReportTestResult( ValidateAsinh            <nbits, es>(tag, bReportIndividualTestCases), tag, "asinh                    ");

	nrOfFailedTestCases += ReportTestResult( ValidatePowerFunction    <nbits, es>(tag, bReportIndividualTestCases), tag, "pow                      ");

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
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
