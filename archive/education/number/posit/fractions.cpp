//  fractions.cpp : examples of working with posit fractions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/posit1/posit1.hpp>

// test reporting helper
int ReportTestResult(int nrOfFailedTests, const std::string& description, const std::string& test_operation)
{
	if (nrOfFailedTests > 0) {
		std::cout << description << " " << test_operation << " FAIL " << nrOfFailedTests << " failed test cases" << std::endl;
	}
	else {
		std::cout << description << " " << test_operation << " PASS" << std::endl;
	}
	return nrOfFailedTests;
}

template<unsigned fbits>
void ReportError(const std::string& test_case, const std::string& op, double input, double reference, const sw::universal::positFraction<fbits>& _fraction) {
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(10) << input
		<< " did not convert to "
		<< std::setw(10) << reference << " instead it yielded "
		<< std::setw(10) << _fraction.value()
		<< std::endl;
}

template<unsigned fbits>
int ValidateFractionValue(const std::string& tag, bool bReportIndividualTestCases)
{
	const uint64_t NR_OF_FRACTIONS = (uint64_t(1) << fbits);
	int nrOfFailedTests = 0;

	sw::universal::positFraction<fbits> _fraction;
	double divisor = uint64_t(1) << fbits;
	for (uint64_t i = 0; i < NR_OF_FRACTIONS; i++) {
		sw::universal::bitblock<fbits> bits = sw::universal::convert_to_bitblock<fbits, uint64_t>(i);
		_fraction.set(bits);  // use default nr of fraction bits to be full size
		// fraction value is the 'fraction' of the operand: (fraction to ull)/2^fbits
		double v = _fraction.value();
		double reference = double(i) / divisor;
		// std::cout << "fraction = " << _fraction << " value " << v << " reference " << reference << std::endl;
		if (std::abs(v - reference) > 0.0000000001) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportError("FAIL", "to value", v, reference, _fraction);
		}
	}
	return nrOfFailedTests;
}

template<unsigned fbits>
int ValidateFixedPointNumber(const std::string& tag, bool bReportIndividualTestCases)
{
	const uint64_t NR_OF_FRACTIONS = (uint64_t(1) << fbits);
	int nrOfFailedTests = 0;

	double divisor = uint64_t(1) << fbits;
	sw::universal::positFraction<fbits> _fraction;
	sw::universal::bitblock<fbits + 1> _fixed_point;
	for (uint64_t i = 0; i < NR_OF_FRACTIONS; i++) {
		sw::universal::bitblock<fbits> bits = sw::universal::convert_to_bitblock<fbits, uint64_t>(i);
		_fraction.set(bits);  // use default nr of fraction bits to be full size
							  // fraction value is the 'fraction' of the operand: (fraction to ull)/2^fbits
		double v = 1.0 + _fraction.value();
		double reference = 1.0 + double(i) / divisor;
		_fixed_point = _fraction.get_fixed_point();
		//std::cout << "fixed_point = " << _fixed_point << " value " << v << " reference " << reference << std::endl;
		if (std::abs(v - reference) > 0.0000000001) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportError("FAIL", "to value", v, reference, _fraction);
		}
	}
	return nrOfFailedTests;
}

