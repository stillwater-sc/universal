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
		OutputType dot(const blas::vector<InputType>& a, const blas::vector<InputType>& b) {
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
		void GenerateDotProducts(const std::vector<sw::universal::blas::vector<InputType>>& data, std::vector<OutputType>& dots) {
			using namespace sw::universal;
			size_t N = size(data);
			dots.resize(N);
			for (size_t i = 0; i < N; ++i) {
				auto result = dot<InputType, ProductType, AccumulationType, OutputType>(data[0], data[i]);
				if (N < 10) std::cout << "custom dot product : " << to_binary(result) << " : " << result << '\n';
				dots[i] = result;
			}
		}

		// generate a test set of N vectors of length L in double as reference
		void GenerateTestVectors(unsigned N, unsigned L, std::vector<sw::universal::blas::vector<double>>& data, double d) {
			using namespace sw::universal;
			blas::vector<double> reference_data(L);
			data.resize(N);
			for (unsigned i = 0; i < N; ++i) {
				data[i].resize(L);
				data[i] = d;
			}
		}

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
				idata[i] = data[i];
			}
		}

		template<typename Scalar>
		void PrintStdVector(const std::string& header, const std::vector<Scalar>& vec) {
			std::cout << "\n>>>>>>>  " << header << "  <<<<<<<\n";
			for (auto e : vec) {
				std::cout << e << '\n';
			}
		}

		template<typename DataType>
		void PrintDataSet(const std::string& header, const std::vector<sw::universal::blas::vector<DataType>>& data) {
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
				auto result = dot<double, double, double, double>(data[0], data[i]);
				if (N < 10) std::cout << "reference dot product : " << to_binary(result) << " : " << result << '\n';
				dots[i] = result;
			}
		}

		/// <summary>
		/// Given two values, u, and v, calculate the relative error between u and v
		/// </summary>
		/// <param name="u"></param>
 		/// <param name="v"></param>
		/// <returns>relative error half of the difference ln(v) - ln(u)</returns>
		double relativeError(double u, double v) {
			using std::log;
			using std::abs;
			double relativeErr = 0.5* (log(abs(v)) - log(abs(u)));
			return relativeErr;
		}

		template<typename InputType, typename ProductType, typename AccumulationType, typename OutputType>
		void QuantizationVsAccuracy(const std::vector<sw::universal::blas::vector<double>>& data, bool reportTypeRanges = false) {
			using namespace sw::universal;
			constexpr bool bCSV = true;
			if (reportTypeRanges) {
				std::cout << "input arithmetic type         : " << symmetry_range<InputType>() << '\n';
				std::cout << "product arithmetic type       : " << symmetry_range<ProductType>() << '\n';
				std::cout << "accumulation arithmetic type  : " << symmetry_range<AccumulationType>() << '\n';
				std::cout << "output arithmetic type        : " << symmetry_range<OutputType>() << '\n';
			}
			unsigned faEquivalency{ 0 };
			{
				if constexpr (is_fixpnt<ProductType> == true) {
					constexpr unsigned nbits = ProductType::nbits;
					faEquivalency += (nbits + 1) * (nbits + 1);
				}
				else {
					if constexpr (std::is_floating_point<ProductType>::value == true) {
						constexpr unsigned fbits = ieee754_parameter<ProductType>::fbits;
						faEquivalency += (fbits + 1) * (fbits + 1);
						constexpr unsigned ebits = ieee754_parameter<ProductType>::ebits;
						faEquivalency += ebits + 1;
					}
					else {
						constexpr unsigned fbits = ProductType::fbits;
						faEquivalency += (fbits + 1) * (fbits + 1);
						constexpr unsigned es = ProductType::es;
						faEquivalency += es + 1;
					}
				}
			}
			{
				if constexpr (std::is_floating_point<AccumulationType>::value == true) {
					faEquivalency += sizeof(AccumulationType) * 8;
				}
				else {
					constexpr unsigned nbits = AccumulationType::nbits;
					// constexpr unsigned fbits = AccumulationType::fbits;
					faEquivalency += nbits + 1;
				}
			}
			if constexpr (bCSV) {
				std::cout << type_tag(InputType()) << ", ";
			}
			else {
				std::cout << type_tag(InputType()) << '\n';
			}

			size_t N = size(data);
			if (N < 10) PrintDataSet("Reference data set", data);
			std::vector<double> referenceDots(N);
			GenerateReferenceDotProducts(data, referenceDots);
			if (N < 10) PrintStdVector("reference dots ", referenceDots);

			std::vector < blas::vector<InputType> > idata;
			ConvertToInputType(data, idata);
			if (N < 10) PrintDataSet("InputType data set", idata);

			std::vector< OutputType > dots;
			GenerateDotProducts< InputType, ProductType, AccumulationType, OutputType >(idata, dots);

			// we now have N samples on which we can calculate a relative error.
			std::vector<double> errors(N);
			for (size_t i = 0; i < N; ++i) {
				double u{ referenceDots[i] };
				double v{ dots[i] };
				errors[i] = relativeError(u, v);
			}

			if (N < 10) {
				constexpr unsigned WIDTH = 10;
				for (size_t i = 0; i < N; ++i) {
					std::cout << std::setw(WIDTH) << dots[i] << std::setw(WIDTH) << referenceDots[i] << std::setw(WIDTH) << errors[i] << '\n';
				}
			}
			auto stats = blas::summaryStatistics(errors);

			if constexpr (bCSV) {
				std::cout << stats.mean << ", " << faEquivalency << ", " << stats.stddev << '\n';
			}
			else {
				std::cout << stats << '\n';
			}

		}
} }


