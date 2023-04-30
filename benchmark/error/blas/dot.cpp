// dot.cpp: error measurement of the approximation of a number system computing a dot product
//
// Copyright (C) 2022-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/number/integer/integer.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/blas/blas.hpp>

template<typename Scalar>
void TraceProducts(const sw::universal::blas::vector<Scalar>& x, const sw::universal::blas::vector<Scalar>& y) {
	using std::abs;
	auto nrSamples = size(x); 
	Scalar minInput = abs(x[0]);
	Scalar maxInput = minInput;
//	Scalar minOutput = minInput;
//	Scalar maxOutput = maxInput;
	Scalar minProduct = abs(x[0] * y[0]);
	Scalar maxProduct = minProduct;
	for (unsigned i = 1; i < nrSamples; ++i) {
		Scalar input = abs(x[i]);
		if (minInput > input) minInput = input;
		if (maxInput < input) maxInput = input;
		Scalar product = abs(x[i] * y[i]);
		if (minProduct > product) minProduct = product;
		if (maxProduct < product) maxProduct = product;
	}
	std::cout << "input   range = [ " << minInput << ", " << maxInput << "]\n";
	std::cout << "product range = [ " << minProduct << ", " << maxProduct << "]\n";
	std::cout << sw::universal::symmetry_range<Scalar>() << '\n';
	// std::cout << sw::universal::dynamic_range<Scalar>() << '\n'; // not that interesting in this context
	std::cout << sw::universal::minmax_range<Scalar>() << '\n';
}

template<typename Scalar, bool verbose = true>
void DotProductError(const sw::universal::blas::vector<double>& x, double minx, double maxx, const sw::universal::blas::vector<double>& y, double miny, double maxy) {
	using std::log;
	using namespace sw::universal; 
	std::cout << "\nScalar type : " << type_tag(Scalar()) << '\n';

	auto minpos = double(std::numeric_limits<Scalar>::min());
	auto maxpos = double(std::numeric_limits<Scalar>::max());
	auto maxxy = std::max(maxx, maxy);
	auto focus{ 1.0 }, expand{ 1.0 };
	if (maxxy*maxxy > maxpos) { // need to scale the vectors
		double upperbound = sqrt(maxpos);
		focus = upperbound / maxxy;
		expand = maxxy / upperbound;  // is this a more precise calculation than (1.0 / scale)? 
		                              // Yes, but how much more precise? The trouble spots are at the extremes of the range

		// check for underflow
		auto smallestScaledx = focus * minx;
		auto smallestScaledy = focus * miny;
		auto smallestScaledElement = std::min(smallestScaledx, smallestScaledy);
		if (focus * minx < minpos || focus * miny < minpos) {
			std::cout << "Scaling is causing underflow: " << smallestScaledElement << " < " << minpos << '\n';
		}
	}
	auto nrSamples = size(x);
	blas::vector<Scalar> xx(nrSamples);
	xx = focus * x;
	blas::vector<Scalar> yy(nrSamples);
	yy = focus * y;

	double real = x * y;
	double sample = double(xx * yy) * expand;
	TraceProducts(xx, yy);
	double dotError = log(real / sample);
	constexpr unsigned COLWIDTH = 15;
	if constexpr (verbose) {
		std::cout << std::setw(10) << "Reference" << std::setw(COLWIDTH) << "Target Type" << std::setw(COLWIDTH) << "Ratio" << std::setw(COLWIDTH) << "ln(ratio)" << '\n';
		std::cout << std::setw(10) << real << std::setw(COLWIDTH) << sample << std::setw(COLWIDTH) << (real / sample) << std::setw(COLWIDTH) << dotError << '\n';
	}
	else std::cout << "DOT product sampling error : " << dotError << '\n';
}