template<unsigned fbits>
int ValidateRoundingAssessment(const std::string& tag, bool bReportIndividualTestCases) {
	int nrOfFailedTests = 0;

	sw::universal::positFraction<fbits> _fraction;
	sw::universal::bitblock<fbits> bits = sw::universal::convert_to_bitblock<fbits, uint32_t>(0x50);
	for (unsigned i = 0; i < fbits; i++) {
		bool rb = _fraction.assign2(i, bits);
		std::cout << "#fbits = " << i << " " << bits << " fraction " << _fraction << " " << (rb ? "up" : "dn") << '\n';
	}
	std::cout << std::endl;

	return nrOfFailedTests;
}

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	// generate individual testcases to hand trace/debug
	ValidateFixedPointNumber<4>("Hello", true);
	ValidateRoundingAssessment<8>("", true);

	std::cout << "Fraction tests\n";
	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = false;

	nrOfFailedTestCases += ReportTestResult(ValidateFixedPointNumber<3>("Fixed point conversion failed: ", bReportIndividualTestCases), "fraction<3>", "get_fixed_point()");
	nrOfFailedTestCases += ReportTestResult(ValidateFixedPointNumber<4>("Fixed point conversion failed: ", bReportIndividualTestCases), "fraction<4>", "get_fixed_point()");
	nrOfFailedTestCases += ReportTestResult(ValidateFixedPointNumber<5>("Fixed point conversion failed: ", bReportIndividualTestCases), "fraction<5>", "get_fixed_point()");
	nrOfFailedTestCases += ReportTestResult(ValidateFixedPointNumber<6>("Fixed point conversion failed: ", bReportIndividualTestCases), "fraction<6>", "get_fixed_point()");
	nrOfFailedTestCases += ReportTestResult(ValidateFixedPointNumber<7>("Fixed point conversion failed: ", bReportIndividualTestCases), "fraction<7>", "get_fixed_point()");
	nrOfFailedTestCases += ReportTestResult(ValidateFixedPointNumber<8>("Fixed point conversion failed: ", bReportIndividualTestCases), "fraction<8>", "get_fixed_point()");

	nrOfFailedTestCases += ReportTestResult(ValidateFractionValue<3>("Value conversion failed: ", bReportIndividualTestCases), "fraction<3>", "value()");
	nrOfFailedTestCases += ReportTestResult(ValidateFractionValue<4>("Value conversion failed: ", bReportIndividualTestCases), "fraction<4>", "value()");
	nrOfFailedTestCases += ReportTestResult(ValidateFractionValue<5>("Value conversion failed: ", bReportIndividualTestCases), "fraction<5>", "value()");
	nrOfFailedTestCases += ReportTestResult(ValidateFractionValue<6>("Value conversion failed: ", bReportIndividualTestCases), "fraction<6>", "value()");
	nrOfFailedTestCases += ReportTestResult(ValidateFractionValue<7>("Value conversion failed: ", bReportIndividualTestCases), "fraction<7>", "value()");
	nrOfFailedTestCases += ReportTestResult(ValidateFractionValue<8>("Value conversion failed: ", bReportIndividualTestCases), "fraction<8>", "value()");
	nrOfFailedTestCases += ReportTestResult(ValidateFractionValue<9>("Value conversion failed: ", bReportIndividualTestCases), "fraction<9>", "value()");
	nrOfFailedTestCases += ReportTestResult(ValidateFractionValue<10>("Value conversion failed: ", bReportIndividualTestCases), "fraction<10>", "value()");
	nrOfFailedTestCases += ReportTestResult(ValidateFractionValue<12>("Value conversion failed: ", bReportIndividualTestCases), "fraction<12>", "value()");
	nrOfFailedTestCases += ReportTestResult(ValidateFractionValue<14>("Value conversion failed: ", bReportIndividualTestCases), "fraction<14>", "value()");
	nrOfFailedTestCases += ReportTestResult(ValidateFractionValue<16>("Value conversion failed: ", bReportIndividualTestCases), "fraction<16>", "value()");
	nrOfFailedTestCases += ReportTestResult(ValidateFractionValue<18>("Value conversion failed: ", bReportIndividualTestCases), "fraction<18>", "value()");


#if STRESS_TESTING
	nrOfFailedTestCases += ReportTestResult(ValidateFractionValue<24>("Value conversion failed: ", bReportIndividualTestCases), "fraction<24>", "value()");
	nrOfFailedTestCases += ReportTestResult(ValidateFractionValue<28>("Value conversion failed: ", bReportIndividualTestCases), "fraction<28>", "value()");
	//nrOfFailedTestCases += ReportTestResult(ValidateFractionValue<32>("Value conversion failed: ", bReportIndividualTestCases), "fraction<32>", "value()");
#endif


	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
