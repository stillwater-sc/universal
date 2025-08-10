// dot.cpp: error measurement of the approximation of a number system computing a dot product
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/number/integer/integer.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>

// Stillwater BLAS library
#include <blas/blas.hpp>

using namespace sw::numeric::containers;
using namespace sw::blas;

template<typename Scalar>
void TraceProducts(const vector<Scalar>& x, const vector<Scalar>& y) {
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
	std::string infoStr = sw::universal::symmetry_range<Scalar>();
	std::cout << infoStr << '\n';
	infoStr = sw::universal::minmax_range<Scalar>();
	std::cout << infoStr << '\n';
}

template<typename Scalar, bool verbose = true>
void DotProductError(const vector<double>& x, double minx, double maxx, const vector<double>& y, double miny, double maxy) {
	using std::log;
	using namespace sw::universal; 
	std::cout << "\nScalar type : " << type_tag(Scalar()) << '\n';

	auto minpos = static_cast<double>(std::numeric_limits<Scalar>::min());
	auto maxpos = static_cast<double>(std::numeric_limits<Scalar>::max());
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
	vector<Scalar> xx(nrSamples);
	xx = focus * x;
	vector<Scalar> yy(nrSamples);
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

	auto x = gaussian_random_vector<double>(N, mean, stddev);
	auto y = gaussian_random_vector<double>(N, mean, stddev);

	double minx = x[amin(N, x, 1)];
	double maxx = x[amax(N, x, 1)];
	double miny = y[amin(N, y, 1)];
	double maxy = y[amax(N, y, 1)];
	constexpr bool verbose = true;
	DotProductError< duble, verbose >(x, minx, maxx, y, miny, maxy);
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

	auto x = gaussian_random_vector<double>(N, mean, stddev);
	auto y = gaussian_random_vector<double>(N, mean, stddev);

	double minx = x[amin(N, x, 1)];
	double maxx = x[amax(N, x, 1)];
	double miny = y[amin(N, y, 1)];
	double maxy = y[amax(N, y, 1)];
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
 */


int main()
try {
	using namespace sw::universal;

	unsigned N{ 10000 };
	double mean{ 0.0 }, stddev{ 1.0 };

	TestSampleError(N, mean, stddev);

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