// Generate an experiment with single type FMA but progressively narrower floating-point
void GenerateFloatingPointSamples(const std::vector<sw::universal::blas::vector<double>>& data) {
	using namespace sw::universal;

	// InputTypes
	using fp12_tf = cfloat<12, 5, uint16_t, true, false, false>;
	using fp11_tf = cfloat<11, 5, uint16_t, true, false, false>;
	using fp10_tf = cfloat<10, 5, uint16_t, true, false, false>;
	using fp9_tf  = cfloat< 9, 5, uint16_t, true, false, false>;
	using fp8e4_tf  = cfloat<8, 4, uint8_t, true, false, false>;
	using fp8_tf = cfloat<8, 5, uint8_t, true, false, false>;
	using fp7_tf  = cfloat<7, 5, uint8_t, true, false, false>;
	using fp6_tf  = cfloat<6, 4, uint8_t, true, false, false>;
	using fp5_tf  = cfloat<5, 3, uint8_t, true, false, false>;
	using fp4_tf  = cfloat<4, 2, uint8_t, true, false, false>;
	QuantizationVsAccuracy< single, single, single, single >(data);
	QuantizationVsAccuracy< bfloat_t, bfloat_t, bfloat_t, bfloat_t >(data);
	QuantizationVsAccuracy< half, half, half, half >(data);
	QuantizationVsAccuracy< fp12_tf, fp12_tf, fp12_tf, fp12_tf >(data);
	QuantizationVsAccuracy< fp11_tf, fp11_tf, fp11_tf, fp11_tf >(data);
	QuantizationVsAccuracy< fp10_tf, fp10_tf, fp10_tf, fp10_tf >(data);
	QuantizationVsAccuracy< fp9_tf, fp9_tf, fp9_tf, fp9_tf >(data);
	QuantizationVsAccuracy< fp8e4_tf, fp8e4_tf, fp8e4_tf, fp8e4_tf >(data);
	QuantizationVsAccuracy< fp8_tf, fp8_tf, fp8_tf, fp8_tf >(data);
	QuantizationVsAccuracy< fp7_tf, fp7_tf, fp7_tf, fp7_tf >(data);
	QuantizationVsAccuracy< fp6_tf, fp6_tf, fp6_tf, fp6_tf >(data);
	QuantizationVsAccuracy< fp5_tf, fp5_tf, fp5_tf, fp5_tf >(data);
	QuantizationVsAccuracy< fp4_tf, fp4_tf, fp4_tf, fp4_tf >(data);
}

