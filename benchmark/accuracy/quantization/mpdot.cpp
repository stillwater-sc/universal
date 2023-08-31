// mpfma.cpp: accuracy/quantization measurement of mixed-precision dot products
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/dbns/dbns.hpp>
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

			OutputType result = OutputType(sum(acc));

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

		unsigned IntegerAdder(unsigned nbits) {
			using std::log2;
			return (nbits * static_cast<unsigned>(log2(nbits)));
		}
		unsigned IntegerMultiplier(unsigned nbits) {
			return (nbits * nbits);
		}
		unsigned FloatingPointMultiplier(unsigned ebits, unsigned fbits) {
			return IntegerMultiplier(fbits + 1) + IntegerAdder(ebits);
		}
		unsigned FloatingPointAdder(unsigned ebits, unsigned fbits) {
			return IntegerAdder(fbits + 1) + IntegerAdder(ebits);
		}

		template<typename ProductType>
		unsigned MultiplierCircuitComplexity() {
			unsigned faEquivalency{ 0 };
			if constexpr (std::is_floating_point<ProductType>::value == true) {
				//std::cerr << "ProductType is floating-point\n";
				constexpr unsigned ebits = ieee754_parameter<ProductType>::ebits;
				constexpr unsigned fbits = ieee754_parameter<ProductType>::fbits;
				faEquivalency += FloatingPointMultiplier(ebits, fbits);
			}
			else if constexpr (is_cfloat<ProductType> == true) {
				//std::cerr << "ProductType is cfloat\n";
				constexpr unsigned ebits = ProductType::es;
				constexpr unsigned fbits = ProductType::fbits;
				faEquivalency += FloatingPointMultiplier(ebits, fbits);
			}
			else if constexpr (is_integer<ProductType> == true) {
				//std::cerr << "ProductType is integer<>\n";
				constexpr unsigned nbits = ProductType::nbits;
				faEquivalency += IntegerMultiplier(nbits);
			}
			else if constexpr (is_fixpnt<ProductType> == true) {
				//std::cerr << "ProductType is fixpnt\n";
				constexpr unsigned nbits = ProductType::nbits;
				faEquivalency += IntegerMultiplier(nbits);
			}
			else if constexpr (is_lns<ProductType> == true) {
				//std::cerr << "ProductType is lns\n";
				constexpr unsigned nbits = ProductType::nbits - 1u;
				faEquivalency += IntegerAdder(nbits);
			}
			else if constexpr (is_dbns<ProductType> == true) {
				//std::cerr << "ProductType is dbns\n";
				constexpr unsigned nbits = ProductType::nbits - 1u;
				faEquivalency += IntegerAdder(nbits);
			}
			else {
				std::cerr << "ProductType :" << type_tag(ProductType()) << " is unsupported\n";
			}
			return faEquivalency;
		}



		template<typename AccumulationType>
		unsigned AccumulatorCircuitComplexity()
		{
			unsigned faEquivalency{ 0 };
			if constexpr (std::is_floating_point<AccumulationType>::value == true) {
				//std::cerr << "AccumulationType is floating-point\n";
				unsigned fbits = ieee754_parameter<AccumulationType>::fbits + 1;
				unsigned ebits = ieee754_parameter<AccumulationType>::ebits + 1;
				faEquivalency += FloatingPointAdder(ebits, fbits);
			}
			else if constexpr (is_cfloat<AccumulationType> == true) {
				//std::cerr << "AccumulationType is cfloat\n";
				constexpr unsigned ebits = AccumulationType::es + 1;
				constexpr unsigned fbits = AccumulationType::fbits + 1;
				faEquivalency += FloatingPointAdder(ebits, fbits);
			}
			else if constexpr (is_integer<AccumulationType> == true) {
				//std::cerr << "AccumulationType is integer<>\n";
				constexpr unsigned nbits = AccumulationType::nbits;
				faEquivalency += IntegerAdder(nbits);
			}
			else if constexpr (is_fixpnt<AccumulationType> == true) {
				//std::cerr << "AccumulationType is fixpnt\n";
				constexpr unsigned nbits = AccumulationType::nbits;
				faEquivalency += IntegerAdder(nbits);
			}
			else if constexpr (is_lns<AccumulationType> == true) {
				//std::cerr << "AccumulationType is lns\n";
				constexpr unsigned nbits = AccumulationType::nbits;
				faEquivalency += 4*IntegerAdder(nbits - 1);
			}
			else if constexpr (is_dbns<AccumulationType> == true) {
				//std::cerr << "AccumulationType is dbns\n";
				constexpr unsigned nbits = AccumulationType::nbits;
				faEquivalency += 2 * IntegerAdder(nbits - 1);
			}
			else {
				std::cerr << "AccumulationType :" << type_tag(AccumulationType()) << " is unsupported\n";
			}
			return faEquivalency;
		}


		template<typename InputType, typename ProductType, typename AccumulationType, typename OutputType>
		void QuantizationVsAccuracy(const std::string& tag, const std::vector<sw::universal::blas::vector<double>>& data, const std::vector<double>& referenceDots, bool reportTypeRanges = false) {
			using namespace sw::universal;
			constexpr bool bCSV = true;
			if (reportTypeRanges) {
				std::string info = symmetry_range<InputType>();
				std::cout << "input arithmetic type         : " << info << '\n';
				info = symmetry_range<ProductType>();
				std::cout << "product arithmetic type       : " << info << '\n';
				info = symmetry_range<AccumulationType>();
				std::cout << "accumulation arithmetic type  : " << info << '\n';
				info = symmetry_range<OutputType>();
				std::cout << "output arithmetic type        : " << info << '\n';
			}
			unsigned faEquivalency = MultiplierCircuitComplexity<ProductType>() + AccumulatorCircuitComplexity<AccumulationType>();

			if constexpr (bCSV) {
				std::cout << type_tag(InputType()) << ", " << tag << ", ";
			}
			else {
				std::cout << type_tag(InputType()) << ' ' << tag << '\n';
			}

			size_t N = size(data);

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
				std::cout << stats.stddev << ", " << faEquivalency << ", " << stats.mean ;
				for (int i = 0; i < 5; ++i) {
					std::cout << ", " << stats.quartiles[i];
				}
				std::cout << '\n';
			}
			else {
				std::cout << stats << '\n';
			}

		}
} }


