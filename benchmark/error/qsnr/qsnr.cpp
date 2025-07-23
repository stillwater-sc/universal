// qsnr.cpp: Quantization Signal to Noise ratio for a sampling
//     
// Copyright(c) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT 
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/integer/integer.hpp>
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/fixpnt/fixpnt.hpp>
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/cfloat/cfloat.hpp>
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#define LNS_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/lns/lns.hpp>

// Stillwater BLAS library
#include <blas/blas.hpp>
#include <universal/quantization/qsnr.hpp>

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
	using namespace sw::blas;
	using namespace sw::numeric::containers;

	constexpr int nrExperiments = 10;
	std::map<std::string, vector<double>> table;
	std::vector<std::string> arithmeticTypename = {
		"fixpnt<8,2>",
		"fixpnt<8,3>",
		"fixpnt<8,4>",
		"fixpnt<8,5>",
		"fp8e2m5",
		"fp8e3m4",
		"fp8e4m3",
		"fp8e5m2",
		"posit<8,0>",
		"posit<8,1>",
		"posit<8,2>",
		"posit<8,3>",
		"lns<8,2>",
		"lns<8,3>",
		"lns<8,4>",
		"lns<8,5>"
	};

	for (int i = 0; i < nrExperiments; ++i) {
		constexpr unsigned N = 32;
		constexpr double mean = 0.0;
		constexpr double stddev = 1.0;
		auto data = gaussian_random_vector<double>(N, mean, stddev);
		table["fixpnt<8,2>"].push_back(qsnr<fixpnt<8, 2>>(data));
		table["fixpnt<8,3>"].push_back(qsnr<fixpnt<8, 3>>(data));
		table["fixpnt<8,4>"].push_back(qsnr<fixpnt<8, 4>>(data));
		table["fixpnt<8,5>"].push_back(qsnr<fixpnt<8, 5>>(data));
		table["fp8e2m5"].push_back(qsnr<fp8e2m5>(data));
		table["fp8e3m4"].push_back(qsnr<fp8e3m4>(data));
		table["fp8e4m3"].push_back(qsnr<fp8e4m3>(data));
		table["fp8e5m2"].push_back(qsnr<fp8e5m2>(data));
		table["posit<8,0>"].push_back(qsnr<posit<8, 0>>(data));
		table["posit<8,1>"].push_back(qsnr<posit<8, 1>>(data));
		table["posit<8,2>"].push_back(qsnr<posit<8, 2>>(data));
		table["posit<8,3>"].push_back(qsnr<posit<8, 3>>(data));
		table["lns<8,2>"].push_back(qsnr<lns<8, 2>>(data));
		table["lns<8,3>"].push_back(qsnr<lns<8, 3>>(data));
		table["lns<8,4>"].push_back(qsnr<lns<8, 4>>(data));
		table["lns<8,5>"].push_back(qsnr<lns<8, 5>>(data));
	}

	for (auto tag : arithmeticTypename) {
		std::cout << std::setw(15) << tag << " : " << quantiles(table[tag]) << '\n';
	}

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught internal exception: " << err.what() << std::endl;
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
