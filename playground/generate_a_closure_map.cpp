
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/lns/lns.hpp> 
#include <universal/utility/generateClosurePlots.hpp>


/// Version 13 April 2025
/// 
/// About:
/// This file allows the user to generate a .txt and .csv file for a closure mapping associated with a specified Real number system.
///
/// The closure plot can then be built from draw_closure_plots.ipynb *edits must be made to the last driver function there
///
///The user must configure the system below.


///////////////////////////////     CONFIGURE Real NUMBER SYSTEM BELOW       ///////////////////////////////
namespace sw::universal{
constexpr unsigned nbits{ 2 };
constexpr unsigned es{ 4 };
using Real= posit<nbits, es>; 
std::string type = "posit";
///////////////////////////////     DONT EDIT BELOE THIS LINE       ///////////////////////////////

template int buildClosurePlot<Real>(std::string, std::ostream&, std::ostream&);
}


/// <summary>
/// This utility calls the buildclosureplot() function of genereateClosurePlots.hpp 
/// 
/// the generated closure plots can be found in the dir: build/mappings/user_generated
/// </summary>
int main() {
    using namespace sw::universal;

    //handle the name and output directory
    std::string sys_name_str = type + "_" + std::to_string(nbits) + "_" + std::to_string(es); 
    std::filesystem::path mappings{"mappings/user_generated/"};
    std::filesystem::create_directories(mappings / sys_name_str); 
    
    //output files
    std::ofstream sys_txt_file(mappings / sys_name_str / (sys_name_str + ".txt")); 
    std::ofstream sys_csv_file(mappings / sys_name_str / (sys_name_str + ".csv"));

    //actual function call
    buildClosurePlot<Real>(sys_name_str, sys_txt_file, sys_csv_file);

    return EXIT_SUCCESS;
}