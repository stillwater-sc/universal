// sampling.cpp: error measurement of the approximation of a number system sampling real values
//
// Copyright (C) 2022-2022 Stillwater Supercomputing, Inc.
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
#include <universal/blas/blas.hpp>

template<typename Scalar, bool verbose = false>
void SampleError(sw::universal::blas::vector<double>& reals) {
	std::cout << "\nScalar type : " << typeid(Scalar).name() << '\n';
	using namespace sw::universal; 

	auto nrSamples = size(reals);
	blas::vector<Scalar> samples(nrSamples);
	samples = reals;

	double avgError{ 0 }, maxError{ 0 };
	constexpr unsigned COLWIDTH = 15;
	for (unsigned i = 0; i < nrSamples; ++i) {
		double real = reals[i];
		double sample = double(samples[i]);
		if (sample == 0) sample = real;
		double sampleError = log(real / sample);
		if constexpr (verbose) std::cout << std::setw(4) << i << std::setw(10) << real << std::setw(COLWIDTH) << sample << std::setw(COLWIDTH) << (real/sample) << std::setw(COLWIDTH) << sampleError << '\n';
		avgError += sampleError;
		double absError = abs(sampleError);
		if (absError > maxError) maxError = absError;
	}
	avgError /= static_cast<double>(nrSamples);
	std::cout << "Average sampling error : " << avgError << '\n';
	std::cout << "Maximum sampling error : " << maxError << '\n';
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

	auto reals = sw::universal::blas::gaussian_random_vector<double>(10, 0.0, 32.0);

	constexpr bool Verbose = false;
	SampleError< integer<8>, Verbose >(reals);
	SampleError< fixpnt<16, 8, Saturate, std::uint16_t>, Verbose >(reals);
	// with a stddev around 32.0, 5 bits are not sufficient to capture the outliers. The extreme value will saturate and thus NOT correctly calculate the sample difference as it cannot be represented in this number system
	// there for fixpnt<12,6> and fixpnt<8,4> will not work
	// SampleError< fixpnt<12, 6, Saturate, std::uint16_t>, Verbose >(reals); 
	// SampleError< fixpnt< 8, 4, Saturate, std::uint16_t>, Verbose >(reals);
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
