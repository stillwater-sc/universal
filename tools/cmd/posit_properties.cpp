// posit_properties.cpp: cli to show the arithmetic properties of posit configurations
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/posit/posit>

typedef std::numeric_limits< double > dbl;
const char* msg = "arithmetic properties of a posit<16, 1> environment\n\
 posit< 16, 1> useed scale     2     minpos scale - 28     maxpos scale         28\n\
  minpos                     : 16.1x0001p + 3.72529e-09\n\
  maxpos                     : 16.1x7fffp + 2.68435e+08\n\
Properties of a quire<16, 1, 8>\n\
  dynamic range of product   : 112\n\
  radix point of accumulator :  56\n\
  full  quire size in bits   : 120\n\
  lower quire size in bits   :  56\n\
  upper quire size in bits   :  57\n\
  capacity bits              :   8\n\
Quire segments\n\
+ : 00000000_000000000000000000000000000000000000000000000000000000000.00000000000000000000000000000000000000000000000000000000\n";

template<size_t nbits, size_t es, size_t capacity = 10>
void arithmetic_properties(std::ostream& ostr) {
	ostr << sw::unum::posit_range<nbits, es>() << std::endl;
	ostr << "  minpos                     : " << sw::unum::posit_format(sw::unum::minpos<nbits, es>()) << " " << sw::unum::minpos<nbits, es>() << std::endl;
	ostr << "  maxpos                     : " << sw::unum::posit_format(sw::unum::maxpos<nbits, es>()) << " " << sw::unum::maxpos<nbits, es>() << std::endl;
	ostr << sw::unum::quire_properties<nbits, es, capacity>() << std::endl;
	ostr << "Quire segments" << std::endl;
	ostr << sw::unum::quire<nbits, es, capacity>() << std::endl;
}

