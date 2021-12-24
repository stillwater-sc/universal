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
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/cfloat/table.hpp>

template<bool hasSubnormals, bool hasSupernormals, bool isSaturating, typename bt = uint8_t>
void GenerateCfloatTables(std::ostream& ostr, bool csv) {
	using namespace sw::universal;

	if constexpr(hasSubnormals && hasSupernormals)
		GenerateTable< cfloat<3, 1, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);

	if constexpr (hasSubnormals && hasSupernormals)
		GenerateTable< cfloat<4, 1, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<4, 2, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);

	if constexpr (hasSubnormals && hasSupernormals)
		GenerateTable< cfloat<5, 1, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<5, 2, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<5, 3, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);

	if constexpr (hasSubnormals && hasSupernormals)
		GenerateTable< cfloat<6, 1, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<6, 2, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<6, 3, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<6, 4, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);

	if constexpr (hasSubnormals && hasSupernormals)
		GenerateTable< cfloat<7, 1, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<7, 2, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<7, 3, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<7, 4, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<7, 5, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);

	if constexpr (hasSubnormals && hasSupernormals)
		GenerateTable< cfloat<8, 1, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<8, 2, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<8, 3, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<8, 4, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<8, 5, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<8, 6, bt, hasSubnormals, hasSupernormals, isSaturating> >(ostr, csv);

}

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
//	constexpr bool isSaturating = true;
	constexpr bool notSaturating = false;

	std::ofstream ostr;
	std::string filename, extension;
	extension = (csv ? ".csv" : ".txt");
	filename = std::string("cfloat_fff") + extension;
	ostr.open(filename);
	// no subnormals, has normals, no supernormals, not saturating
	GenerateCfloatTables<noSubnormals, noSupernormals, notSaturating, uint8_t>(ostr, csv);
	std::cout << "Created cfloat tables for noSubnormals, Normals, noSupernormals in " << filename << '\n';
	ostr.close();

	filename = std::string("cfloat_tff") + extension;
	ostr.open(filename);
	// has subnormals, has normals, no supernormals, not saturating
	GenerateCfloatTables<hasSubnormals, noSupernormals, notSaturating>(ostr, csv);
	std::cout << "Created cfloat tables for Subnormals, Normals, noSupernormals in " << filename << '\n';
	ostr.close();

	filename = std::string("cfloat_ftf") + extension;
	ostr.open(filename);
	// no subnormals, has normals, has supernormals, not saturating
	GenerateCfloatTables<noSubnormals, hasSupernormals, notSaturating, uint8_t>(ostr, csv);
	std::cout << "Created cfloat tables for noSubnormals, Normals, Supernormals in " << filename << '\n';
	ostr.close();

	filename = std::string("cfloat_ttf") + extension;
	ostr.open(filename);
	// has subnormals, has normals, has supernormals, not saturating
	GenerateCfloatTables<hasSubnormals, hasSupernormals, notSaturating>(ostr, csv);
	std::cout << "Created cfloat tables for Subnormals, Normals, and Supernormals in " << filename << '\n';
	ostr.close();

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc error: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& e) {
	std::cerr << "Caught unexpected runtime error: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
