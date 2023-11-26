// qsnr.cpp: Quantization Signal to Noise ratio for a sampling
//
// Copyright (C) 2022-2023 Stillwater Supercomputing, Inc.
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

// Universal BLAS
#include <universal/blas/blas.hpp>

#include <universal/blas/statistics.hpp>

namespace sw { namespace universal {

		// Function to calculate SNR
		double calculateSNRFP8(const blas::vector<double>& v) {

			size_t N = size(v);

			blas::SummaryStats stats = blas::summaryStatistics(v);
			auto stddev = stats.stddev;
			
			double sum = 0.0;
			for (auto number : v) {
				double quantized = round(number * 255) / 255; // Quantize to FP8
				double error = number - quantized;
				std::cout << number << " : " << quantized << " : " << error << '\n';
				sum += error * error;
			}

			double noise_power = sum / N;
			double signal_power = stddev * stddev;
			double SNR = 10 * log10(signal_power / noise_power);

			return SNR;
		}

		template<typename Scalar>
		double calculateSNR(const blas::vector<double>& v) {
			std::cout << type_tag(Scalar()) << " : " << symmetry_range<Scalar>() << '\n';
			size_t N = size(v);

			blas::SummaryStats stats = blas::summaryStatistics(v);
			auto stddev = stats.stddev;

			double sum = 0.0;
			for (auto number : v) {
				double quantized = double(Scalar(number)); // Quantize to Scalar
				double error = number - quantized;
				// std::cout << number << " : " << quantized << " : " << error << '\n';
				sum += error * error;
			}

			double noise_power = sum / N;
			double signal_power = stddev * stddev;
			double SNR = 10 * log10(signal_power / noise_power);

			return SNR;
		}
}
}

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

	constexpr int nrExperiments = 1;

	for (int i = 0; i < nrExperiments; ++i) {
		constexpr unsigned N = 10000;
		constexpr double mean = 0.0;
		constexpr double stddev = 1.0;
		auto data = sw::universal::blas::gaussian_random_vector<double>(N, mean, stddev);
		std::cout << "fixpnt<8,2> : " << calculateSNR<fixpnt<8, 2>>(data) << '\n';
		std::cout << "fixpnt<8,3> : " << calculateSNR<fixpnt<8, 3>>(data) << '\n';
		std::cout << "fixpnt<8,4> : " << calculateSNR<fixpnt<8, 4>>(data) << '\n';
		std::cout << "fixpnt<8,5> : " << calculateSNR<fixpnt<8, 5>>(data) << '\n';
		std::cout << "fp8e2m5     : " << calculateSNR<fp8e2m5>(data) << '\n';
		std::cout << "fp8e3m4     : " << calculateSNR<fp8e3m4>(data) << '\n';
		std::cout << "fp8e4m3     : " << calculateSNR<fp8e4m3>(data) << '\n';
		std::cout << "fp8e5m2     : " << calculateSNR<fp8e5m2>(data) << '\n';
		std::cout << "posit<8,0>  : " << calculateSNR<posit<8, 0>>(data) << '\n';
		std::cout << "posit<8,1>  : " << calculateSNR<posit<8, 1>>(data) << '\n';
		std::cout << "posit<8,2>  : " << calculateSNR<posit<8, 2>>(data) << '\n';
		std::cout << "posit<8,3>  : " << calculateSNR<posit<8, 3>>(data) << '\n';
		std::cout << "lns<8,2>    : " << calculateSNR<lns<8, 2>>(data) << '\n';
		std::cout << "lns<8,3>    : " << calculateSNR<lns<8, 3>>(data) << '\n';
		std::cout << "lns<8,4>    : " << calculateSNR<lns<8, 4>>(data) << '\n';
		std::cout << "lns<8,5>    : " << calculateSNR<lns<8, 5>>(data) << '\n';
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
