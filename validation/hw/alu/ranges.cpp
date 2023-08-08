// ranges.cpp: comparing minpos/maxpos ranges for different small encoding number systems
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <universal/hw/alu.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>

static constexpr unsigned TYPENAMEWIDTH = 80;
static constexpr unsigned COLWIDTH = 15;

void PrintMinposMaxposHeader(std::ostream& ostr, bool tsv) {
	if (tsv) {
		ostr << "number type\tminpos\tmaxpos\n";
	}
	else {
		ostr << std::setw(TYPENAMEWIDTH) << "number type" << std::setw(COLWIDTH) << "minpos" << std::setw(COLWIDTH) << "maxpos" << '\n';
	}
}

template<typename NumberType>
void PrintMinposMaxpos(std::ostream& ostr, bool tsv) {
	using namespace sw::universal;
	if (tsv) {
		ostr << type_tag(NumberType()) << "\t" << NumberType(SpecificValue::minpos) << "\t" << NumberType(SpecificValue::maxpos) << '\n';
	}
	else {
		ostr << std::setw(TYPENAMEWIDTH) << type_tag(NumberType()) << std::setw(COLWIDTH) << NumberType(SpecificValue::minpos) << std::setw(COLWIDTH) << NumberType(SpecificValue::maxpos) << '\n';
	}
}

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	using fp8_3_nosubsup = cfloat<8,3, std::uint8_t, false, false, false>;
	using fp8_4_nosubsup = cfloat<8,4, std::uint8_t, false, false, false>;
	using fp8_5_nosubsup = cfloat<8,5, std::uint8_t, false, false, false>;
	
	using fp8_3_subsup = cfloat<8,3,   std::uint8_t, true, true, false>;
	using fp8_4_subsup = cfloat<8,4,   std::uint8_t, true, true, false>;
	using fp8_5_subsup = cfloat<8,5,   std::uint8_t, true, true, false>;

	using lns8_3 = lns<8,3>;
	using lns8_4 = lns<8,4>;
	using lns8_5 = lns<8,5>;

	using dbns8_3 = dbns<8,3>;
	using dbns8_4 = dbns<8,4>;
	using dbns8_5 = dbns<8,5>;

	using posit8_0 = posit<8,0>;
	using posit8_1 = posit<8,1>;
	using posit8_2 = posit<8,2>;

	bool tsv = false;
	std::string filename("range.tsv");
	if (argc > 2) {
		std::cerr << "Usage: hw_range [tab-separated file name]\n";
		return EXIT_SUCCESS;
	}
	else if (argc == 2) {
		tsv = true;
		filename = std::string(argv[1]);
	}

	if (tsv) {
		// create a tab-sepated values file
		std::ofstream ostr;
		ostr.open(filename);

		PrintMinposMaxposHeader(ostr, tsv);
		PrintMinposMaxpos<fp8_3_nosubsup>(ostr, tsv);
		PrintMinposMaxpos<fp8_4_nosubsup>(ostr, tsv);
		PrintMinposMaxpos<fp8_5_nosubsup>(ostr, tsv);

		PrintMinposMaxpos<fp8_3_subsup>(ostr, tsv);
		PrintMinposMaxpos<fp8_4_subsup>(ostr, tsv);
		PrintMinposMaxpos<fp8_5_subsup>(ostr, tsv);

		PrintMinposMaxpos<lns8_3 > (ostr, tsv);
		PrintMinposMaxpos<lns8_4 > (ostr, tsv);
		PrintMinposMaxpos<lns8_5 > (ostr, tsv);

		PrintMinposMaxpos<dbns8_3 > (ostr, tsv);
		PrintMinposMaxpos<dbns8_4 > (ostr, tsv);
		PrintMinposMaxpos<dbns8_5 > (ostr, tsv);

		PrintMinposMaxpos<posit8_0 > (ostr, tsv);
		PrintMinposMaxpos<posit8_1 > (ostr, tsv);
		PrintMinposMaxpos<posit8_2 > (ostr, tsv);

		ostr << '\n';

		ostr.close();
	}
	else {
		PrintMinposMaxposHeader(std::cout, tsv);
		PrintMinposMaxpos<fp8_3_nosubsup>(std::cout, tsv);
		PrintMinposMaxpos<fp8_4_nosubsup>(std::cout, tsv);
		PrintMinposMaxpos<fp8_5_nosubsup>(std::cout, tsv);

		PrintMinposMaxpos<fp8_3_subsup>(std::cout, tsv);
		PrintMinposMaxpos<fp8_4_subsup>(std::cout, tsv);
		PrintMinposMaxpos<fp8_5_subsup>(std::cout, tsv);

		PrintMinposMaxpos<lns8_3 >(std::cout, tsv);
		PrintMinposMaxpos<lns8_4 >(std::cout, tsv);
		PrintMinposMaxpos<lns8_5 >(std::cout, tsv);

		PrintMinposMaxpos<dbns8_3 >(std::cout, tsv);
		PrintMinposMaxpos<dbns8_4 >(std::cout, tsv);
		PrintMinposMaxpos<dbns8_5 >(std::cout, tsv);

		PrintMinposMaxpos<posit8_0 >(std::cout, tsv);
		PrintMinposMaxpos<posit8_1 >(std::cout, tsv);
		PrintMinposMaxpos<posit8_2 >(std::cout, tsv);

		std::cout << '\n';
	}

	dbns<8, 3> minpos(SpecificValue::minpos), maxpos(SpecificValue::maxpos);
	dbns<8, 3> minneg(SpecificValue::minneg), maxneg(SpecificValue::maxneg);
	std::cout << "dbns<8, 3> maxneg : " << to_binary(maxneg) << " : " << maxneg << '\n';
	std::cout << "dbns<8, 3> minneg : " << to_binary(minneg) << " : " << minneg << '\n';
	std::cout << "dbns<8, 3> minpos : " << to_binary(minpos) << " : " << minpos << '\n';
	std::cout << "dbns<8, 3> maxpos : " << to_binary(maxpos) << " : " << maxpos << '\n';

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
