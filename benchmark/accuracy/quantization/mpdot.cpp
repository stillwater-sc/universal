// mpfma.cpp: accuracy/quantization measurement of mixed-precision dot products
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/blas/blas.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

constexpr unsigned FIELD_WIDTH = 8;

namespace sw {
	namespace universal {

		template<typename InputType, typename ProductType, typename AccumulationType, typename OutputType>
		OutputType DotProductExperiment(const blas::vector<InputType>& a, const blas::vector<InputType>& b) {
			size_t Na = size(a);
			size_t Nb = size(b);
			assert(Na == Nb && "vectors are not of the same length");

			// upsample the inputs for the multiplication step
			blas::vector<ProductType> aa(Na), cc(Nb);
			aa = a; cc = b;
			// element-wise product
			cc *= aa;

			// upsample to accumulation type
			blas::vector<AccumulationType> acc(Na);
			acc = cc;

			OutputType result = sum(acc);

			return result;
		}

		/// <summary>
		/// generate the custom dot products
		/// </summary>
		/// <param name="data">input vectors</param>
		/// <param name="dots">output dot product results</param>
		template<typename InputType, typename ProductType, typename AccumulationType, typename OutputType>
		void GenerateSpecializedDotProducts(const std::vector<sw::universal::blas::vector<InputType>>& data, std::vector<OutputType>& dots) {
			using namespace sw::universal;
			size_t N = size(data);
			dots.resize(N);
			for (size_t i = 0; i < N; ++i) {
				auto result = DotProductExperiment<InputType, ProductType, AccumulationType, OutputType>(data[0], data[i]);
				std::cout << "custom dot product : " << to_binary(result) << " : " << result << '\n';
			}
		}
} }

// generate a set of N vectors of length L in double as reference
void GenerateRandomVectors(unsigned N, unsigned L, std::vector<sw::universal::blas::vector<double>>& data) {
	using namespace sw::universal;
	blas::vector<double> reference_data(L);
	data.resize(N);
	double mean{ 0.0 }, stddev{ 1.0 };
	for (unsigned i = 0; i < N; ++i) {
		data[i].resize(L);
		blas::gaussian_random(data[i], mean, stddev);
	}
}

template<typename InputType>
void ConvertToInputType(const std::vector<sw::universal::blas::vector<double>>& data, std::vector<sw::universal::blas::vector<InputType>>& idata) {
	size_t N = size(data);
	size_t L = size(data[0]);
	idata.resize(N);
	for (size_t i = 0; i < size(data); ++i) {
		idata[i].resize(L);
		idata[i]  = data[i];
	}
}

template<typename DataType>
void PrintRandomVectors(const std::string& header, const std::vector<sw::universal::blas::vector<DataType>>& data) {
	std::cout << "\n>>>>>>>  " << header << "  <<<<<<<\n";
	for (auto e : data) {
		std::cout << e << '\n';
	}
}


/// <summary>
/// generate the reference dot products
/// </summary>
/// <param name="data">input vectors</param>
/// <param name="dots">output dot product results</param>
void GenerateReferenceDotProducts(const std::vector<sw::universal::blas::vector<double>>& data, std::vector<double>& dots) {
	using namespace sw::universal;
	size_t N = size(data);
	dots.resize(N);
	for (size_t i = 0; i < N; ++i) {
		auto result = DotProductExperiment<double, double, double, double>(data[0], data[i]);
		std::cout << "reference dot product : " << to_binary(result) << " : " << result << '\n';
	}
}



