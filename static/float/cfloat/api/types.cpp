// types.cpp: comparison of the different cfloat types with and without sub/max-exponent values
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/cfloat/table.hpp>

int main()
try {
	using namespace sw::universal;

	//bool bReportIndividualTestCases = false;

	std::cout << "comparison of different cfloat types\n\n";

	constexpr bool hasSubnormals   = true;
	constexpr bool noSubnormals    = false;
	constexpr bool hasMaxExpValues = true;
	constexpr bool noSupernormals  = false;
	constexpr bool isSaturating    = true;
	constexpr bool notSaturating   = false;

	// if you  have 1 exponent bits, then all encodings are either subnormals or max-exponent values.
	// In the following set of cfloat<5,1> types, the last type with subnormals and max-exponent values 
	// is the only type that has non-trivial values for its encodings.
//	GenerateTable<cfloat<5, 1, uint8_t, noSubnormals , noSupernormals , notSaturating> >(std::cout); // invalid
//	GenerateTable<cfloat<5, 1, uint8_t, hasSubnormals, noSupernormals , notSaturating> >(std::cout); // invalid
//	GenerateTable<cfloat<5, 1, uint8_t, noSubnormals , hasMaxExpValues, notSaturating> >(std::cout); // invalid
	GenerateTable<cfloat<5, 1, uint8_t, hasSubnormals, hasMaxExpValues, notSaturating> >(std::cout); // <---- only interesting encoding interpretation for es=1

	GenerateTable<cfloat<5, 2, uint8_t, noSubnormals , noSupernormals , notSaturating> >(std::cout);
	GenerateTable<cfloat<5, 2, uint8_t, hasSubnormals, noSupernormals , notSaturating> >(std::cout);
	GenerateTable<cfloat<5, 2, uint8_t, noSubnormals , hasMaxExpValues, notSaturating> >(std::cout);
	GenerateTable<cfloat<5, 2, uint8_t, hasSubnormals, hasMaxExpValues, notSaturating> >(std::cout);

	GenerateTable<cfloat<5, 3, uint8_t, noSubnormals , noSupernormals , notSaturating> >(std::cout);
	GenerateTable<cfloat<5, 3, uint8_t, hasSubnormals, noSupernormals , notSaturating> >(std::cout);
	GenerateTable<cfloat<5, 3, uint8_t, noSubnormals , hasMaxExpValues, notSaturating> >(std::cout);
	GenerateTable<cfloat<5, 3, uint8_t, hasSubnormals, hasMaxExpValues, notSaturating> >(std::cout);

	// saturing is a property of the arithmetic, not the encoding, and thus the tables of values are identical
	GenerateTable<cfloat<5, 3, uint8_t, hasSubnormals, hasMaxExpValues, isSaturating> >(std::cout);

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
