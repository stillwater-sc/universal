// experiments.cpp: experiments with the quad-double floating-point number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <numbers>
// minimum set of include files to reflect source code dependencies
// Configure the qd template environment
// enable/disable arithmetic exceptions
#define DOUBLEDOUBLE_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/qd/qd.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/native/error_free_ops.hpp>

namespace sw {
	namespace universal {




	}
}



int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "quad-double (qd) experiments";
	int nrOfFailedTestCases = 0;

	auto oldPrec = std::cout.precision();



	std::cout << std::setprecision(oldPrec);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}


/*
void ThreeSumExperiments() {
	using namespace sw::universal;
	std::cout << "three_sum experiments\n";

	double a{ 1.0 };
	double b{ ulp(a) / 2.0 };
	double c{ ulp(b) / 2.0 };
	ReportValue(a, "a = 1.0");
	ReportValue(b, "b = ulp(1.0)/2");
	ReportValue(c, "c = ulp(b)/2");

	std::cout << "two_sum\n";
	double r{ 0 };
	double s = two_sum(a, b, r);
	ReportValue(s, "sum");
	ReportValue(r, "residual");

	std::cout << "three_sum\n";
	double aa{ a }, bb{ b }, cc{ c };
	ReportValue(a, "a");
	ReportValue(b, "b");
	ReportValue(c, "c");
	three_sum(a, b, c);
	ReportValue(a, "a");
	ReportValue(b, "b");
	ReportValue(c, "c");

	std::cout << "three_sum2\n";
	a = aa, b = bb, c = cc; // reload
	ReportValue(a, "a");
	ReportValue(b, "b");
	ReportValue(c, "c");
	three_sum2(a, b, c);
	ReportValue(a, "a");
	ReportValue(b, "b");
	// c is unchanged

	std::cout << "three_sum3\n";
	a = aa, b = bb, c = cc; // reload
	ReportValue(a, "a");
	ReportValue(b, "b");
	ReportValue(c, "c");
	double sum = three_sum3(a, b, c);
	ReportValue(sum, "three_sum3");

}

three_sum experiments
             a = 1.0 : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
      b = ulp(1.0)/2 : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
        c = ulp(b)/2 : 0b0.011'1001'0101.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.232595e-32
two_sum
                 sum : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
            residual : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
three_sum
 in                a : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
                   b : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
                   c : 0b0.011'1001'0101.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.232595e-32
 out               a : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
                   b : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
                   c : 0b0.011'1001'0101.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.232595e-32
three_sum2
 in                a : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
                   b : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
                   c : 0b0.011'1001'0101.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.232595e-32
 out               a : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
                   b : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
three_sum3
 in                a : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1
                   b : 0b0.011'1100'1010.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.110223e-16
                   c : 0b0.011'1001'0101.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1.232595e-32
 out      three_sum3 : 0b0.011'1111'1111.0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000 : 1

*/