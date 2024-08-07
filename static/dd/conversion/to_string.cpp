// to_string.cpp: test suite runner for the string conversion operators for double-double (dd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <version>
#if __cpp_lib_format >= 202311L
constexpr auto revision() { return " (post C++26)"; }
#include <format>
#else
constexpr auto revision() { return " (pre C++26)"; }
#endif

#include <cstdio>
#include <initializer_list>
#include <iostream>
#include <string>
#include <algorithm>
#include <random>

#include <universal/number/dd/dd.hpp>
#include <universal/verification/test_suite.hpp>


namespace sw {
	namespace universal {

		/*
		* the width requested is subservient to the precision, that is, if precision is width < precision + surplus, you are
		* going to get a string that is precision + surplus
		* in scientific format the surplus is 7 characters 1.precisionE+300;
		 */
		template<typename TestType>
		void ScanWidth(TestType v, int precision = 7) {
			std::cout << type_tag(v) << '\n';
			for (int width = precision; width < precision + 7; ++width) {
				std::cout << "precision    : " << precision << '\n';
				std::cout << "columnWidth  : " << width << '\n';
				std::cout << "default      : _" << std::setprecision(precision) << std::setw(width) << v << '_' << '\n';
				std::cout << "scientific   : _" << std::scientific << std::setprecision(precision) << std::setw(width) << v << '_' << '\n';
				std::cout << "fixed        : _" << std::fixed << std::setprecision(precision) << std::setw(width) << v << '_' << '\n';
				std::cout << std::defaultfloat;
			}
		}
		template<typename TestType>
		void ScanPrecision(TestType v) {
			std::cout << type_tag(v) << '\n';
			for (int i = 0; i < 6; ++i) {
				int precision = 7 * i;
				int width = precision + 7;
				std::cout << "precision    : " << precision << '\n';
				std::cout << "columnWidth  : " << width << '\n';
				std::cout << "default      : _" << std::setprecision(precision) << std::setw(width) << v << '_' << '\n';
				std::cout << "scientific   : _" << std::scientific << std::setprecision(precision) << std::setw(width) << v << '_' << '\n';
				std::cout << "fixed        : _" << std::fixed << std::setprecision(precision) << std::setw(width) << v << '_' << '\n';
				std::cout << std::defaultfloat;
			}
		}

		void ScanTest(int selection) {
			// 2^166 = exponent of 1e50
			double clean = std::ldexp(1.0, 170);  // construct a double with the fraction bits of 1.0 and the exponent of 2^170
			switch (selection) {
			case 0:
			{
				// reference native double
				double base{ clean };
				for (int i = 0; i < 7; ++i) {
					std::cout << to_hex(base) << " : " << to_binary(base) << '\n';
					ScanPrecision(base);
					base *= clean;
				}
			}
			break;
			case 1:
			{
				// comparative double-double
				dd base{ clean };
				for (int i = 0; i < 7; ++i) {
					std::cout << to_hex(double(base)) << " : " << to_binary(double(base)) << '\n';
					ScanPrecision(base);
					base *= clean;
				}
			}
			break;
			}
		}
		int VerifyStreamRoundTrip(bool reportTestCases, dd seed, std::streamsize precision = 32, std::streamsize width = 35, unsigned nrTrials = 10) {
			int nrOfTestFailures{ 0 };
			dd a{seed}, b{};
			
			std::random_device rd;     // get a random seed from the OS entropy device
			std::mt19937_64 eng(rd()); // use the 64-bit Mersenne Twister 19937 generator and seed it with entropy.
			double min = -1024 * 1024, max = 1024 * 1024;
			std::uniform_real_distribution <double> distr(min, max); // define the distribution

			for (unsigned i = 0; i < nrTrials; ++i) {
				std::stringstream s1;
				s1 << std::setprecision(precision) << std::setw(width) << a << ' ';
				std::string input = s1.str();
				//std::cout << "out: " << input;
				s1 >> b;
				std::stringstream s2;
				s2 << std::setprecision(precision) << std::setw(width) << b << ' ';
				std::string output = s2.str();
				//std::cout << "in : " << output << '\n';
				if (output != input) {
					++nrOfTestFailures;
					if (reportTestCases) std::cerr << "FAIL: " << input << " != " << output << '\n';
				}
				a = seed * distr(eng);
			}
			return nrOfTestFailures;
		}
	}
}
// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite         = "doubledouble string conversion validation";
	std::string test_tag           = "doubledouble string conversion";
	bool reportTestCases           = true;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING


	ScanTest(0); // reference native double
	ScanTest(1); // comparative double-double

	double dSeed = 1.0e50;
	std::cout << scale(dSeed) << '\n';
	dd seed(dSeed);
	nrOfFailedTestCases += VerifyStreamRoundTrip(reportTestCases, seed, 7, 10, 3);
	nrOfFailedTestCases += VerifyStreamRoundTrip(reportTestCases, seed, 10, 15, 3);
	nrOfFailedTestCases += VerifyStreamRoundTrip(reportTestCases, seed, 25, 30, 3);
	nrOfFailedTestCases += VerifyStreamRoundTrip(reportTestCases, seed, 32, 35, 3);


	for (const double f : {1.23456789555555, 23.43, 1e-9, 1e40, 1e-40, 123456789.0}) {
		std::cout << "to_string:\t" << std::to_string(f) << revision() << '\n';

		// Before C++26, the output of std::to_string matches std::printf.
		std::printf("printf:\t\t%f\n", f);

		// As of C++26, the output of std::to_string matches std::format.
//		std::cout << std::format("format:\t\t{}\n", f);

		std::cout << "std::cout:\t" << f << "\n\n";
	}


	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	dd seed{ 125.125125125125125125125 };

	return 0;
	nrOfFailedTestCases += VerifyStreamRoundTrip(reportTestCases, seed, 7, 7);
	std::cout << "failures " << nrOfFailedTestCases << '\n';
	nrOfFailedTestCases += VerifyStreamRoundTrip(reportTestCases, seed, 10, 7);
	std::cout << "failures " << nrOfFailedTestCases << '\n';
	nrOfFailedTestCases += VerifyStreamRoundTrip(reportTestCases, seed, 15, 7);
	std::cout << "failures " << nrOfFailedTestCases << '\n';
	nrOfFailedTestCases += VerifyStreamRoundTrip(reportTestCases, seed, 25, 7);
	std::cout << "failures " << nrOfFailedTestCases << '\n';
	nrOfFailedTestCases += VerifyStreamRoundTrip(reportTestCases, seed, 32, 7);
	std::cout << "failures " << nrOfFailedTestCases << '\n';

	seed.assign("0.333333333333333333333333333333333333333");
	nrOfFailedTestCases += VerifyStreamRoundTrip(reportTestCases, seed, 7, 10);
	nrOfFailedTestCases += VerifyStreamRoundTrip(reportTestCases, seed, 10, 15);
	nrOfFailedTestCases += VerifyStreamRoundTrip(reportTestCases, seed, 25, 30);
	nrOfFailedTestCases += VerifyStreamRoundTrip(reportTestCases, seed, 32, 35);
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