void GenerateSmallFloatingPointSamples(const std::vector<sw::universal::blas::vector<double>>& data) {
	using namespace sw::universal;

	// InputTypes

	using fp9_tf = cfloat< 9, 5, uint16_t, true, false, false>;
	using fp8e3_tf = cfloat<8, 3, uint8_t, true, false, false>;
	using fp8e4_tf = cfloat<8, 4, uint8_t, true, false, false>;
	using fp8e5_tf = cfloat<8, 5, uint8_t, true, false, false>;
	using fp7_tf = cfloat<7, 5, uint8_t, true, false, false>;

	QuantizationVsAccuracy< single, single, single, single >(data);
	QuantizationVsAccuracy< fp9_tf, fp9_tf, float, fp9_tf >(data);
	QuantizationVsAccuracy< fp8e3_tf, fp8e3_tf, float, fp8e3_tf >(data);
	QuantizationVsAccuracy< fp8e3_tf, half, float, fp8e3_tf >(data);
	QuantizationVsAccuracy< fp8e4_tf, fp8e4_tf, float, fp8e4_tf >(data);
	QuantizationVsAccuracy< fp8e4_tf, half, float, fp8e4_tf >(data);
	QuantizationVsAccuracy< fp8e5_tf, fp8e5_tf, float, fp8e5_tf >(data);
	QuantizationVsAccuracy< fp7_tf, fp7_tf, float, fp7_tf >(data);

}

void GenerateSmallFixedPointSamples(const std::vector<sw::universal::blas::vector<double>>& data) {
	using namespace sw::universal;

	// InputTypes

	using fp9r2_tf = fixpnt< 9, 2, Saturate, uint16_t>;
	using fp8r2_tf = fixpnt< 8, 2, Saturate, uint8_t>;
	using fp8r3_tf = fixpnt< 8, 3, Saturate, uint8_t>;
	using fp8r4_tf = fixpnt< 8, 4, Saturate, uint8_t>;
	using fp8r5_tf = fixpnt< 8, 5, Saturate, uint8_t>;
	using fp7r4_tf = fixpnt< 7, 4, Saturate, uint8_t>;

	QuantizationVsAccuracy< single, single, single, single >(data);
	QuantizationVsAccuracy< fp9r2_tf, fp9r2_tf, float, fp9r2_tf >(data);
	QuantizationVsAccuracy< fp8r3_tf, fp8r3_tf, float, fp8r3_tf >(data);
	QuantizationVsAccuracy< fp8r3_tf, float, float, fp8r3_tf >(data);
	QuantizationVsAccuracy< fp8r4_tf, fp8r4_tf, float, fp8r4_tf >(data);
	QuantizationVsAccuracy< fp8r4_tf, float, float, fp8r4_tf >(data);
	QuantizationVsAccuracy< fp8r5_tf, fp8r5_tf, float, fp8r5_tf >(data);
	QuantizationVsAccuracy< fp7r4_tf, fp7r4_tf, float, fp7r4_tf >(data);

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

	QuantizationVsAccuracy< fp8e4m3_tt, fp8e4m3_tt, fp8e4m3_tt, fp8e4m3_tt >(data);
	QuantizationVsAccuracy< fp8e4m3_tt, fp8e5m2_tt, fp16e5m10_tt, fp8e4m3_tt >(data);
	QuantizationVsAccuracy< half, half, float, half >(data);
}

void checkRelativeError() {
	double u{ 1.0 }, v{ 1.0 };
	for (unsigned i = 0; i < 10; ++i) {
		std::cout << "v : " << v << " u : " << u << " : relative error : " << sw::universal::relativeError(u, v) << '\n';
		v *= 1.1;
	}
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
	//GenerateRandomVectors(100, 4096, data);
	GenerateRandomVectors(10, 8192, data);
//	GenerateTestVectors(5, 5, data, 0.75);
	
	// GenerateSmallFixedPointSamples(data);
	GenerateSmallFloatingPointSamples(data);
	// GenerateFloatingPointSamples(data);
	// GenerateParetoSamples(data);

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