void TestSampleError(unsigned N = 10000, double mean = 0.0, double stddev = 2.0) {
	using namespace sw::universal;

	auto x = sw::universal::blas::gaussian_random_vector<double>(N, mean, stddev);
	auto y = sw::universal::blas::gaussian_random_vector<double>(N, mean, stddev);

	double minx = x[blas::amin(N, x, 1)];
	double maxx = x[blas::amax(N, x, 1)];
	double miny = y[blas::amin(N, y, 1)];
	double maxy = y[blas::amax(N, y, 1)];
	constexpr bool Verbose = true;
	DotProductError< duble >(x, minx, maxx, y, miny, maxy);
	DotProductError< single >(x, minx, maxx, y, miny, maxy);
	DotProductError< half >(x, minx, maxx, y, miny, maxy);
	DotProductError< cfloat<8, 2, uint8_t, true, false> >(x, minx, maxx, y, miny, maxy);
	DotProductError< cfloat<8, 3, uint8_t, true, false> >(x, minx, maxx, y, miny, maxy);
	DotProductError< cfloat<8, 4, uint8_t, true, false> >(x, minx, maxx, y, miny, maxy);
	DotProductError< cfloat<8, 5, uint8_t, true, false> >(x, minx, maxx, y, miny, maxy);
	DotProductError< cfloat<8, 2, uint8_t, true, true> >(x, minx, maxx, y, miny, maxy);
	DotProductError< cfloat<8, 3, uint8_t, true, true> >(x, minx, maxx, y, miny, maxy);
	DotProductError< cfloat<8, 4, uint8_t, true, true> >(x, minx, maxx, y, miny, maxy);
	DotProductError< cfloat<8, 5, uint8_t, true, true> >(x, minx, maxx, y, miny, maxy);
}

void SampleError(unsigned N = 10000, double mean = 0.0, double stddev = 2.0) {
	using namespace sw::universal;

	auto x = sw::universal::blas::gaussian_random_vector<double>(N, mean, stddev);
	auto y = sw::universal::blas::gaussian_random_vector<double>(N, mean, stddev);

	double minx = x[blas::amin(N, x, 1)];
	double maxx = x[blas::amax(N, x, 1)];
	double miny = y[blas::amin(N, y, 1)];
	double maxy = y[blas::amax(N, y, 1)];
	constexpr bool Verbose = true;
	DotProductError< double >(x, minx, maxx, y, miny, maxy);
	DotProductError< float >(x, minx, maxx, y, miny, maxy);
	DotProductError< single >(x, minx, maxx, y, miny, maxy);
	DotProductError< half >(x, minx, maxx, y, miny, maxy);
	DotProductError< fixpnt<16, 8>, Verbose >(x, minx, maxx, y, miny, maxy);
	DotProductError< cfloat<8, 2> >(x, minx, maxx, y, miny, maxy);
	DotProductError< cfloat<8, 2, uint8_t, true> >(x, minx, maxx, y, miny, maxy);
	DotProductError< cfloat<8, 3> >(x, minx, maxx, y, miny, maxy);
	DotProductError< cfloat<8, 3, uint8_t, true> >(x, minx, maxx, y, miny, maxy);
	DotProductError< cfloat<8, 4> >(x, minx, maxx, y, miny, maxy);
	DotProductError< cfloat<8, 4, uint8_t, true> >(x, minx, maxx, y, miny, maxy);
	DotProductError< posit<16, 2> >(x, minx, maxx, y, miny, maxy);
	DotProductError< posit< 8, 2> >(x, minx, maxx, y, miny, maxy);
	DotProductError< lns<8, 3> >(x, minx, maxx, y, miny, maxy);
	DotProductError< lns<8, 4> >(x, minx, maxx, y, miny, maxy);
	DotProductError< lns<8, 5> >(x, minx, maxx, y, miny, maxy);
	DotProductError< lns<8, 5> >(x, minx, maxx, y, miny, maxy);
	DotProductError< integer<8> >(x, minx, maxx, y, miny, maxy);
}

namespace sw {
	namespace universal {
		// data normalization

