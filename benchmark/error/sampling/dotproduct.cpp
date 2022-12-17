// dotproduct.cpp: error measurement of the approximation of a number system encoding on dot products
//
// Copyright (C) 2022-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// enable the following define to show the intermediate steps in the fused-dot product
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/integer/integer.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/blas/blas.hpp>

template<typename Scalar, bool verbose = false>
void SampleError(sw::universal::blas::vector<double>& reals) {
	std::cout << "\nScalar type : " << typeid(Scalar).name() << '\n';
	using namespace sw::universal; 

	auto nrSamples = size(reals);
	blas::vector<Scalar> samples(nrSamples);
	samples = reals;

	double avg{ 0 };
	constexpr unsigned COLWIDTH = 15;
	for (unsigned i = 0; i < nrSamples; ++i) {
		double real = reals[i];
		double sample = double(samples[i]);
		if (sample == 0) sample = real;
		double sampleError = log(real / sample);
		if constexpr (verbose) std::cout << std::setw(4) << i << std::setw(10) << real << std::setw(COLWIDTH) << sample << std::setw(COLWIDTH) << (real/sample) << std::setw(COLWIDTH) << sampleError << '\n';
		avg += sampleError;
	}
	avg /= nrSamples;
	std::cout << "Average sampling error : " << avg << '\n';
}

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	auto reals = sw::universal::blas::gaussian_random_vector<double>(10, 0.0, 32.0);

	constexpr bool Verbose = true;
	SampleError< integer<8>, Verbose >(reals);
	SampleError< fixpnt<16, 8>, Verbose >(reals);
	SampleError< fixpnt<12, 6>, Verbose >(reals);
	SampleError< fixpnt< 8, 4>, Verbose >(reals);
	SampleError< float >(reals);
	SampleError< single >(reals);
	SampleError< half >(reals);
	SampleError< cfloat<8, 3> >(reals);
	SampleError< cfloat<8, 4> >(reals);
	SampleError< posit<16, 2> >(reals);
	SampleError< posit< 8, 2> >(reals);
	SampleError< lns<8, 3> >(reals);
	SampleError< lns<8, 4> >(reals);
	SampleError< lns<8, 5> >(reals);

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
