// propq.cpp: cli to show a table of quires
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/posit/posit.hpp>

typedef std::numeric_limits< double > dbl;

template<size_t nbits, size_t capacity = 10>
void QuireSizeTableRow(std::ostream& ostr, unsigned first_column, unsigned size_column) {
	using namespace std;
	using namespace sw::universal;
	ostr << setw(first_column) << nbits;
	ostr << setw(size_column) << quire_size<nbits, 0, capacity>();
	ostr << setw(size_column) << quire_size<nbits, 1, capacity>();
	ostr << setw(size_column) << quire_size<nbits, 2, capacity>();
	ostr << setw(size_column) << quire_size<nbits, 3, capacity>();
	ostr << setw(size_column) << quire_size<nbits, 4, capacity>();
	ostr << setw(size_column) << quire_size<nbits, 5, capacity>();
	ostr << setw(size_column) << quire_size<nbits, 6, capacity>();
	ostr << setw(size_column) << quire_size<nbits, 7, capacity>();
	ostr << setw(size_column) << quire_size<nbits, 8, capacity>();
	ostr << setw(size_column) << quire_size<nbits, 9, capacity>();
	ostr << endl;
}

template<size_t nbits, size_t capacity = 10>
void QuireSizeTable(std::ostream& ostr) {
	using namespace std;
	using namespace sw::universal;

	ostr << "Quire size table as a function of <nbits, es, capacity = " << capacity << ">" << endl;
	ostr << "Capacity is 2^" << capacity << " accumulations of max_pos^2" << endl;
	unsigned first_column = 8;
	unsigned size_column = 8;
	ostr << setw(first_column) << "nbits" << setw(size_column * 5) << "es value" << endl;
	ostr << setw(first_column) << "   +";
	for (int i = 0; i < 10; ++i) {
		ostr << setw(size_column) << i;
	}
	ostr << endl;
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
	using namespace std;
	using namespace sw::universal;

	if (argc == 1) cout << argv[0] << ": print quire size tables" << endl;

	// print a standard quire size table
	QuireSizeTable<4>(cout); cout << '\n';
	QuireSizeTable<8>(cout); cout << '\n';
	QuireSizeTable<16>(cout); cout << '\n';
	QuireSizeTable<24>(cout); cout << '\n';
	QuireSizeTable<32>(cout); cout << '\n';
	QuireSizeTable<40>(cout); cout << '\n';
	QuireSizeTable<48>(cout); cout << '\n';
	QuireSizeTable<56>(cout); cout << '\n';
	QuireSizeTable<64>(cout); cout << '\n';
	QuireSizeTable<80>(cout); cout << '\n';
	
	cout << endl;

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
