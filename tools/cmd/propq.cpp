// propq.cpp: cli to show a table of quires
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/posit/posit.hpp>
#include <universal/number/posit/quire.hpp>

typedef std::numeric_limits< double > dbl;

template<size_t nbits, size_t capacity = 10>
void QuireSizeTableRow(std::ostream& ostr, unsigned first_column, unsigned size_column) {
	using namespace sw::universal;
	ostr << std::setw(first_column) << nbits;
	ostr << std::setw(size_column) << quire_size<nbits, 0, capacity>();
	ostr << std::setw(size_column) << quire_size<nbits, 1, capacity>();
	ostr << std::setw(size_column) << quire_size<nbits, 2, capacity>();
	ostr << std::setw(size_column) << quire_size<nbits, 3, capacity>();
	ostr << std::setw(size_column) << quire_size<nbits, 4, capacity>();
	ostr << std::setw(size_column) << quire_size<nbits, 5, capacity>();
	ostr << std::setw(size_column) << quire_size<nbits, 6, capacity>();
	ostr << std::setw(size_column) << quire_size<nbits, 7, capacity>();
	ostr << std::setw(size_column) << quire_size<nbits, 8, capacity>();
	ostr << std::setw(size_column) << quire_size<nbits, 9, capacity>();
	ostr << std::endl;
}

template<size_t nbits, size_t capacity = 10>
void QuireSizeTable(std::ostream& ostr) {
	using namespace sw::universal;

	ostr << "Quire size table as a function of <nbits, es, capacity = " << capacity << ">\n";
	ostr << "Capacity is 2^" << capacity << " accumulations of max_pos^2\n";
	unsigned first_column = 8;
	unsigned size_column = 8;
	ostr << std::setw(first_column) << "nbits" << std::setw(size_column * 5) << "es value\n";
	ostr << std::setw(first_column) << "   +";
	for (int i = 0; i < 10; ++i) {
		ostr << std::setw(size_column) << i;
	}
	ostr << std::endl;
	QuireSizeTableRow<nbits + 0, capacity>(ostr, first_column, size_column);
	QuireSizeTableRow<nbits + 1, capacity>(ostr, first_column, size_column);
	QuireSizeTableRow<nbits + 2, capacity>(ostr, first_column, size_column);
	QuireSizeTableRow<nbits + 3, capacity>(ostr, first_column, size_column);
	QuireSizeTableRow<nbits + 4, capacity>(ostr, first_column, size_column);
	QuireSizeTableRow<nbits + 5, capacity>(ostr, first_column, size_column);
	QuireSizeTableRow<nbits + 6, capacity>(ostr, first_column, size_column);
	QuireSizeTableRow<nbits + 7, capacity>(ostr, first_column, size_column);
	QuireSizeTableRow<nbits + 8, capacity>(ostr, first_column, size_column);
}

// receive a float and print its components
int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc == 1) std::cout << argv[0] << ": print quire size tables\n";

	// print a standard quire size table
	QuireSizeTable<4>(std::cout); std::cout << '\n';
	QuireSizeTable<8>(std::cout); std::cout << '\n';
	QuireSizeTable<16>(std::cout); std::cout << '\n';
	QuireSizeTable<24>(std::cout); std::cout << '\n';
	QuireSizeTable<32>(std::cout); std::cout << '\n';
	QuireSizeTable<40>(std::cout); std::cout << '\n';
	QuireSizeTable<48>(std::cout); std::cout << '\n';
	QuireSizeTable<56>(std::cout); std::cout << '\n';
	QuireSizeTable<64>(std::cout); std::cout << '\n';
	QuireSizeTable<80>(std::cout); std::cout << '\n';
	
	std::cout << std::endl;

	return EXIT_SUCCESS;
}
catch (const char* const msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
