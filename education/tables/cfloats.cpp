// cfloats.cpp: create detailed component tables that decompose the components that comprise a classic cfloat
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if defined(_MSC_VER)
#pragma warning(disable : 4514)  // unreferenced function is removed
#pragma warning(disable : 4710)  // function is not inlined
#pragma warning(disable : 5045)  // compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#endif 
#include <fstream>
#include <iostream>
#include <iomanip>

// enable/disable special hex format I/O
#define CFLOAT_ROUNDING_ERROR_FREE_IO_FORMAT 1
// if you want to trace conversion: NOTE: tracing will destroy the constexpr-ness of operator=()
#define TRACE_CONVERSION 0
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/cfloat/table.hpp>

#define MANUAL_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	// Usage: tables_cfloats [-csv]
	bool csv = false;
	if (argc == 2) {
		if (std::string(argv[1]) == std::string("-csv")) csv = true;
	}
	std::cout << "Generate value tables for cfloat configurations\n";

	constexpr bool hasSubnormals = true;
	constexpr bool noSubnormals = false;
	constexpr bool hasSupernormals = true;
	constexpr bool noSupernormals = false;
	constexpr bool isSaturating = true;
	constexpr bool notSaturating = false;

#if MANUAL_TESTING

	std::ofstream ostr;
	ostr.open("cfloat_8_1_subnormal_supernormal_notsaturating.csv");
	GenerateTable< cfloat<8, 1, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(ostr, true);
	ostr.close();
	ostr.open("cfloat_8_1_subnormal_nosupernormal_notsaturating.csv");
	GenerateTable< cfloat<8, 1, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(ostr, true);
	ostr.close();
	ostr.open("cfloat_8_1_nosubnormal_supernormal_notsaturating.csv");
	GenerateTable< cfloat<8, 1, uint8_t, noSubnormals, hasSupernormals, notSaturating> >(ostr, true);
	ostr.close();
	ostr.open("cfloat_8_1_nosubnormal_nosupernormal_notsaturating.csv");
	GenerateTable< cfloat<8, 1, uint8_t, noSubnormals, noSupernormals, notSaturating> >(ostr, true);
	ostr.close();

	ostr.open("cfloat_8_1_nosubnormal_nosupernormal_notsaturating.txt");
	GenerateTable< cfloat<8, 1, uint8_t, noSubnormals, noSupernormals, notSaturating> >(ostr, false);
	ostr.close();

#else // !MANUAL_TESTING

	// no subnormals, no supernormals, not saturating
	GenerateTable< cfloat<3, 1, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);

	GenerateTable< cfloat<4, 1, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<4, 2, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);

	GenerateTable< cfloat<5, 1, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<5, 2, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<5, 3, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);

	GenerateTable< cfloat<6, 1, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<6, 2, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<6, 3, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<6, 4, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);

	GenerateTable< cfloat<7, 1, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<7, 2, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<7, 3, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<7, 4, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<7, 5, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);

	GenerateTable< cfloat<8, 1, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<8, 2, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<8, 3, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<8, 4, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<8, 5, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<8, 6, uint8_t, noSubnormals, noSupernormals, notSaturating> >(std::cout, csv);

	// subnormals, no supernormals, not saturating
	GenerateTable< cfloat<3, 1, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);

	GenerateTable< cfloat<4, 1, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<4, 2, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);

	GenerateTable< cfloat<5, 1, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<5, 2, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<5, 3, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);

	GenerateTable< cfloat<6, 1, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<6, 2, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<6, 3, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<6, 4, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);

	GenerateTable< cfloat<7, 1, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<7, 2, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<7, 3, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<7, 4, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<7, 5, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);

	GenerateTable< cfloat<8, 1, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<8, 2, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<8, 3, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<8, 4, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<8, 5, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<8, 6, uint8_t, hasSubnormals, noSupernormals, notSaturating> >(std::cout, csv);

	// subnormals, supernormals, not saturating
	GenerateTable< cfloat<3, 1, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);

	GenerateTable< cfloat<4, 1, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<4, 2, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);

	GenerateTable< cfloat<5, 1, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<5, 2, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<5, 3, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);

	GenerateTable< cfloat<6, 1, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<6, 2, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<6, 3, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<6, 4, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);

	GenerateTable< cfloat<7, 1, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<7, 2, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<7, 3, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<7, 4, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<7, 5, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);

	GenerateTable< cfloat<8, 1, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<8, 2, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<8, 3, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<8, 4, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<8, 5, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);
	GenerateTable< cfloat<8, 6, uint8_t, hasSubnormals, hasSupernormals, notSaturating> >(std::cout, csv);
#endif

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& e) {
	std::cerr << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
