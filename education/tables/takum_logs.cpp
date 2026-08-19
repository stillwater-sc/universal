// takum_logs.cpp: generates encoding tables of logarithmic takum configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The companion to takums.cpp.  The two variants share the (S, D, R, C, M) bit
// layout exactly, so reading the two tables side by side is the clearest way to
// see what the value map alone changes: identical encodings, identical sign,
// direction, regime and characteristic columns, and completely different values.
//
// The last column differs by design.  A linear takum's value is (1 + f) * 2^c, so
// its base-2 scale is the natural companion; a logarithmic takum's value is
// sqrt(e)^l, and l -- the quantity the encoding actually carries -- is printed
// instead.  takum_log::scale() exists and is base 2 for library-wide consistency,
// but it is a converted quantity and would be the less honest column here.
#include <universal/utility/directives.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>

// Configure the takum template environment
#include <universal/number/takum/takum.hpp>
#include <universal/number/takum/table.hpp>

#define MANUAL_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	// Usage: edu_tables_takum_logs [-csv]
	bool csv = false;
	if (argc == 2) {
		std::string arg(argv[1]);
		if (arg == "-csv") {
			csv = true;
		}
		else {
			std::cerr << "Usage: " << argv[0] << " [-csv]\n";
			return EXIT_FAILURE;
		}
	}
	else if (argc > 2) {
		std::cerr << "Usage: " << argv[0] << " [-csv]\n";
		return EXIT_FAILURE;
	}
	std::cout << "Generate value tables for logarithmic takum number system configurations\n";

#if MANUAL_TESTING

	GenerateTakumLogTable<6, 3>(std::cout, csv);
	GenerateTakumLogTable<8, 3>(std::cout, csv);

#else // !MANUAL_TESTING

	std::ofstream ostr;
	std::string filename, extension;
	extension = (csv ? ".csv" : ".txt");
	filename = std::string("takum_logs") + extension;
	ostr.open(filename);
	if (!ostr) {
		std::cerr << "Error: could not open " << filename << " for writing\n";
		return EXIT_FAILURE;
	}

	// The same configurations takums.cpp emits, so the two files line up row for row.

	// Standard takum configurations (rbits = 3, the paper's default)
	// Small configurations: 6, 7, 8 bits
	GenerateTakumLogTable<6, 3>(ostr, csv);
	GenerateTakumLogTable<7, 3>(ostr, csv);
	GenerateTakumLogTable<8, 3>(ostr, csv);

	// Medium configurations: 10, 12 bits
	GenerateTakumLogTable<10, 3>(ostr, csv);
	GenerateTakumLogTable<12, 3>(ostr, csv);

	// Exploration: varying regime widths at 8 bits
	// rbits=2: narrower range, more mantissa bits
	GenerateTakumLogTable<8, 2>(ostr, csv);
	// rbits=4: wider range, fewer mantissa bits
	GenerateTakumLogTable<8, 4>(ostr, csv);

	ostr.close();
	std::cout << "Created value tables for takum_log<nbits, rbits> in " << filename << '\n';

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
catch (const std::exception& err) {
	std::cerr << "Uncaught exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
