// types.cpp: comparison of the different cfloat types with and without sub/supernormals
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
// Configure the cfloat template environment
// first: enable general or specialized configurations
#define CFLOAT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable trace conversion
#define TRACE_CONVERSION 0

#include <universal/number/cfloat/cfloat_impl.hpp>
#include <universal/number/cfloat/manipulators.hpp>
#include <universal/number/cfloat/table.hpp>

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	bool bReportIndividualTestCases = false;

	std::cout << "comparison of different cfloat types\n\n";

	constexpr bool hasSubnormals = true;
	constexpr bool noSubnormals = false;
	constexpr bool hasSupernormals = true;
	constexpr bool noSupernormals = false;
	constexpr bool isSaturating = true;
	constexpr bool notSaturating = false;

	// if you  have 1 exponent bits, then all encodings are either subnormals or supernormals.
	// In the following set of cfloat<5,1> types, the last type with subnormals and supernormals 
	// is the only type that has non-trivial values for its encodings.
	GenerateTable<cfloat<5, 1, uint8_t, noSubnormals , noSupernormals , notSaturating> >(std::cout);
	GenerateTable<cfloat<5, 1, uint8_t, hasSubnormals, noSupernormals , notSaturating> >(std::cout);
	GenerateTable<cfloat<5, 1, uint8_t, noSubnormals , hasSupernormals, notSaturating> >(std::cout);
	GenerateTable<cfloat<5, 1, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout); // <---- only interesting encoding interpretation for es=1

	GenerateTable<cfloat<5, 2, uint8_t, noSubnormals , noSupernormals , notSaturating> >(std::cout);
	GenerateTable<cfloat<5, 2, uint8_t, hasSubnormals, noSupernormals , notSaturating> >(std::cout);
	GenerateTable<cfloat<5, 2, uint8_t, noSubnormals , hasSupernormals, notSaturating> >(std::cout);
	GenerateTable<cfloat<5, 2, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout);

	GenerateTable<cfloat<5, 3, uint8_t, noSubnormals , noSupernormals , notSaturating> >(std::cout);
	GenerateTable<cfloat<5, 3, uint8_t, hasSubnormals, noSupernormals , notSaturating> >(std::cout);
	GenerateTable<cfloat<5, 3, uint8_t, noSubnormals , hasSupernormals, notSaturating> >(std::cout);
	GenerateTable<cfloat<5, 3, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout);

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_arithmetic_exception& err) {
	std::cerr << "Uncaught cfloat arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_internal_exception& err) {
	std::cerr << "Uncaught cfloat internal exception: " << err.what() << std::endl;
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
