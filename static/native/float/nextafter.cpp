// nextafter.cpp : exploration of the nextafter stdlib function to manipulate units in the last place
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <concepts>
#include <cfenv>
#include <universal/verification/test_suite.hpp>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
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

	std::string test_suite  = "nextafter test";
	std::string test_tag    = "nextafter";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

    {
        float from = 0, to = std::nextafter(from, 1.f);
        std::cout << "The next representable float after " << std::setprecision(20) << from
            << " is " << to
            << std::hexfloat << " (" << to << ")\n" << std::defaultfloat;
    }

    {
        float from = 1, to = std::nextafter(from, 2.f);
        std::cout << "The next representable float after " << from << " is " << to
            << std::hexfloat << " (" << to << ")\n" << std::defaultfloat;
    }

    {
        double from = std::nextafter(0.1, 0), to = 0.1;
        std::cout << "The number 0.1 lies between two valid doubles:\n"
            << std::setprecision(56) << "    " << from
            << std::hexfloat << " (" << from << ')' << std::defaultfloat
            << "\nand " << to << std::hexfloat << "  (" << to << ")\n"
            << std::defaultfloat << std::setprecision(20);
    }

    {
        std::cout << "\nDifference between nextafter and nexttoward:\n";
        float from = 0.0f;
        long double dir = std::nextafter(from, 1.0L); // first subnormal long double
        float x = std::nextafter(from, dir); // first converts dir to float, giving 0
        std::cout << "With nextafter, next float after " << from << " is " << x << '\n';
        x = std::nexttoward(from, dir);
        std::cout << "With nexttoward, next float after " << from << " is " << x << '\n';
    }

    std::cout << "\nSpecial values:\n";
    {
        // #pragma STDC FENV_ACCESS ON
        std::feclearexcept(FE_ALL_EXCEPT);
        double from4 = DBL_MAX, to4 = std::nextafter(from4, INFINITY);
        std::cout << "The next representable double after " << std::setprecision(6)
            << from4 << std::hexfloat << " (" << from4 << ')'
            << std::defaultfloat << " is " << to4
            << std::hexfloat << " (" << to4 << ")\n" << std::defaultfloat;

        if (std::fetestexcept(FE_OVERFLOW))
            std::cout << "   raised FE_OVERFLOW\n";
        if (std::fetestexcept(FE_INEXACT))
            std::cout << "   raised FE_INEXACT\n";
    } // end FENV_ACCESS block

    {
        float from = 0.0, to = std::nextafter(from, -0.0);
        std::cout << "std::nextafter(+0.0, -0.0) gives " << std::fixed << to << '\n';
    }

    auto precision_loss_demo = []<std::floating_point Fp>(const auto rem, const Fp start)
    {
        std::cout << rem;
        for (Fp from = start, to, delta;
            (delta = (to = std::nextafter(from, +INFINITY)) - from) < Fp(10.0);
            from *= Fp(10.0)) {
            std::cout << "nextafter(" << std::scientific << std::setprecision(0) << from
                << ", INF) gives " << std::fixed << std::setprecision(6) << to
                << ";  delta = " << delta << '\n';
        }
    };

    precision_loss_demo("\nPrecision loss demo for float:\n", 10.0f);
    precision_loss_demo("\nPrecision loss demo for double:\n", 10.0e9);

#if LONG_DOUBLE_SUPPORT
    precision_loss_demo("\nPrecision loss demo for long double:\n", 10.0e17L);

	constexpr long double denorm_min = std::numeric_limits<long double>::denorm_min();
	std::cout << "smallest long double: " << to_binary(denorm_min) << " : " << denorm_min << '\n';
#endif


    auto ulp_progression = []<std::floating_point Fp>(const auto rem, const Fp start)
    {
        std::cout << rem;
        for (Fp from = start, to, delta;
            (delta = (to = std::nextafter(from, +INFINITY)) - from) < Fp(10.0);
            from *= Fp(10.0)) {
            std::cout << "ulp(" << std::scientific << std::setprecision(0) << from
                << ") gives " << to_binary(ulp(from)) << " : " 
                << std::fixed << std::setprecision(6) << ulp(from) << '\n';
        }
    };

    ulp_progression("\nULP progression for float:\n", 10.0f);
    ulp_progression("\nULP progression for double:\n", 10.0e9);

    std::cout << '\n';
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
