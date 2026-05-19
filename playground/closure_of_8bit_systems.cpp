// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// Author: Colby Wirth
//
// Version 13 April 2025
//
// This utility calls the buildClosurePlot function genereateClosurePlots.hpp and generates the closure plots for three 8 bit number systems:
// cfloat<8,4> , posit<8,0> , lns<8,3>
// These number systems are all of similar dynamic ranges
// 
// the generated closure plots can be found in the dir: build/mappings/user_generated

#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/lns/lns.hpp> 

#include <universal/utility/generateClosurePlots.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

///This is the configuration the generate the three comparable number systems
namespace sw::universal {

constexpr unsigned nbits{ 8 };  // size in bits of the encoding
constexpr unsigned cfloat_exp{ 4 };
constexpr unsigned posit_exp{ 0 }; 
constexpr unsigned lns_exp{ 3 }; 

using RealC = cfloat<nbits, cfloat_exp, uint8_t, true, false, false>; // range of ~[2^-9, 2^8]
using RealP = posit<nbits, posit_exp>; // range of ~[2^-6 , 2^6]
using RealL = lns<nbits, lns_exp>;  // range of ~[2^-8 , 2^8]

template int buildClosurePlot<RealC>(std::string, std::ostream&, std::ostream&);
template int buildClosurePlot<RealP>(std::string, std::ostream&, std::ostream&);
template int buildClosurePlot<RealL>(std::string, std::ostream&, std::ostream&);

template<typename TestType>
void generateClosurePlots(const std::string& ns_name, unsigned param2) {
	constexpr unsigned    nbits  = TestType::nbits;
	std::string           ns_str = ns_name + "_" + std::to_string(nbits) + "_" + std::to_string(param2);
	std::filesystem::path mappings{"mappings/user_generated/"};
	std::filesystem::create_directories(mappings / ns_str);

	std::ofstream txt_file(mappings / ns_str / (ns_str + ".txt"));
	std::ofstream csv_file(mappings / ns_str / (ns_str + ".csv"));

	buildClosurePlot<TestType>(ns_str, txt_file, csv_file);
}

}

#define MANUAL_TESTING 0

int main() {
	using namespace sw::universal;

#if MANUAL_TESTING
	std::cout << "Manual testing mode: only generating closure plots for 8 bit systems\n";

	// TODO: the whole stack is currently hardcoded to 8-bit systems.
    generateClosurePlots<cfloat<8, 4>>("cfloat", 4u);
	generateClosurePlots<posit<8, 0>>("posit", 0u);
    generateClosurePlots<lns<8, 3>>("lns", 3u);
	
#else
	std::cout << "Regression testing mode: NOP\n";

#endif

    return EXIT_SUCCESS;
}