// Generate an experiment with single type FMA but progressively narrower floating-point
void GenerateFloatingPointSamples(const std::vector<sw::universal::blas::vector<double>>& data, const std::vector<double>& referenceDots) {
	using namespace sw::universal;

	// InputTypes
	using fp12_tf = cfloat<12, 5, uint16_t, true, false, false>;
	using fp11_tf = cfloat<11, 5, uint16_t, true, false, false>;
	using fp10_tf = cfloat<10, 5, uint16_t, true, false, false>;
	using fp9_tf  = cfloat< 9, 5, uint16_t, true, false, false>;
	using fp8_tf  = cfloat<8, 5, uint8_t, true, false, false>;
	using fp7_tf  = cfloat<7, 5, uint8_t, true, false, false>;
	using fp6_tf  = cfloat<6, 4, uint8_t, true, false, false>;
	using fp5_tf  = cfloat<5, 3, uint8_t, true, false, false>;
	using fp4_tf  = cfloat<4, 2, uint8_t, true, false, false>;
	QuantizationVsAccuracy< single, single, single, single >("fp32_ieee", data, referenceDots);
	QuantizationVsAccuracy< bfloat_t, bfloat_t, bfloat_t, bfloat_t >("bfloat16_ieee", data, referenceDots);
	QuantizationVsAccuracy< half, half, half, half >("fp16e5_ieee", data, referenceDots);
	QuantizationVsAccuracy< fp12_tf, fp12_tf, fp12_tf, fp12_tf >("fp12e5_ieee", data, referenceDots);
	QuantizationVsAccuracy< fp11_tf, fp11_tf, fp11_tf, fp11_tf >("fp11e5_ieee", data, referenceDots);
	QuantizationVsAccuracy< fp10_tf, fp10_tf, fp10_tf, fp10_tf >("fp10e5_ieee", data, referenceDots);
	QuantizationVsAccuracy< fp9_tf, fp9_tf, fp9_tf, fp9_tf >("fp9e5_ieee", data, referenceDots);
	QuantizationVsAccuracy< fp8_tf, fp8_tf, fp8_tf, fp8_tf >("fp8e5_ieee", data, referenceDots);
	QuantizationVsAccuracy< fp7_tf, fp7_tf, fp7_tf, fp7_tf >("fp7e5_ieee", data, referenceDots);
	QuantizationVsAccuracy< fp6_tf, fp6_tf, fp6_tf, fp6_tf >("fp6e4_ieee", data, referenceDots);
	QuantizationVsAccuracy< fp5_tf, fp5_tf, fp5_tf, fp5_tf >("fp5e3_ieee", data, referenceDots);
	QuantizationVsAccuracy< fp4_tf, fp4_tf, fp4_tf, fp4_tf >("fp4e2_ieee", data, referenceDots);

	using fp8e4_tf = cfloat<8, 4, uint8_t, true, false, false>;
	QuantizationVsAccuracy< fp8e4_tf, fp8e4_tf, fp8e4_tf, fp8e4_tf >("fp8e4_ieee", data, referenceDots);
}


