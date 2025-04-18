#pragma once

// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// Author: Colby Wirth
// Version: 18 April 2025
// 
// About:
// This utility calls the buildClosurePlot function from genereateClosurePlots.hpp 
// and generates the closure statistics for a templatized number system. 
//
//
// ****************************************************************
// *** Use 'processASystem()' function call to use this utility ***
// ****************************************************************
// 
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <string>
#include <string_view>
#include <fstream>

#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/utility/generateClosurePlots.hpp>


// the data for each operation in a buildClosurePlots is stored in this struct
struct operationResults{
    float TotalOps;
    std::string Exact;
    std::string Approximate;
    std::string Overflow;
    std::string Underflow;
    std::string Saturate;
    std::string narOrNan;
};

// a storeResults struct stores the operationResult structs for a number system
struct storeResults{
    std::string sys_name ;
    operationResults addition;
    operationResults subtraction;
    operationResults multiplication;
    operationResults division;
};

///<summary>
/// helper function for processASystem()
/// formats and prints the results
/// /</summary>
///
///<param> name="results" a struct that stores all of the statistics of the closure plot for a number system </param>
/// <returns> void </returns>
/// 
void printResults(const storeResults& results) {
    constexpr unsigned data_width = 15;

    std::cout << results.sys_name << "\n";
    std::cout << std::setw(data_width) << " " << std::setw(data_width) << "         Total_Ops" << std::setw(data_width) << "Exact"
              << std::setw(data_width) << "Approximate" << std::setw(data_width) << "Overflow" << std::setw(data_width) << "Underflow"
              << std::setw(data_width) << "Saturate" << std::setw(data_width) << "NAR/NAN" << "\n";
    std::cout << "addition        : " << std::setw(data_width) << results.addition.TotalOps << std::setw(data_width) << results.addition.Exact
              << std::setw(data_width) << results.addition.Approximate << std::setw(data_width) << results.addition.Overflow
              << std::setw(data_width) << results.addition.Underflow << std::setw(data_width) << results.addition.Saturate
              << std::setw(data_width) << results.addition.narOrNan << "\n";
    std::cout << "subtraction     : " << std::setw(data_width) << results.subtraction.TotalOps << std::setw(data_width) << results.subtraction.Exact
              << std::setw(data_width) << results.subtraction.Approximate << std::setw(data_width) << results.subtraction.Overflow
              << std::setw(data_width) << results.subtraction.Underflow << std::setw(data_width) << results.subtraction.Saturate
              << std::setw(data_width) << results.subtraction.narOrNan << "\n";
    std::cout << "multiplication  : " << std::setw(data_width) << results.multiplication.TotalOps << std::setw(data_width) << results.multiplication.Exact
              << std::setw(data_width) << results.multiplication.Approximate << std::setw(data_width) << results.multiplication.Overflow
              << std::setw(data_width) << results.multiplication.Underflow << std::setw(data_width) << results.multiplication.Saturate
              << std::setw(data_width) << results.multiplication.narOrNan << "\n";
    std::cout << "division        : " << std::setw(data_width) << results.division.TotalOps << std::setw(data_width) << results.division.Exact
              << std::setw(data_width) << results.division.Approximate << std::setw(data_width) << results.division.Overflow
              << std::setw(data_width) << results.division.Underflow << std::setw(data_width) << results.division.Saturate
              << std::setw(data_width) << results.division.narOrNan << "\n\n\n";
}



///<summary>
/// helper function for getDataFromBufferStream()
/// coverts calculated from buildClosurePlot() to a percentage wrt the totalOps
/// /</summary>
///
///<param> name="value" the current value to be converted to a percentage </param>
///<param> name="totalOps" the total amount of operations performed for the current arithmetic operation </param>
/// <returns> string , the string representation of the percentage </returns>
/// 
std::string toPercentageString(float value, float totalOps) {
    if (totalOps == 0) 
        return "0.00%";
        
    float percentage = (value / totalOps) * 100.0f;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << percentage << "%";

    return oss.str();
}


///<summary>
/// helper function for processASystem()
/// processes the data from buildClosurePlot() line by line
/// /</summary>
///
///<param> name="results" a struct that stores all of the statistics of the closure plot for a number system </param>
///<param> name="bufferStream" the stream that holds the data from stdout from buildClosurePlot() </param>
///<param> name="oldBuf" the old stdout stream buffer </param>
///
/// <returns> int 0 if successful, 1 if errir wuth parsing data from the bufferStream </returns>
/// 
int getDataFromBufferStream(storeResults& results, std::ostringstream& bufferStream, std::streambuf* oldBuf){

    std::string operandName;

    std::istringstream lineStream(bufferStream.str()); //pass the data to a stream
    std::string line;

    while (std::getline(lineStream, line)) { // iterate to the beginning of the data
            if (line.find(results.sys_name) != std::string::npos) {
            break;
        }
    }

    // maps the strings in the input to the proper values in operationStructs 
    std::map<std::string, operationResults*> opMap = {
        {"addition", &results.addition},
        {"subtraction", &results.subtraction},
        {"multiplication", &results.multiplication},
        {"division", &results.division}
    };

    while (std::getline(lineStream, line)) {
        if (line.empty()) continue;

        std::istringstream tokenStream(line);
        tokenStream >> operandName;

        if (opMap.find(operandName) != opMap.end()) {
            operationResults* op = opMap[operandName];
            std::string temp;
            tokenStream >> temp;  // Skip the ":"

            // Temporary variables to hold the numeric values
            float exactValue, approximateValue, overflowValue, underflowValue, saturateValue, narOrNanValue;
            if (!(tokenStream >> op->TotalOps >> exactValue >> approximateValue
                           >> overflowValue >> underflowValue >> saturateValue
                           >> narOrNanValue)) {
                std::cerr << "Error parsing data for " << operandName << " in " << results.sys_name << std::endl;
                std::cout.rdbuf(oldBuf);
                return 1;
            }

            // Convert all values to percentage strings
            op->Exact = toPercentageString(exactValue, op->TotalOps);
            op->Approximate = toPercentageString(approximateValue, op->TotalOps);
            op->Overflow = toPercentageString(overflowValue, op->TotalOps);
            op->Underflow = toPercentageString(underflowValue, op->TotalOps);
            op->Saturate = toPercentageString(saturateValue, op->TotalOps);
            op->narOrNan = toPercentageString(narOrNanValue, op->TotalOps);

        }
    }
    return 0;
}


/// <summary> 
/// * driver function, performs the following operations:
/// 1. calls buildClosurePlot to calculate closure plot values for a templatized number system
/// 2. puts the data to a buffer and converts processes it into structs
/// 3. prints the processed data to stdout
/// </summary>
/// <template> name="NumberType" a generic number system compatible with this libray  </template>
/// <param> name="sys_name" the string represntation of the Number System </param>
/// <return> 0 on success, 1 on failure </return>
template<typename NumberType>
int processASystem(std::string sys_name){

    storeResults results; // store the data
    results.sys_name = sys_name;

    std::ostringstream nullStream; // nullstream to not print files to a csv or txt
    std::streambuf* oldBuf = std::cout.rdbuf(); // redirect stdout to a buffer to process data.
    std::ostringstream bufferStream;
    std::cout.rdbuf(bufferStream.rdbuf());

    sw::universal::buildClosurePlot<NumberType>(sys_name, nullStream, nullStream); // get the data
    if (getDataFromBufferStream(results, bufferStream, oldBuf) != 0){ //process the data
        return 1;
    }    
    std::cout.rdbuf(oldBuf); //restore buffer
    printResults(results); // print the data

    return 0;
}