// transformation of user-provided values to constexpr values
template<size_t capacity>
void ReportArithmeticProperties(size_t nbits, size_t es) {
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
			arithmetic_properties<nbits, 0, capacity>(cout);
			break;
		case 1:
			arithmetic_properties<nbits, 1, capacity>(cout);
			break;
		case 2:
			arithmetic_properties<nbits, 2, capacity>(cout);
			break;
		case 3:
			arithmetic_properties<nbits, 3, capacity>(cout);
			break;
		case 4:
			arithmetic_properties<nbits, 4, capacity>(cout);
			break;
		case 5:
			arithmetic_properties<nbits, 5, capacity>(cout);
			break;
		case 6:
			arithmetic_properties<nbits, 6, capacity>(cout);
			break;
		case 7:
			arithmetic_properties<nbits, 7, capacity>(cout);
			break;
		case 8:
			arithmetic_properties<nbits, 8, capacity>(cout);
			break;
		case 9:
			arithmetic_properties<nbits, 9, capacity>(cout);
			break;
		default:
			cerr << "es = " << es << " reporting is not supported by this program" << endl;
		}
	}
		break;
	case 16:
	{
		constexpr size_t nbits = 16;
		switch (es) {
		case 0:
			arithmetic_properties<nbits, 0, capacity>(cout);
			break;
		case 1:
			arithmetic_properties<nbits, 1, capacity>(cout);
			break;
		case 2:
			arithmetic_properties<nbits, 2, capacity>(cout);
			break;
		case 3:
			arithmetic_properties<nbits, 3, capacity>(cout);
			break;
		case 4:
			arithmetic_properties<nbits, 4, capacity>(cout);
			break;
		case 5:
			arithmetic_properties<nbits, 5, capacity>(cout);
			break;
		case 6:
			arithmetic_properties<nbits, 6, capacity>(cout);
			break;
		case 7:
			arithmetic_properties<nbits, 7, capacity>(cout);
			break;
		case 8:
			arithmetic_properties<nbits, 8, capacity>(cout);
			break;
		case 9:
			arithmetic_properties<nbits, 9, capacity>(cout);
			break;
		default:
			cerr << "es = " << es << " reporting is not supported by this program" << endl;
		}
	}
		break;
	case 31:
	{
		constexpr size_t nbits = 31;
		switch (es) {
		case 0:
			arithmetic_properties<nbits, 0, capacity>(cout);
			break;
		case 1:
			arithmetic_properties<nbits, 1, capacity>(cout);
			break;
		case 2:
			arithmetic_properties<nbits, 2, capacity>(cout);
			break;
		case 3:
			arithmetic_properties<nbits, 3, capacity>(cout);
			break;
		case 4:
			arithmetic_properties<nbits, 4, capacity>(cout);
			break;
		case 5:
			arithmetic_properties<nbits, 5, capacity>(cout);
			break;
		case 6:
			arithmetic_properties<nbits, 6, capacity>(cout);
			break;
		case 7:
			arithmetic_properties<nbits, 7, capacity>(cout);
			break;
		case 8:
			arithmetic_properties<nbits, 8, capacity>(cout);
			break;
		case 9:
			arithmetic_properties<nbits, 9, capacity>(cout);
			break;
		default:
			cerr << "es = " << es << " reporting is not supported by this program" << endl;
		}
	}
		break;
	case 32:
	{
		constexpr size_t nbits = 32;
		switch (es) {
		case 0:
			arithmetic_properties<nbits, 0, capacity>(cout);
			break;
		case 1:
			arithmetic_properties<nbits, 1, capacity>(cout);
			break;
		case 2:
			arithmetic_properties<nbits, 2, capacity>(cout);
			break;
		case 3:
			arithmetic_properties<nbits, 3, capacity>(cout);
			break;
		case 4:
			arithmetic_properties<nbits, 4, capacity>(cout);
			break;
		case 5:
			arithmetic_properties<nbits, 5, capacity>(cout);
			break;
		case 6:
			arithmetic_properties<nbits, 6, capacity>(cout);
			break;
		case 7:
			arithmetic_properties<nbits, 7, capacity>(cout);
			break;
		case 8:
			arithmetic_properties<nbits, 8, capacity>(cout);
			break;
		case 9:
			arithmetic_properties<nbits, 9, capacity>(cout);
			break;
		default:
			cerr << "es = " << es << " reporting is not supported by this program" << endl;
		}
	}
		break;
	case 64:
		{
			constexpr size_t nbits = 64;
			switch (es) {
			case 0:
				arithmetic_properties<nbits, 0, capacity>(cout);
				break;
			case 1:
				arithmetic_properties<nbits, 1, capacity>(cout);
				break;
			case 2:
				arithmetic_properties<nbits, 2, capacity>(cout);
				break;
			case 3:
				arithmetic_properties<nbits, 3, capacity>(cout);
				break;
			case 4:
				arithmetic_properties<nbits, 4, capacity>(cout);
				break;
			case 5:
				arithmetic_properties<nbits, 5, capacity>(cout);
				break;
			case 6:
				arithmetic_properties<nbits, 6, capacity>(cout);
				break;
			case 7:
				arithmetic_properties<nbits, 7, capacity>(cout);
				break;
			case 8:
				arithmetic_properties<nbits, 8, capacity>(cout);
				break;
			case 9:
				arithmetic_properties<nbits, 9, capacity>(cout);
				break;
			default:
				cerr << "es = " << es << " reporting is not supported by this program" << endl;
			}
		}
		break;
	default:
		cerr << "nbits = " << nbits << " reporting is not supported by this program" << endl;
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
	using namespace sw::unum;

	if (argc == 1) {
		// print a standard quire size table
		QuireSizeTable<4>(cout); cout << endl;
		QuireSizeTable<8>(cout); cout << endl;
		QuireSizeTable<16>(cout); cout << endl;
		QuireSizeTable<24>(cout); cout << endl;
		QuireSizeTable<32>(cout); cout << endl;
		QuireSizeTable<40>(cout); cout << endl;
		QuireSizeTable<48>(cout); cout << endl;
		QuireSizeTable<56>(cout); cout << endl;
		QuireSizeTable<64>(cout); cout << endl;
		return EXIT_SUCCESS;
	}
	else if (argc != 4) {
		cerr << "Show the arithmetic properties of a posit." << endl;
	    cerr << "Usage: posit_properties [nbits es capacity]" << endl;
		cerr << "Example: posit_properties 16 1 8" << endl;
		cerr <<  msg << endl;
		return EXIT_SUCCESS;  // signal successful completion for ctest
	}

	size_t nbits = atoi(argv[1]);
	size_t es = atoi(argv[2]);
	size_t capacity = atoi(argv[3]);

	switch (capacity) {
	case 0:
		ReportArithmeticProperties<0>(nbits, es);
		break;
	case 4:
		ReportArithmeticProperties<4>(nbits, es);
		break;
	case 8:
		ReportArithmeticProperties<8>(nbits, es);
		break;
	case 10:
		ReportArithmeticProperties<10>(nbits, es);
		break;
	case 16:
		ReportArithmeticProperties<16>(nbits, es);
		break;
	case 20:
		ReportArithmeticProperties<20>(nbits, es);
		break;
	case 24:
		ReportArithmeticProperties<24>(nbits, es);
		break;
	case 32:
		ReportArithmeticProperties<32>(nbits, es);
		break;
	default:
		cerr << "capacity = " << capacity << " reporting is not supported by this program: set of values to select from is [0,4,8,10,16,20,24,32]";
	}
	
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
