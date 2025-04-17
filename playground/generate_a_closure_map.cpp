
// x_over_one_minus_x.hpp: generic implementation of the function x / (1 - x)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// Author: Colby Wirth
// 
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/lns/lns.hpp> 
#include <universal/utility/generateClosurePlots.hpp>   // contains buildClosurePlot<ArithmeticType>() function

/// Version 13 April 2025
/// 
/// About:
/// This file allows the user to generate a .txt and .csv file for a closure mapping 
/// associated with a specified Real number system.
///
/// The closure plot can then be built from draw_closure_plots.ipynb 
/// *edits must be made to the last driver function there
///



/// <summary>
/// Generate .txt and .csv files containing the data to construct a closure map
/// for the specified number system.
/// 
/// the generated closure plots can be found in the dir: ./build/mappings/user_generated
/// </summary>
int main() {
    using namespace sw::universal;

    constexpr unsigned NBITS{ 8 };
    constexpr unsigned ES{ 2 };
    using Real = posit<NBITS, ES>;
    std::string type = "posit";

    //handle the name and output directory
    std::string sys_name_str = type + "_" + std::to_string(NBITS) + "_" + std::to_string(ES); 
    std::filesystem::path mappings{"mappings/user_generated/"};
    std::filesystem::create_directories(mappings / sys_name_str); 
    
    //output files
    std::ofstream sys_txt_file(mappings / sys_name_str / (sys_name_str + ".txt")); 
    std::ofstream sys_csv_file(mappings / sys_name_str / (sys_name_str + ".csv"));

    //actual function call
    buildClosurePlot<Real>(sys_name_str, sys_txt_file, sys_csv_file);

    return EXIT_SUCCESS;
}