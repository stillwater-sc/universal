// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// Author: Colby Wirth
// Version: 18 April 2025
// 
// About:
// This program utilizes the evaluate_closure_of_number_sysetm utility via  processASystem() function call
// and generates the closure statistics for small bit configures of cfloats, postis and lns
//
// ******************
// ***HOW TO USE: ***
// ******************
//
// Run the program with any of the following flags, or none to default to 8 bit systems:
// -4 :  4 bit configurations: cfloat<4,2> , posit<4,0> , lns<4,1>
// -6 :  bit configurations: cfloat<6,4> , posit<6,0> , lns<6,1>
// -8 : bit configurations: cfloat<8,4> , posit<8,0> , lns<8,3>
// -10 : bit configurations: cfloat<10,6> , posit<10,2> , lns<10,3>
// -12 : bit configurations: cfloat<12,7> , posit<12,3> , lns<12,3>
//
// 
// **************************************************************************
// *** NOTE: 10 bit and 12 bit number systems may take minutes to compute ***
// **************************************************************************
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <string>
#include <string_view>
#include <fstream>
#include <filesystem>

#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/lns/lns.hpp>


#include <universal/utility/evaluate_closure_of_number_systems.hpp>
#include <universal/utility/generateClosurePlots.hpp>

using namespace sw::universal;

// Config structs to hold the number systems - they are dynamically selected by the user via the flags
struct cfloatConfigs {
    using cfg4_t  = cfloat<4,2,uint8_t,true,false,false>;
    using cfg6_t  = cfloat<6,4,uint8_t,true,false,false>;
    using cfg8_t  = cfloat<8,4,uint8_t,true,false,false>;
    using cfg10_t = cfloat<10,6,uint8_t,true,false,false>;
    using cfg12_t = cfloat<12,7,uint8_t,true,false,false>;

    static inline const std::string cfg4_str  = "cfloat_4_2";
    static inline const std::string cfg6_str  = "cfloat_6_4";
    static inline const std::string cfg8_str  = "cfloat_8_4";
    static inline const std::string cfg10_str = "cfloat_10_6";
    static inline const std::string cfg12_str = "cfloat_12_7";
};

struct positConfigs {
    using cfg4_t  = posit<4,0>;
    using cfg6_t  = posit<6,0>;
    using cfg8_t  = posit<8,0>;
    using cfg10_t = posit<10,2>;
    using cfg12_t = posit<12,3>;

    static inline const std::string cfg4_str  = "posit_4_0";
    static inline const std::string cfg6_str  = "posit_6_0";
    static inline const std::string cfg8_str  = "posit_8_0";
    static inline const std::string cfg10_str = "posit_10_2";
    static inline const std::string cfg12_str = "posit_12_3";
};

struct lnsConfigs {
    using cfg4_t  = lns<4,1>;
    using cfg6_t  = lns<6,1>;
    using cfg8_t  = lns<8,3>;
    using cfg10_t = lns<10,3>;
    using cfg12_t = lns<12,3>;

    static inline const std::string cfg4_str  = "lns_4_1";
    static inline const std::string cfg6_str  = "lns_6_1";
    static inline const std::string cfg8_str  = "lns_8_3";
    static inline const std::string cfg10_str = "lns_10_3";
    static inline const std::string cfg12_str = "lns_12_3";

};


    int main(int argc, char *argv[]) {
        using namespace std;

        bool _4bit = false;
        bool _6bit = false;
        bool _8bit = false;
        bool _10bit = false;
        bool _12bit = false;

        if(argc >= 2){
            // Loop through each argument
            for (int i = 1; i < argc; ++i) {
                if (string(argv[i]) == "-4")
                    _4bit = true;
                else if (string(argv[i]) == "-6")
                    _6bit = true;
                else if (string(argv[i]) == "-8")
                    _8bit = true;
                else if (string(argv[i]) == "-10")
                    _10bit = true;
                else if (string(argv[i]) == "-12")
                    _12bit = true;
                else{
                    cerr << "\nInvalid flag detected.  Enter one or many flags of the form:\n'-4' for 4 bit systems\n'-6' for 6 bit systems\n'-8' for 8 bit systems\n'-10' for 10 bit systems\n'-12' for 12 bit systems"<< endl;
                    return EXIT_FAILURE;
                }
            }
        }
        else //default to only 8 bit systems
            _8bit = true;


        //process the systems in sequential order
        if(_4bit){
            cout << "Processing 4 bit systems:\n\n" ;
            processASystem<cfloatConfigs::cfg4_t>(cfloatConfigs::cfg4_str);
            processASystem<positConfigs::cfg4_t>(positConfigs::cfg4_str);
            processASystem<lnsConfigs::cfg4_t>(lnsConfigs::cfg4_str);
            cout << "\n";
        }

        if(_6bit){
            cout << "Processing 6 bit systems:\n\n" ;
            processASystem<cfloatConfigs::cfg6_t>(cfloatConfigs::cfg6_str);
            processASystem<positConfigs::cfg6_t>(positConfigs::cfg6_str);
            processASystem<lnsConfigs::cfg6_t>(lnsConfigs::cfg6_str);
            cout << "\n";
        }

        if(_8bit){
            cout << "Processing 8 bit systems:\n\n" ;
            processASystem<cfloatConfigs::cfg8_t>(cfloatConfigs::cfg8_str);
            processASystem<positConfigs::cfg8_t>(positConfigs::cfg8_str);
            processASystem<lnsConfigs::cfg8_t>(lnsConfigs::cfg8_str);
            cout << "\n";
        }


        if(_10bit){
            cout << "Processing 10 bit systems:\n\n" ;
            processASystem<cfloatConfigs::cfg10_t>(cfloatConfigs::cfg10_str);
            processASystem<positConfigs::cfg10_t>(positConfigs::cfg10_str);
            processASystem<lnsConfigs::cfg10_t>(lnsConfigs::cfg10_str);
            cout << "\n";
        }

        if(_12bit){
            cout << "Processing 12 bit systems:\n\n" ;
            processASystem<cfloatConfigs::cfg12_t>(cfloatConfigs::cfg12_str);
            processASystem<positConfigs::cfg12_t>(positConfigs::cfg12_str);
            processASystem<lnsConfigs::cfg12_t>(lnsConfigs::cfg12_str);
            cout << "\n";
        }
    return EXIT_SUCCESS;
}