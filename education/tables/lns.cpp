// lns.cpp: generates encoding tables of lns configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>
// Configure the lns template environment
// first: enable general or specialized lns configurations
#define LNS_FAST_SPECIALIZATION
// second: enable/disable lns arithmetic exceptions
#define LNS_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/lns/lns.hpp>
#include <universal/number/lns/table.hpp>

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	// Usage: edu_tables_lns [-csv]
	bool csv = false;
	if (argc == 2) {
		if (std::string(argv[1]) == std::string("-csv")) csv = true;
	}
	std::cout << "Generate value tables for logarithmic number system configurations\n";

#if MANUAL_TESTING

	GenerateLnsTable<4, 2>(std::cout, csv);
	GenerateLnsTable<6, 2>(std::cout, csv);
	GenerateLnsTable<8, 4>(std::cout, csv);

#else // !MANUAL_TESTING

	std::ofstream ostr;
	std::string filename, extension;
	extension = (csv ? ".csv" : ".txt");
	filename = std::string("lns") + extension;
	ostr.open(filename);

	GenerateLnsTable<4, 0>(ostr, csv);
	GenerateLnsTable<4, 1>(ostr, csv);
	GenerateLnsTable<4, 2>(ostr, csv);
	GenerateLnsTable<4, 3>(ostr, csv);

	GenerateLnsTable<5, 0>(ostr, csv);
	GenerateLnsTable<5, 1>(ostr, csv);
	GenerateLnsTable<5, 2>(ostr, csv);
	GenerateLnsTable<5, 3>(ostr, csv);
	GenerateLnsTable<5, 4>(ostr, csv);

	GenerateLnsTable<6, 0>(ostr, csv);
	GenerateLnsTable<6, 1>(ostr, csv);
	GenerateLnsTable<6, 2>(ostr, csv);
	GenerateLnsTable<6, 3>(ostr, csv);
	GenerateLnsTable<6, 4>(ostr, csv);
	GenerateLnsTable<6, 5>(ostr, csv);

	GenerateLnsTable<8, 0>(ostr, csv);
	GenerateLnsTable<8, 1>(ostr, csv);
	GenerateLnsTable<8, 2>(ostr, csv);
	GenerateLnsTable<8, 3>(ostr, csv);
	GenerateLnsTable<8, 4>(ostr, csv);
	GenerateLnsTable<8, 5>(ostr, csv);
	GenerateLnsTable<8, 6>(ostr, csv);
	GenerateLnsTable<8, 7>(ostr, csv);

	// edge case, where sign and exponent msb are in different limbs
	GenerateLnsTable<9, 2>(ostr, csv);
	GenerateLnsTable<9, 4>(ostr, csv);
	GenerateLnsTable<9, 6>(ostr, csv);

	ostr.close();
	std::cout << "Created value tables for lns<nbits, rbits> in " << filename << '\n';

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
