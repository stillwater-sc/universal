// cfloats.cpp: create detailed component tables that decompose the components that comprise a classic cfloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>

// enable/disable special hex format I/O
#define CFLOAT_ROUNDING_ERROR_FREE_IO_FORMAT 1
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/cfloat/table.hpp>

template<bool hasSubnormals, bool hasMaxExpValues, bool isSaturating, typename bt = uint8_t>
void GenerateCfloatTables(std::ostream& ostr, bool csv) {
	using namespace sw::universal;

	if constexpr(hasSubnormals && hasMaxExpValues)
		GenerateTable< cfloat<3, 1, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);

	if constexpr (hasSubnormals && hasMaxExpValues)
		GenerateTable< cfloat<4, 1, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<4, 2, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);

	if constexpr (hasSubnormals && hasMaxExpValues)
		GenerateTable< cfloat<5, 1, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<5, 2, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<5, 3, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);

	if constexpr (hasSubnormals && hasMaxExpValues)
		GenerateTable< cfloat<6, 1, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<6, 2, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<6, 3, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<6, 4, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);

	if constexpr (hasSubnormals && hasMaxExpValues)
		GenerateTable< cfloat<7, 1, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<7, 2, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<7, 3, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<7, 4, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<7, 5, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);

	if constexpr (hasSubnormals && hasMaxExpValues)
		GenerateTable< cfloat<8, 1, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<8, 2, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<8, 3, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<8, 4, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<8, 5, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);
	GenerateTable< cfloat<8, 6, bt, hasSubnormals, hasMaxExpValues, isSaturating> >(ostr, csv);

}

template<bool hasSubnormals, bool hasMaxExpValues, bool isSaturating, typename BlockType>
void GenerateCfloatTableFiles(bool csv) {
	std::ofstream ostr;
	std::string filename, extension;
	extension = (csv ? ".csv" : ".txt");
	std::string sub, subTypename, sup, supTypename, sat, satTypename;
	if constexpr (hasSubnormals) {
		sub = "t";
		subTypename = "Subnormals";
	}
	else {
		sub = "f";
		subTypename = "noSubnormals";
	}
	if constexpr (hasMaxExpValues) {
		sup = "t";
		supTypename = "Supernormals";
	}
	else {
		sup = "f";
		supTypename = "noSupernormals";
	}
	if constexpr (isSaturating) {
		sat = "t";
		satTypename = "Saturating";
	}
	else {
		sat = "f";
		satTypename = "notSaturating";
	}
	filename = std::string("cfloat_") + sub + sup + sat + extension;
	ostr.open(filename);
	GenerateCfloatTables<hasSubnormals, hasMaxExpValues, isSaturating, uint8_t>(ostr, csv);
	ostr.close();

	std::cout << "Created " << satTypename << " cfloat tables for " << subTypename << ", Normals, " << supTypename << " in " << filename << '\n';

}

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
	constexpr bool hasMaxExpValues = true;
	constexpr bool noSupernormals = false;
	//	constexpr bool isSaturating = true;
	constexpr bool notSaturating = false;

	GenerateCfloatTableFiles<noSubnormals, noSupernormals, notSaturating, uint8_t>(csv);
	GenerateCfloatTableFiles<hasSubnormals, noSupernormals, notSaturating, uint8_t>(csv);
	GenerateCfloatTableFiles<noSubnormals, hasMaxExpValues, notSaturating, uint8_t>(csv);
	GenerateCfloatTableFiles<hasSubnormals, hasMaxExpValues, notSaturating, uint8_t>(csv);

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
