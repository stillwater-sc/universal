// mpfma.cpp: accuracy/quantization measurement of mixed-precision dot products
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/blas/blas.hpp>

constexpr unsigned FIELD_WIDTH = 8;

template<typename RepresentationType, typename AccumulationType>
void QuantizationExperiment(unsigned nrSamples, unsigned vectorSize, double mean = 0.0, double stddev = 1.0, bool verbose = false) {
	using namespace sw::universal;

	std::cout << "Experiment: nrSamples(" << nrSamples << ") vectorSize(" << vectorSize << ") mean(" << mean << ") stddev(" << stddev << ")\n";
	int64_t L{ vectorSize }, N{ nrSamples };
	blas::vector<double> reference_data(L);
	blas::vector<double> y_data(L);
	gaussian_random(y_data, mean, stddev);
	blas::vector<double> sorted(L);
	blas::vector<RepresentationType> quantized_data(L), quantized_sorted(L), quantized_y(L);
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
		auto quantized_avg = double(blas::sum(quantized_data)) / L;
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
	std::cout << "experimental mean : " << (experimentalMean / N) << '\n';
	std::cout << "quantized    mean : " << (quantizedMean / N) << '\n';

	AccumulationType avg = sum(dotProduct) / N;
	std::cout << "dot product  mean : " << avg << '\n';
}

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	std::streamsize prec = std::cout.precision();
	std::cout << std::setprecision(3);
	
	// generate a set of N vectors of length L in double as reference
	using fp8 = fp8e2m5;
	using fp12 = cfloat<12, 5, uint16_t, true, true, false>; // accumulation type

	double mean{ 0.0 }, stddev{ 1.0 };
	std::cout << "representation type : " << symmetry_range<fp8>() << '\n';
	std::cout << "accumulation type   : " << symmetry_range<fp12>() << '\n';
	unsigned nrSamples{ 10000 };
	QuantizationExperiment<fp8, fp12>(nrSamples, 50, mean, stddev);
	QuantizationExperiment<fp8, fp12>(nrSamples, 100, mean, stddev);
	QuantizationExperiment<fp8, fp12>(nrSamples, 200, mean, stddev);
	QuantizationExperiment<fp8, fp12>(nrSamples, 400, mean, stddev);
	QuantizationExperiment<fp8, fp12>(nrSamples, 600, mean, stddev);
	QuantizationExperiment<fp8, fp12>(nrSamples, 800, mean, stddev);
	QuantizationExperiment<fp8, fp12>(nrSamples, 1000, mean, stddev);
	QuantizationExperiment<fp8, fp12>(nrSamples, 2000, mean, stddev);
	QuantizationExperiment<fp8, fp12>(nrSamples, 4000, mean, stddev);

	std::cout << std::setprecision(prec);

	return EXIT_SUCCESS;
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
