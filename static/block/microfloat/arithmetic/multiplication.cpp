// multiplication.cpp: multiplication correctness tests for microfloat types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define MICROFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/microfloat/microfloat.hpp>
#include <universal/verification/test_suite.hpp>

template<typename MicrofloatType>
int VerifyMultiplication() {
	constexpr unsigned nbits = MicrofloatType::nbits;
	constexpr unsigned total_encodings = (1u << nbits);
	int nrOfFailedTestCases = 0;

	for (unsigned i = 0; i < total_encodings; ++i) {
		MicrofloatType a;
		a.setbits(i);
		if (a.isnan() || a.isinf()) continue;

		for (unsigned j = 0; j < total_encodings; ++j) {
			MicrofloatType b;
			b.setbits(j);
			if (b.isnan() || b.isinf()) continue;

			float fa = float(a);
			float fb = float(b);
			float fprod = fa * fb;
			MicrofloatType prod = a * b;
			MicrofloatType ref(fprod);

			if (prod.isnan() && ref.isnan()) continue;
			if (prod.iszero() && ref.iszero()) continue;

			if (prod.bits() != ref.bits()) {
				++nrOfFailedTestCases;
				if (nrOfFailedTestCases < 10) {
					std::cerr << "FAIL: " << float(a) << " * " << float(b)
						<< " = " << float(prod) << " (expected " << float(ref) << ")\n";
				}
			}
		}
	}
	return nrOfFailedTestCases;
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "microfloat multiplication tests";
	int nrOfFailedTestCases = 0;

	std::cout << "e2m1 multiplication (exhaustive 4-bit)\n";
	nrOfFailedTestCases += VerifyMultiplication<e2m1>();

	std::cout << "e2m3 multiplication (exhaustive 6-bit)\n";
	nrOfFailedTestCases += VerifyMultiplication<e2m3>();

	std::cout << "e3m2 multiplication (exhaustive 6-bit)\n";
	nrOfFailedTestCases += VerifyMultiplication<e3m2>();

	std::cout << "e4m3 multiplication (exhaustive 8-bit)\n";
	nrOfFailedTestCases += VerifyMultiplication<e4m3>();

	std::cout << "e5m2 multiplication (exhaustive 8-bit)\n";
	nrOfFailedTestCases += VerifyMultiplication<e5m2>();

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
