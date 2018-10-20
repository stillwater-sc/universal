// posit_properties.cpp: show the arithmetic properties of posit configurations
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "common.hpp"

#include <posit>

typedef std::numeric_limits< double > dbl;
const char* msg = "arithmetic properties of a posit<32, 1> environment\n\
posit< 32, 1> useed scale     2     minpos scale - 60     maxpos scale         60\n\
Properties of a quire<32, 1, 10>\n\
dynamic range of product   : 240\n\
radix point of accumulator : 120\n\
full  quire size in bits   : 250\n\
lower quire size in bits   : 120\n\
upper quire size in bits   : 121\n\
capacity bits : 10\n\
\n\
+ : 0000000000_0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000.000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";

template<size_t nbits, size_t es, size_t capacity = 10>
void arithmetic_properties(std::ostream& ostr) {
	ostr << dynamic_range(sw::unum::posit<nbits, es>()) << std::endl;
	ostr << sw::unum::quire_properties<nbits, es, capacity>() << std::endl;
	ostr << sw::unum::quire<nbits, es, capacity>() << std::endl;
}


// transformation of user-provided values to constexpr values
void ReportArithmeticProperties(size_t nbits, size_t es, size_t capacity) {
	using namespace std;
	using namespace sw::unum;

	cout << "arithmetic properties of a posit<" << nbits << ", " << es << "> environment" << endl;

/*
	constexpr size_t es_0 = 0;
	constexpr size_t es_1 = 1;
	constexpr size_t es_2 = 2;
	constexpr size_t es_3 = 3;
	constexpr size_t es_4 = 4;
	constexpr size_t es_5 = 5;
	constexpr size_t es_6 = 6;
	constexpr size_t es_7 = 7;
	constexpr size_t es_8 = 8;
	constexpr size_t es_9 = 9;
*/

	switch (nbits) {
	case 1:
		cerr << "nbits = 1 implies just a sign bit" << endl;
		break;
	case 8:
	{
		constexpr size_t nbits = 8;
		switch (es) {
		case 0:
			arithmetic_properties<nbits, 0, 10>(cout);
			break;
		case 1:
			arithmetic_properties<nbits, 1, 10>(cout);
			break;
		case 2:
			arithmetic_properties<nbits, 2, 10>(cout);
			break;
		case 3:
			arithmetic_properties<nbits, 3, 10>(cout);
			break;
		case 4:
			arithmetic_properties<nbits, 4, 10>(cout);
			break;
		case 5:
			arithmetic_properties<nbits, 5, 10>(cout);
			break;
		case 6:
			arithmetic_properties<nbits, 6, 10>(cout);
			break;
		case 7:
			arithmetic_properties<nbits, 7, 10>(cout);
			break;
		case 8:
			arithmetic_properties<nbits, 8, 10>(cout);
			break;
		case 9:
			arithmetic_properties<nbits, 9, 10>(cout);
			break;
		default:
			cerr << "es = " << es << " unsupported" << endl;
		}
	}
		break;
	case 16:
	{
		constexpr size_t nbits = 16;
		switch (es) {
		case 0:
			arithmetic_properties<nbits, 0, 10>(cout);
			break;
		case 1:
			arithmetic_properties<nbits, 1, 10>(cout);
			break;
		case 2:
			arithmetic_properties<nbits, 2, 10>(cout);
			break;
		case 3:
			arithmetic_properties<nbits, 3, 10>(cout);
			break;
		case 4:
			arithmetic_properties<nbits, 4, 10>(cout);
			break;
		case 5:
			arithmetic_properties<nbits, 5, 10>(cout);
			break;
		case 6:
			arithmetic_properties<nbits, 6, 10>(cout);
			break;
		case 7:
			arithmetic_properties<nbits, 7, 10>(cout);
			break;
		case 8:
			arithmetic_properties<nbits, 8, 10>(cout);
			break;
		case 9:
			arithmetic_properties<nbits, 9, 10>(cout);
			break;
		default:
			cerr << "es = " << es << " unsupported" << endl;
		}
	}
		break;
	case 31:
	{
		constexpr size_t nbits = 31;
		switch (es) {
		case 0:
			arithmetic_properties<nbits, 0, 10>(cout);
			break;
		case 1:
			arithmetic_properties<nbits, 1, 10>(cout);
			break;
		case 2:
			arithmetic_properties<nbits, 2, 10>(cout);
			break;
		case 3:
			arithmetic_properties<nbits, 3, 10>(cout);
			break;
		case 4:
			arithmetic_properties<nbits, 4, 10>(cout);
			break;
		case 5:
			arithmetic_properties<nbits, 5, 10>(cout);
			break;
		case 6:
			arithmetic_properties<nbits, 6, 10>(cout);
			break;
		case 7:
			arithmetic_properties<nbits, 7, 10>(cout);
			break;
		case 8:
			arithmetic_properties<nbits, 8, 10>(cout);
			break;
		case 9:
			arithmetic_properties<nbits, 9, 10>(cout);
			break;
		default:
			cerr << "es = " << es << " unsupported" << endl;
		}
	}
		break;
	case 32:
	{
		constexpr size_t nbits = 32;
		switch (es) {
		case 0:
			arithmetic_properties<nbits, 0, 10>(cout);
			break;
		case 1:
			arithmetic_properties<nbits, 1, 10>(cout);
			break;
		case 2:
			arithmetic_properties<nbits, 2, 10>(cout);
			break;
		case 3:
			arithmetic_properties<nbits, 3, 10>(cout);
			break;
		case 4:
			arithmetic_properties<nbits, 4, 10>(cout);
			break;
		case 5:
			arithmetic_properties<nbits, 5, 10>(cout);
			break;
		case 6:
			arithmetic_properties<nbits, 6, 10>(cout);
			break;
		case 7:
			arithmetic_properties<nbits, 7, 10>(cout);
			break;
		case 8:
			arithmetic_properties<nbits, 8, 10>(cout);
			break;
		case 9:
			arithmetic_properties<nbits, 9, 10>(cout);
			break;
		default:
			cerr << "es = " << es << " unsupported" << endl;
		}
	}
		break;
	case 64:
		{
			constexpr size_t nbits = 64;
			switch (es) {
			case 0:
				arithmetic_properties<nbits, 0, 10>(cout);
				break;
			case 1:
				arithmetic_properties<nbits, 1, 10>(cout);
				break;
			case 2:
				arithmetic_properties<nbits, 2, 10>(cout);
				break;
			case 3:
				arithmetic_properties<nbits, 3, 10>(cout);
				break;
			case 4:
				arithmetic_properties<nbits, 4, 10>(cout);
				break;
			case 5:
				arithmetic_properties<nbits, 5, 10>(cout);
				break;
			case 6:
				arithmetic_properties<nbits, 6, 10>(cout);
				break;
			case 7:
				arithmetic_properties<nbits, 7, 10>(cout);
				break;
			case 8:
				arithmetic_properties<nbits, 8, 10>(cout);
				break;
			case 9:
				arithmetic_properties<nbits, 9, 10>(cout);
				break;
			default:
				cerr << "es = " << es << " unsupported" << endl;
			}
		}
		break;
	default:
		cerr << "nbits = " << nbits << " unsupported" << endl;
	}
}

template<size_t nbits, size_t capacity = 10>
void QuireSizeTableRow(std::ostream& ostr, unsigned first_column, unsigned size_column) {
	using namespace std;
	using namespace sw::unum;
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
	using namespace sw::unum;

	ostr << "Quire size table as a function of <nbits, es, capacity>" << endl;
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
	using namespace sw::unum;

	if (argc == 1) {
		// print a standard quire size table
		QuireSizeTable<4>(cout); cout << endl;
		QuireSizeTable<8>(cout); cout << endl;
		QuireSizeTable<16>(cout); cout << endl;
		QuireSizeTable<24>(cout); cout << endl;
		return EXIT_SUCCESS;
	}
	else if (argc != 4) {
		cerr << "Show the arithmetic properties of a posit." << endl;
	    cerr << "Usage: posit_properties [nbits es capacity]" << endl;
		cerr << "Example: posit_properties 32 1 10" << endl;
		cerr <<  msg << endl;
		return EXIT_SUCCESS;  // signal successful completion for ctest
	}

	size_t nbits = atoi(argv[1]);
	size_t es = atoi(argv[2]);
	size_t capacity = atoi(argv[3]);

	ReportArithmeticProperties(nbits, es, capacity);


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