void GenerateParetoSamples(const std::vector<sw::universal::blas::vector<double>>& data) {
	using namespace sw::universal;

	// InputTypes
//	using fp4e3m0_ff = cfloat<4, 3, uint8_t, false, false, false>; // not supported by cfloat<>
//	using fp4e3m0_tt = cfloat<4, 3, uint8_t, true, true, false>; // not supported by cfloat<>
	using fp4e2m1_ff = cfloat<4, 2, uint8_t, false, false, false>;
	using fp6e3m2_ff = cfloat<6, 3, uint8_t, false, false, false>;
	using fp6e4m1_ff = cfloat<6, 4, uint8_t, false, false, false>;
	using fp8e4m3_ff = cfloat<8, 4, uint8_t, false, false, false>;
	using fp8e5m2_ff = cfloat<8, 5, uint8_t, false, false, false>;
	using fp10e5m4_ff = cfloat<10, 5, uint8_t, false, false, false>;
	using fp10e6m3_ff = cfloat<10, 6, uint8_t, false, false, false>;
	using fp12e5m6_ff = cfloat<12, 5, uint8_t, false, false, false>;
	using fp12e6m5_ff = cfloat<12, 6, uint8_t, false, false, false>;
	using fp12e7m4_ff = cfloat<12, 7, uint8_t, false, false, false>;
	using fp16e5ms10_ff = cfloat<16, 5, uint8_t, false, false, false>;
	using fp16e8ms7_ff = cfloat<16, 8, uint8_t, false, false, false>;
	using fp16e9ms6_ff = cfloat<16, 9, uint8_t, false, false, false>;
	// subnormal and supernormal enabled
	using fp4e2m1_tt = cfloat<4, 2, uint8_t, true, true, false>;
	using fp6e3m2_tt = cfloat<6, 3, uint8_t, true, true, false>;
	using fp6e4m1_tt = cfloat<6, 4, uint8_t, true, true, false>;
	using fp8e4m3_tt = cfloat<8, 4, uint8_t, true, true, false>;
	using fp8e5m2_tt = cfloat<8, 5, uint8_t, true, true, false>;
	using fp10e5m4_tt = cfloat<10, 5, uint8_t, true, true, false>;
	using fp10e6m3_tt = cfloat<10, 6, uint8_t, true, true, false>;
	using fp12e5m6_tt = cfloat<12, 5, uint8_t, true, true, false>;
	using fp12e6m5_tt = cfloat<12, 6, uint8_t, true, true, false>;
	using fp12e7m4_tt = cfloat<12, 7, uint8_t, true, true, false>;
	using fp16e5m10_tt = cfloat<16, 5, uint8_t, true, true, false>;
	using fp16e8m7_tt = cfloat<16, 8, uint8_t, true, true, false>;
	using fp16e9m6_tt = cfloat<16, 9, uint8_t, true, true, false>;

	// ProductTypes
	using fp7e3m3_ff = cfloat<7, 3, uint8_t, false, false, false>;
	using fp9e4m4_ff = cfloat<9, 4, uint8_t, false, false, false>;
	using fp9e6m2_ff = cfloat<9, 6, uint8_t, false, false, false>;
	// subnormal and supernormal enabled
	using fp7e3m3_tt = cfloat<7, 3, uint8_t, true, true, false>;
	using fp9e4m4_tt = cfloat<9, 4, uint8_t, true, true, false>;
	using fp9e6m2_tt = cfloat<9, 6, uint8_t, true, true, false>;

//	DotProductExperiment<InputType, ProductType, AccumulationType, OutputType>;
	size_t N = size(data);
	PrintRandomVectors("Reference data set", data);
	std::vector<double> referenceDots(N);
	GenerateReferenceDotProducts(data, referenceDots);

	std::vector < blas::vector<fp8e4m3_tt> > idata;
	ConvertToInputType(data, idata);
	PrintRandomVectors("InputType data set", idata);

	std::vector< fp8e4m3_tt > dots;
	GenerateSpecializedDotProducts< fp8e4m3_tt, fp8e4m3_tt, fp8e4m3_tt, fp8e4m3_tt >(idata, dots);

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

void print_cmdline(int argc, char** argv) {
	std::cout << "cmd: ";
	for (int i = 0; i < argc; ++i) {
		std::cout << argv[i] << ' ';
	}
	std::cout << '\n';
}

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	print_cmdline(argc, argv);

	std::streamsize prec = std::cout.precision();
	std::cout << std::setprecision(3);
	
	std::vector<blas::vector<double>> data;
	GenerateRandomVectors(5, 5, data);
	
	GenerateParetoSamples(data);

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
