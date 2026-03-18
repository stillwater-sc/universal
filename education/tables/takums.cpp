// takums.cpp: generates encoding tables of takum configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>
// Configure the takum template environment
#define TAKUM_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/takum/takum.hpp>
#include <universal/number/takum/table.hpp>

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	// Usage: edu_tables_takums [-csv]
	bool csv = false;
	if (argc == 2) {
		if (std::string(argv[1]) == std::string("-csv")) csv = true;
	}
	std::cout << "Generate value tables for takum number system configurations\n";

#if MANUAL_TESTING

	GenerateTakumTable<6, 3>(std::cout, csv);
	GenerateTakumTable<8, 3>(std::cout, csv);

#else // !MANUAL_TESTING

	std::ofstream ostr;
	std::string filename, extension;
	extension = (csv ? ".csv" : ".txt");
	filename = std::string("takums") + extension;
	ostr.open(filename);
	if (!ostr) {
		std::cerr << "Error: could not open " << filename << " for writing\n";
		return EXIT_FAILURE;
	}

	// Standard takum configurations (rbits = 3, the paper's default)
	// Small configurations: 6, 7, 8 bits
	GenerateTakumTable<6, 3>(ostr, csv);
	GenerateTakumTable<7, 3>(ostr, csv);
	GenerateTakumTable<8, 3>(ostr, csv);

	// Medium configurations: 10, 12 bits
	GenerateTakumTable<10, 3>(ostr, csv);
	GenerateTakumTable<12, 3>(ostr, csv);

	// Exploration: varying regime widths at 8 bits
	// rbits=2: narrower range, more mantissa bits
	GenerateTakumTable<8, 2>(ostr, csv);
	// rbits=4: wider range, fewer mantissa bits
	GenerateTakumTable<8, 4>(ostr, csv);

	ostr.close();
	std::cout << "Created value tables for takum<nbits, rbits> in " << filename << '\n';

#endif

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught universal internal exception: " << err.what() << std::endl;
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
