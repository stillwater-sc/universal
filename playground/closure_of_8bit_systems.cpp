#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/lns/lns.hpp> 

#include <universal/utility/generateClosurePlots.hpp>

/// Version 13 Apirl 2025
///
/// This utility calls the buildClosurePlot function genereateClosurePlots.hpp and generates the closure plots for three 8 bit number systems:
/// cfloat<8,4> , posit<8,0> , lns<8,3>
/// These number systems are all of similar dynamic ranges
/// 
/// the generated closure plots can be found in the dir: build/mappings/user_generated


///This is the configuration the generate the three comparable number systems
namespace sw::universal{

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
}

///main driver function
int main() {
    using namespace sw::universal;


    std::string cfloat_str = "cfloat_" + std::to_string(nbits) + "_" + std::to_string(cfloat_exp); 
    std::string posit_str = "posit_" + std::to_string(nbits) + "_" + std::to_string(posit_exp); 
    std::string lns_str = "lns_" + std::to_string(nbits) + "_" + std::to_string(lns_exp); 

    std::filesystem::path mappings{"mappings/user_generated/"};
    std::filesystem::create_directories(mappings / cfloat_str); 
    std::filesystem::create_directories(mappings / posit_str); 
    std::filesystem::create_directories(mappings / lns_str); 

    std::ofstream cfloat_txt_file(mappings / cfloat_str / (cfloat_str + ".txt")); 
    std::ofstream cfloat_csv_file(mappings / cfloat_str / (cfloat_str + ".csv"));

    std::ofstream posit_txt_file(mappings / posit_str / (posit_str + ".txt")); 
    std::ofstream posit_csv_file(mappings / posit_str / (posit_str + ".csv"));

    std::ofstream lns_txt_file(mappings / lns_str / (lns_str + ".txt")); 
    std::ofstream lns_csv_file(mappings / lns_str / (lns_str + ".csv"));

    buildClosurePlot<RealC>(cfloat_str, cfloat_txt_file, cfloat_csv_file);
    buildClosurePlot<RealP>(posit_str, posit_txt_file, posit_csv_file);
    buildClosurePlot<RealL>(lns_str, lns_txt_file, lns_csv_file);

    return EXIT_SUCCESS;
}