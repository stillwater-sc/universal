// mpfma.cpp: accuracy/quantization measurement of mixed-precision dot products
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/lns/lns.hpp>
#include <blas/blas.hpp>

constexpr unsigned FIELD_WIDTH = 8;

template<typename RepresentationType, typename AccumulationType>
void QuantizationExperiment(unsigned nrSamples, unsigned vectorSize, double mean = 0.0, double stddev = 1.0, bool verbose = false) {
	using namespace sw::universal;

	std::cout << "Experiment: nrSamples(" << nrSamples << ") vectorSize(" << vectorSize << ") mean(" << mean << ") stddev(" << stddev << ")\n";
	long long L{ vectorSize }, N{ nrSamples };
	blas::vector<double> reference_data(L);
	blas::vector<double> y_data(L);
	gaussian_random(y_data, mean, stddev);
	blas::vector<double> sorted(L);
	blas::vector<RepresentationType> quantized_data(L), quantized_sorted(L), quantized_y(L);
	blas::vector<double> upSampledToDouble(L);
	quantized_y = y_data;
	blas::vector<AccumulationType> upSampled(L);
	blas::vector<AccumulationType> y(L);
	blas::vector<AccumulationType> ySorted(L);
	y = quantized_y; // upsample the y vector for the dot product x * y
	ySorted = quantized_y;
	std::sort(ySorted.begin(), ySorted.end());
	auto ySortedAvg = sum(ySorted) / L;
	if (verbose) {
		std::cout << std::setw(8) << ""
			<< "[ "
			<< std::setw(FIELD_WIDTH) << ySorted[0] << " ... "
			<< std::setw(FIELD_WIDTH) << ySortedAvg << " ... "
			<< std::setw(FIELD_WIDTH) << ySorted[L - 1] << "]\n";
	}

	blas::vector<AccumulationType> dotProduct(N);
	double experimentalMean{ 0.0 };
	double quantizedMean{ 0.0 };
	for (unsigned i = 0; i < N; ++i) {
		gaussian_random(reference_data, mean, stddev);
		sorted = reference_data;
		std::sort(sorted.begin(), sorted.end());
		auto sorted_avg = blas::sum(sorted) / L;
		experimentalMean += sorted_avg;
		
		quantized_data = reference_data;
		upSampledToDouble = quantized_data;
		auto quantized_avg = double(blas::sum(upSampledToDouble)) / L;
		quantizedMean += quantized_avg;

		// dot products in AccumulationType
		upSampled = quantized_data;
		dotProduct[i] = upSampled * y;

		if (verbose) {
			quantized_sorted = sorted;
			std::cout << std::setw(8) << i
				<< "[ "
				<< std::setw(FIELD_WIDTH) << sorted[0] << " ... "
				<< std::setw(FIELD_WIDTH) << sorted_avg << " ... "
				<< std::setw(FIELD_WIDTH) << sorted[L - 1] << "]\n";
			std::cout << std::setw(8) << ""
				<< "[ "
				<< std::setw(FIELD_WIDTH) << quantized_sorted[0] << " ... "
				<< std::setw(FIELD_WIDTH) << quantized_avg << " ... "
				<< std::setw(FIELD_WIDTH) << quantized_sorted[L - 1] << "]\n";
		}
	}
	std::cout << "experimental mean  : " << (experimentalMean / N) << '\n';
	std::cout << "quantized    mean  : " << (quantizedMean / N) << '\n';

	double dot_avg = double(sum(dotProduct)) / N;
	std::cout << "dot product  mean  : " << dot_avg << '\n';
	double dot_stddev = 0;
	for (auto e : dotProduct) {
		auto diff = (double(e) - dot_avg);
		dot_stddev += (diff * diff);
	}
	dot_stddev /= double(N - 1);
	std::cout << "dot product stddev : " << dot_stddev << '\n';
	std::sort(dotProduct.begin(), dotProduct.end());
	std::cout << std::setw(8) << ""
		<< "[ "
		<< std::setw(FIELD_WIDTH) << dotProduct[0] << " ... "
		<< std::setw(FIELD_WIDTH) << "avg(" << dot_avg << ") ... "
		<< std::setw(FIELD_WIDTH) << "median(" << dotProduct[N / 2] << ") ... "
		<< std::setw(FIELD_WIDTH) << dotProduct[N - 1] << "]\n";
//	unsigned i = 0;
//	for (auto e : dotProduct) {
//		std::cout << " " << e;
//		if (i > 0 && (i % 16) == 0) std::cout << '\n';
//	}
}

template<typename  RepresentationType, typename AccumulationType>
void StatisticalSampling(double mean, double stddev) {
	using namespace sw::universal;
	std::cout << "representation type : " << symmetry_range<RepresentationType>() << '\n';
	std::cout << "accumulation type   : " << symmetry_range<AccumulationType>() << '\n';
	unsigned nrSamples{ 10000 };
	QuantizationExperiment<RepresentationType, AccumulationType>(nrSamples, 50, mean, stddev);
	QuantizationExperiment<RepresentationType, AccumulationType>(nrSamples, 100, mean, stddev);
	QuantizationExperiment<RepresentationType, AccumulationType>(nrSamples, 500, mean, stddev);
	QuantizationExperiment<RepresentationType, AccumulationType>(nrSamples, 1000, mean, stddev);
	QuantizationExperiment<RepresentationType, AccumulationType>(nrSamples, 2000, mean, stddev);
	QuantizationExperiment<RepresentationType, AccumulationType>(nrSamples, 4000, mean, stddev);
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

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	std::streamsize prec = std::cout.precision();
	std::cout << std::setprecision(3);
	
#if MANUAL_TESTING

	using namespace sw::universal;
	using RepresentationType = fp8e4m3;
	using AccumulationType = cfloat<12, 5, uint16_t, true, true, false>; // accumulation type
	std::cout << "representation type : " << symmetry_range<RepresentationType>() << '\n';
	std::cout << "accumulation type   : " << symmetry_range<AccumulationType>() << '\n';
	unsigned nrSamples{ 100 };
	double mean{ 0.0 }, stddev{ 1.0 };
	QuantizationExperiment<RepresentationType, AccumulationType>(nrSamples, 50, mean, stddev);

	std::cout << std::setprecision(prec);

	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1

#endif
#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4

	using fp12 = cfloat<12, 5, uint16_t, true, true, false>; // accumulation type

	double mean{ 0 }, stddev{ 1.0 };
	StatisticalSampling<fp8e3m4, fp12>(mean, stddev);
	StatisticalSampling<fp8e4m3, fp12>(mean, stddev);
	StatisticalSampling<fp8e5m2, fp12>(mean, stddev);

#endif

	std::cout << std::setprecision(prec);

	return EXIT_SUCCESS;
#endif
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught universal internal exception: " << err.what() << std::endl;
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