void GenerateSmallFixedPointSamples(const std::vector<sw::universal::blas::vector<double>>& data, const std::vector<double>& referenceDots) {
	using namespace sw::universal;

	// InputTypes

	using fp9r2_tf = fixpnt< 9, 2, Saturate, uint16_t>;
	using fp8r2_tf = fixpnt< 8, 2, Saturate, uint8_t>;
	using fp8r3_tf = fixpnt< 8, 3, Saturate, uint8_t>;
	using fp8r4_tf = fixpnt< 8, 4, Saturate, uint8_t>;
	using fp8r5_tf = fixpnt< 8, 5, Saturate, uint8_t>;
	using fp7r4_tf = fixpnt< 7, 4, Saturate, uint8_t>;

	QuantizationVsAccuracy< single, single, single, single >("fp32_ieee", data, referenceDots);
	QuantizationVsAccuracy< fp9r2_tf, fp9r2_tf, float, fp9r2_tf >("fixpnt9r2_9r2_fp32", data, referenceDots);
	QuantizationVsAccuracy< fp8r3_tf, fp8r3_tf, float, fp8r3_tf >("fixpnt8r3_8r3_fp32", data, referenceDots);
	QuantizationVsAccuracy< fp8r3_tf, float, float, fp8r3_tf >("fixpnt8r3_fp32_fp32", data, referenceDots);
	QuantizationVsAccuracy< fp8r4_tf, fp8r4_tf, float, fp8r4_tf >("fixpnt8r4_8r4_fp32", data, referenceDots);
	QuantizationVsAccuracy< fp8r4_tf, float, float, fp8r4_tf >("fixpnt8r4_fp32_fp32", data, referenceDots);
	QuantizationVsAccuracy< fp8r5_tf, fp8r5_tf, float, fp8r5_tf >("fixpnt8r5_8r5_fp32", data, referenceDots);
	QuantizationVsAccuracy< fp7r4_tf, fp7r4_tf, float, fp7r4_tf >("fixpnt7r4_7r4_fp32", data, referenceDots);

}

void GenerateParetoSamples(const std::vector<sw::universal::blas::vector<double>>& data, const std::vector<double>& referenceDots) {
	using namespace sw::universal;

	using fi8r4   = fixpnt<8, 4, Saturate, uint8_t>;
	using fi16r8  = fixpnt<16, 8, Saturate, uint8_t>;
	using fi32r16 = fixpnt<32, 16, Saturate, uint8_t>;
	QuantizationVsAccuracy< fi8r4, fi16r8, fi32r16, fi8r4 >("fi8r4_16r8_32r16", data, referenceDots);
	using fi32r8 = fixpnt<32, 8, Saturate, uint8_t>;
	QuantizationVsAccuracy< fi8r4, fi16r8, fi32r8, fi8r4 >("fi8r4_16r8_32r8", data, referenceDots);

	using fp8e4sat = cfloat<8, 4, uint8_t, true, true, true>;
	using fp13e5sat = cfloat<13, 5, uint8_t, true, true, true>;
	using fp16e8sat = cfloat<16, 8, uint8_t, true, true, true>;
	QuantizationVsAccuracy< fp8e4sat, fp13e5sat, fp16e8sat, fp8e4sat >("fp8e4_fp13e5_fp16e8sat", data, referenceDots);
	using fp8e4nonsat = cfloat<8, 4, uint8_t, true, true, false>;
	using fp13e5nonsat = cfloat<13, 5, uint8_t, true, true, false>;
	using fp16e8nonsat = cfloat<16, 8, uint8_t, true, true, false>;
	QuantizationVsAccuracy< fp8e4nonsat, fp13e5nonsat, fp16e8nonsat, fp8e4nonsat >("fp8e4_fp13e5_fp16e8nonsat", data, referenceDots);

	using lns8r3 = lns<8, 3, uint8_t>;
	using lns10r4 = lns<10, 4, uint8_t>;
	using lns12r5 = lns<12, 5, uint8_t>;
	QuantizationVsAccuracy< lns8r3, lns10r4, lns12r5, lns8r3 >("lns8r3_lns10r4_lns12r5", data, referenceDots);
}

