// propp.cpp: cli to show the arithmetic properties of posit configurations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/number/posit/posit.hpp>
#include <universal/number/posit/quire.hpp>

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
	sw::universal::posit<nbits, es> p;
	ostr << sw::universal::posit_range<nbits, es>() << std::endl;
	p.minpos();
	ostr << "  minpos                     : " << sw::universal::hex_format(p) << " " << p << std::endl;
	p.maxpos();
	ostr << "  maxpos                     : " << sw::universal::hex_format(p) << " " << p << std::endl;
	ostr << sw::universal::quire_properties<nbits, es, capacity>() << std::endl;
	ostr << "Quire segments" << std::endl;
	ostr << sw::universal::quire<nbits, es, capacity>() << std::endl;
}

// transformation of user-provided values to constexpr values
template<size_t capacity>
void ReportArithmeticProperties(size_t nbits, size_t es) {
	using namespace sw::universal;

	std::cout << "arithmetic properties of a posit<" << nbits << ", " << es << "> environment\n";

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
		std::cerr << "nbits = 1 implies just a sign bit\n";
		break;
	case 8:
	{
		constexpr size_t nbits = 8;
		switch (es) {
		case 0:
			arithmetic_properties<nbits, 0, capacity>(std::cout);
			break;
		case 1:
			arithmetic_properties<nbits, 1, capacity>(std::cout);
			break;
		case 2:
			arithmetic_properties<nbits, 2, capacity>(std::cout);
			break;
		case 3:
			arithmetic_properties<nbits, 3, capacity>(std::cout);
			break;
		case 4:
			arithmetic_properties<nbits, 4, capacity>(std::cout);
			break;
		case 5:
			arithmetic_properties<nbits, 5, capacity>(std::cout);
			break;
		case 6:
			arithmetic_properties<nbits, 6, capacity>(std::cout);
			break;
		case 7:
			arithmetic_properties<nbits, 7, capacity>(std::cout);
			break;
		case 8:
			arithmetic_properties<nbits, 8, capacity>(std::cout);
			break;
		case 9:
			arithmetic_properties<nbits, 9, capacity>(std::cout);
			break;
		default:
			std::cerr << "es = " << es << " reporting is not supported by this program\n";
		}
	}
		break;
	case 16:
	{
		constexpr size_t nbits = 16;
		switch (es) {
		case 0:
			arithmetic_properties<nbits, 0, capacity>(std::cout);
			break;
		case 1:
			arithmetic_properties<nbits, 1, capacity>(std::cout);
			break;
		case 2:
			arithmetic_properties<nbits, 2, capacity>(std::cout);
			break;
		case 3:
			arithmetic_properties<nbits, 3, capacity>(std::cout);
			break;
		case 4:
			arithmetic_properties<nbits, 4, capacity>(std::cout);
			break;
		case 5:
			arithmetic_properties<nbits, 5, capacity>(std::cout);
			break;
		case 6:
			arithmetic_properties<nbits, 6, capacity>(std::cout);
			break;
		case 7:
			arithmetic_properties<nbits, 7, capacity>(std::cout);
			break;
		case 8:
			arithmetic_properties<nbits, 8, capacity>(std::cout);
			break;
		case 9:
			arithmetic_properties<nbits, 9, capacity>(std::cout);
			break;
		default:
			std::cerr << "es = " << es << " reporting is not supported by this program\n";
		}
	}
		break;
	case 31:
	{
		constexpr size_t nbits = 31;
		switch (es) {
		case 0:
			arithmetic_properties<nbits, 0, capacity>(std::cout);
			break;
		case 1:
			arithmetic_properties<nbits, 1, capacity>(std::cout);
			break;
		case 2:
			arithmetic_properties<nbits, 2, capacity>(std::cout);
			break;
		case 3:
			arithmetic_properties<nbits, 3, capacity>(std::cout);
			break;
		case 4:
			arithmetic_properties<nbits, 4, capacity>(std::cout);
			break;
		case 5:
			arithmetic_properties<nbits, 5, capacity>(std::cout);
			break;
		case 6:
			arithmetic_properties<nbits, 6, capacity>(std::cout);
			break;
		case 7:
			arithmetic_properties<nbits, 7, capacity>(std::cout);
			break;
		case 8:
			arithmetic_properties<nbits, 8, capacity>(std::cout);
			break;
		case 9:
			arithmetic_properties<nbits, 9, capacity>(std::cout);
			break;
		default:
			std::cerr << "es = " << es << " reporting is not supported by this program\n";
		}
	}
		break;
	case 32:
	{
		constexpr size_t nbits = 32;
		switch (es) {
		case 0:
			arithmetic_properties<nbits, 0, capacity>(std::cout);
			break;
		case 1:
			arithmetic_properties<nbits, 1, capacity>(std::cout);
			break;
		case 2:
			arithmetic_properties<nbits, 2, capacity>(std::cout);
			break;
		case 3:
			arithmetic_properties<nbits, 3, capacity>(std::cout);
			break;
		case 4:
			arithmetic_properties<nbits, 4, capacity>(std::cout);
			break;
		case 5:
			arithmetic_properties<nbits, 5, capacity>(std::cout);
			break;
		case 6:
			arithmetic_properties<nbits, 6, capacity>(std::cout);
			break;
		case 7:
			arithmetic_properties<nbits, 7, capacity>(std::cout);
			break;
		case 8:
			arithmetic_properties<nbits, 8, capacity>(std::cout);
			break;
		case 9:
			arithmetic_properties<nbits, 9, capacity>(std::cout);
			break;
		default:
			std::cerr << "es = " << es << " reporting is not supported by this program\n";
		}
	}
		break;
	case 64:
		{
			constexpr size_t nbits = 64;
			switch (es) {
			case 0:
				arithmetic_properties<nbits, 0, capacity>(std::cout);
				break;
			case 1:
				arithmetic_properties<nbits, 1, capacity>(std::cout);
				break;
			case 2:
				arithmetic_properties<nbits, 2, capacity>(std::cout);
				break;
			case 3:
				arithmetic_properties<nbits, 3, capacity>(std::cout);
				break;
			case 4:
				arithmetic_properties<nbits, 4, capacity>(std::cout);
				break;
			case 5:
				arithmetic_properties<nbits, 5, capacity>(std::cout);
				break;
			case 6:
				arithmetic_properties<nbits, 6, capacity>(std::cout);
				break;
			case 7:
				arithmetic_properties<nbits, 7, capacity>(std::cout);
				break;
			case 8:
				arithmetic_properties<nbits, 8, capacity>(std::cout);
				break;
			case 9:
				arithmetic_properties<nbits, 9, capacity>(std::cout);
				break;
			default:
				std::cerr << "es = " << es << " reporting is not supported by this program\n";
			}
		}
		break;
	case 128:
	{
		constexpr size_t nbits = 128;
		switch (es) {
		case 0:
			arithmetic_properties<nbits, 0, capacity>(std::cout);
			break;
		case 1:
			arithmetic_properties<nbits, 1, capacity>(std::cout);
			break;
		case 2:
			arithmetic_properties<nbits, 2, capacity>(std::cout);
			break;
		case 3:
			arithmetic_properties<nbits, 3, capacity>(std::cout);
			break;
		case 4:
			arithmetic_properties<nbits, 4, capacity>(std::cout);
			break;
		case 5:
			arithmetic_properties<nbits, 5, capacity>(std::cout);
			break;
		case 6:
			arithmetic_properties<nbits, 6, capacity>(std::cout);
			break;
		case 7:
			arithmetic_properties<nbits, 7, capacity>(std::cout);
			break;
		case 8:
			arithmetic_properties<nbits, 8, capacity>(std::cout);
			break;
		case 9:
			arithmetic_properties<nbits, 9, capacity>(std::cout);
			break;
		default:
			std::cerr << "es = " << es << " reporting is not supported by this program\n";
		}
	}
		break;
	case 256:
	{
		constexpr size_t nbits = 256;
		switch (es) {
		case 0:
			arithmetic_properties<nbits, 0, capacity>(std::cout);
			break;
		case 1:
			arithmetic_properties<nbits, 1, capacity>(std::cout);
			break;
		case 2:
			arithmetic_properties<nbits, 2, capacity>(std::cout);
			break;
		case 3:
			arithmetic_properties<nbits, 3, capacity>(std::cout);
			break;
		case 4:
			arithmetic_properties<nbits, 4, capacity>(std::cout);
			break;
		case 5:
			arithmetic_properties<nbits, 5, capacity>(std::cout);
			break;
		case 6:
			arithmetic_properties<nbits, 6, capacity>(std::cout);
			break;
		case 7:
			arithmetic_properties<nbits, 7, capacity>(std::cout);
			break;
		case 8:
			arithmetic_properties<nbits, 8, capacity>(std::cout);
			break;
		case 9:
			arithmetic_properties<nbits, 9, capacity>(std::cout);
			break;
		default:
			std::cerr << "es = " << es << " reporting is not supported by this program\n";
		}
	}
	break;
	default:
		std::cerr << "nbits = " << nbits << " reporting is not supported by this program\n";
	}
}


// receive a float and print its components
int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc == 4) std::cout << argv[0] << ": posit properties\n";
	if (argc != 4) {
		std::cerr << "Show the arithmetic properties of a posit.\n";
		std::cerr << "Usage: propp [nbits es capacity]\n";
		std::cerr << "Example: propp 16 1 8\n";
		std::cerr <<  msg << '\n';
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
		std::cerr << "capacity = " << capacity << " reporting is not supported by this program: set of values to select from is [0,4,8,10,16,20,24,32]\n";
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
