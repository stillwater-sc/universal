// contract_expand.cpp: evaluation of contractions and expansions of posit number systems
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// pull in the posit number system
#include <universal/number/posit/posit.hpp>

template<typename Scalar>
void ContractionExpansion(int depth) {
	using namespace sw::universal;

	int columnWidth = 20;
	Scalar seed = 2.0;
	std::cout << "Contraction/Expansion sequence sqrt(sqrt(sqrt(...sqrt(x))))))^depth => seed with seed = " << seed << '\n';
	std::cout << std::setw(3) << "#"
		<< std::setw(columnWidth) << "contraction"
		<< std::setw(columnWidth) << "expansion"
		<< std::setw(columnWidth) << "error"
		<< '\n';
	for (int i = 1; i < depth; ++i) {
		Scalar x = seed;
		for (int k = 1; k < i; ++k) {
			x = sqrt(x);
		}
		Scalar contraction = x;
		for (int k = 1; k < i; ++k) {
			x = exp2(x);
		}
		Scalar expansion = x;
		std::cout << std::setw(3) << i << " "
			<< std::setw(columnWidth) << contraction << " "
			<< std::setw(columnWidth) << expansion << " "
			<< std::setw(columnWidth) << expansion - seed
			<< '\n';
	}
}

int main()
try {
	using namespace sw::universal;

	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	using Posit = posit<nbits,es>;

	// print detailed bit-level computational intermediate results
	// bool verbose = false;

	// preserve the existing ostream precision
	auto precision = std::cout.precision();
	std::cout << std::setprecision(12);

	ContractionExpansion<Posit>(10);

	// restore the previous ostream precision
	std::cout << std::setprecision(precision);

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
