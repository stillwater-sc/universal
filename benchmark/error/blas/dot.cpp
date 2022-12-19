// dot.cpp: error measurement of the approximation of a number system computing a dot product
//
// Copyright (C) 2022-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// enable the following define to show the intermediate steps in the fused-dot product
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
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
	Scalar minOutput = minInput;
	Scalar maxOutput = maxInput;
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
	std::cout << sw::universal::symmetry<Scalar>() << '\n';
}

template<typename Scalar, bool verbose = false>
void DotProductError(const sw::universal::blas::vector<double>& x, const sw::universal::blas::vector<double>& y) {
	std::cout << "\nScalar type : " << typeid(Scalar).name() << '\n';
	using std::log;
	using namespace sw::universal; 

	auto nrSamples = size(x);
	blas::vector<Scalar> xx(nrSamples);
	xx = x;
	blas::vector<Scalar> yy(nrSamples);
	yy = y;

	double real = x * y;
	double sample = double(xx * yy);
	TraceProducts(xx, yy);
	double dotError = log(real / sample);
	constexpr unsigned COLWIDTH = 15;
	if constexpr (verbose) std::cout << std::setw(10) << real << std::setw(COLWIDTH) << sample << std::setw(COLWIDTH) << (real / sample) << std::setw(COLWIDTH) << dotError << '\n';
	std::cout << "DOT product sampling error : " << dotError << '\n';
}

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	auto x = sw::universal::blas::gaussian_random_vector<double>(10, 0.0, 2.0);
	auto y = sw::universal::blas::gaussian_random_vector<double>(10, 0.0, 2.0);

	constexpr bool Verbose = false;
	DotProductError< double, Verbose >(x, y);
	DotProductError< float, Verbose >(x, y);
	DotProductError< single >(x, y);
	DotProductError< half >(x, y);
	DotProductError< integer<8>, Verbose >(x, y);
	DotProductError< fixpnt<16, 8>, Verbose >(x, y);
	DotProductError< cfloat<8, 3> >(x, y);
	DotProductError< cfloat<8, 4> >(x, y);
	DotProductError< posit<16, 2> >(x, y);
	DotProductError< posit< 8, 2> >(x, y);
	DotProductError< lns<8, 3> >(x, y);
	DotProductError< lns<8, 4> >(x, y);
	DotProductError< lns<8, 5> >(x, y);

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