		// MinMaxScaler rescales the elements of a vector from their original 
		// range [min, max] to a new range [lb, ub]
		template<typename Scalar>
		blas::vector<Scalar> minmaxScaler(const blas::vector<Scalar>& v, Scalar lb = 0, Scalar ub = 1) {
			if (lb < ub) {
				std::cerr << "target range is inconsistent\n";
			}
			auto min = abs(v[blas::amin(v.size(), v)]);
			auto max = abs(v[blas::amax(v.size(), v)]);
			auto mapto = (ub - lb) / (max - min);
			blas::vector<Scalar> t;
			for (auto e : v) {
				t.push_back( (e - min) * mapto );
			}
			return t;
		}

	}
}

/*
 * When we want to take arbitrary vectors and want to faithfully calculate a 
 * dot product using lower precision types, we need to 'squeeze' the values
 * of the original vector such that the computational dynamics of the dot product
 * can be emulated. 
 * 
 * When you think about very constrained types like 8-bit floating-point formats
 * the risk of overflow and underflow of the products is the first problem
 * to solve. Secondly, for long vectors overflow and catastrophic cancellation
 * are also risks.
 * 
 * Assume we have a vector x like this
 * 
 *                  *
 *                 ***
 *             ***********
 *       *********************** 
 * -----------------+--------------------
 * |     ^          0^         ^        |
 * |  minneg        min       max       |
 * minneg                             maxpos
 *          |-------------|
 *       minneg        maxpos of target number system
 * 
 * we need to 'squeeze' 
 *    max to sqrt(maxpos) of target system
 *    min to sqrt(minpos) of target system
 * which ever is more constraining;
 * 
 * maxScale = sqrt(maxpos) / max
 * minScale = sqrt(minpos) / min
 *                  
 */

template<typename Real>
std::pair<Real, Real> minmax(const sw::universal::blas::vector<Real>& v) {
	auto minValue = abs(v[sw::universal::blas::amin(v.size(), v)]);
	auto maxValue = abs(v[sw::universal::blas::amax(v.size(), v)]);
	std::cout << "minValue  : " << minValue << '\n';
	std::cout << "maxValue  : " << maxValue << '\n';
	return std::pair(minValue, maxValue);
}

template<typename Target>
sw::universal::blas::vector<Target> squeeze(const sw::universal::blas::vector<double>& v) {
	auto minpos = double(std::numeric_limits<Target>::min());
	auto maxpos = double(std::numeric_limits<Target>::max());

	auto vminmax = minmax(v);
	auto minValue = vminmax.first;
	auto maxValue = vminmax.second;
	
	auto sqrtMinpos = sqrt(minpos);
	auto sqrtMaxpos = sqrt(maxpos);

	auto minScale = sqrtMinpos / minValue;
	auto maxScale = sqrtMaxpos / maxValue;

	std::cout << "minScale  : " << minScale << '\n';
	std::cout << "maxScale  : " << maxScale << '\n';

	sw::universal::blas::vector<Target> t(v.size());
	if (abs(maxValue) < sqrtMaxpos) maxScale = 1.0; // no need to scale
	t = maxScale * v;

	return t;
}

int main()
try {
	using namespace sw::universal;

	unsigned N{ 14 };
	double mean{ 0.0 }, stddev{ 1.0 };

	auto dv = sw::universal::blas::gaussian_random_vector<double>(N, mean, stddev);
	auto dminmax = minmax(dv);
	auto dminmaxScaled = minmaxScaler(dv);

	auto sv = squeeze<float>(dv);
	auto sminmax = minmax(sv);

	auto hv = squeeze<half>(dv);
	auto hminmax = minmax(hv);

	auto qv = squeeze<quarter>(dv);
	auto qminmax = minmax(qv);

	if (N < 15) {
		std::cout << dv << '\n';
		std::cout << dminmaxScaled << '\n';
		std::cout << sv << '\n';
		std::cout << hv << '\n';
		std::cout << qv << '\n';
	}

	//TestSampleError(N, 0.0, 1.0);
	return 0;
	SampleError(N, 0.0, 1.0);
	SampleError(N, 0.0, 2.0);
	SampleError(N, 0.0, 5.0);

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