void GenerateParetoSamples2(const std::vector<sw::universal::blas::vector<double>>& data, const std::vector<double>& referenceDots) {
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

	QuantizationVsAccuracy< fp8e4m3_tt, fp8e4m3_tt, fp8e4m3_tt, fp8e4m3_tt >("fp8e4_tt", data, referenceDots);
	QuantizationVsAccuracy< fp8e4m3_tt, fp8e5m2_tt, fp16e5m10_tt, fp8e4m3_tt >("fp8e4_e5_fp15e5", data, referenceDots);
	QuantizationVsAccuracy< half, half, float, half >("fp16_ieee", data, referenceDots);
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
	
	std::cout << "circuit complexity of single precision accumulator : " << AccumulatorCircuitComplexity<float>() << '\n';
	std::cout << "circuit complexity of single precision accumulator : " << AccumulatorCircuitComplexity<single>() << '\n';
	std::cout << "circuit complexity of 8-bit integer accumulator    : " << AccumulatorCircuitComplexity<integer<8>>() << '\n';
	std::cout << "circuit complexity of 16-bit integer accumulator   : " << AccumulatorCircuitComplexity<integer<16>>() << '\n';
	std::cout << "circuit complexity of 32-bit integer accumulator   : " << AccumulatorCircuitComplexity<integer<32>>() << '\n';
	std::cout << "circuit complexity of 16-bit fixpnt accumulator    : " << AccumulatorCircuitComplexity<fixpnt<16,8>>() << '\n';
	std::cout << "circuit complexity of 8-bit lns accumulator        : " << AccumulatorCircuitComplexity<lns<8, 3>>() << '\n';
	std::cout << "circuit complexity of 8-bit dbns accumulator       : " << AccumulatorCircuitComplexity<dbns<8, 4>>() << '\n';

	std::cout << "circuit complexity of single precision multiplier  : " << MultiplierCircuitComplexity<float>() << '\n';
	std::cout << "circuit complexity of single precision multiplier  : " << MultiplierCircuitComplexity<single>() << '\n';
	std::cout << "circuit complexity of 8-bit integer multiplier     : " << MultiplierCircuitComplexity<integer<8>>() << '\n';
	std::cout << "circuit complexity of 16-bit integer multiplier    : " << MultiplierCircuitComplexity<integer<16>> () << '\n';
	std::cout << "circuit complexity of 32-bit integer multiplier    : " << MultiplierCircuitComplexity<integer<32>>() << '\n';
	std::cout << "circuit complexity of 16-bit fixpnt multiplier     : " << MultiplierCircuitComplexity<fixpnt<16,8>> () << '\n';
	std::cout << "circuit complexity of 8-bit lns multiplier         : " << MultiplierCircuitComplexity<lns<8, 3>>() << '\n';
	std::cout << "circuit complexity of 8-bit dbns multiplier        : " << MultiplierCircuitComplexity<dbns<8, 4>>() << '\n';

	std::vector<blas::vector<double>> data;
	//GenerateRandomVectors(100, 4096, data);
	//GenerateRandomVectors(10, 8192, data);
	GenerateRandomVectors(2, 16, data);
//	GenerateTestVectors(5, 5, data, 0.75);
	size_t N = size(data);
	if (N < 10) PrintDataSet("Reference data set", data);
	std::vector<double> referenceDots(N);

	GenerateReferenceDotProducts(data, referenceDots);
	if (N < 10) PrintStdVector("reference dots ", referenceDots);

	// GenerateSmallFixedPointSamples(data, referenceDots);
	// GenerateFloatingPointSamples(data, referenceDots);
	 GenerateParetoSamples(data, referenceDots);

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
