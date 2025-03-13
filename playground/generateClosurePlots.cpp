/**
 * 
 * Function used to build closure plots for a given number system
 * 
 * HOW TO USE - configure the first few lines of main() with your desired BITS, EXP number system, and the bool CONTAIN_NAR values
 * 
 * Version : 12 Mar 2025
 * 
 */

#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/math/functions/arithmeticoperations.hpp>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <cstdint>
#include <stdbool.h>

// Declarations
template<typename NumberType>
int vectorInitializerAllValues(std::vector<NumberType>& valArray, bool containNar);

template<typename NumberType>
int systemEvaluator(std::string system, std::string masterfile_string, 
                    std::string outfile_string, std::string csv_outfile_string, std::vector<NumberType>& valArray, std::vector<std::uint64_t>& results, bool contain_nars);

int appendResultsToMasterFile(std::string numberSystem, std::ofstream& masterFile, std::vector<std::uint64_t>& results);

template<typename NumberType, char Op>
int buildClosuePlot(std::string system, std::vector<NumberType>& values, std::vector<std::uint64_t>& results,
                    std::ofstream& outFile, std::ofstream& csv_outFile, int indexOfOutput, 
                    const sw::universal::OperationStruc<NumberType, Op>& operation);

/**
 * 
 * Main function drives systemEvaluator
 * 
 * There are three outputs from the program: All outputs are located in build/mappings
 *  1. A human readable mapping at readable_mappings/NumberSystem<BITS, EXP>.txt
 *  2. A csv file used for data visualization at csv_mappings/NumberSystem<BITS, EXP>.txt
 *  3. Appends MasterMappings.txt with the aggregated points on all operations performed
 * 
 */
int main() {
    constexpr unsigned BITS {8};
    constexpr unsigned EXP {2};
    using myPosit = sw::universal::posit<BITS, EXP>;
    
    bool CONTAIN_NAR = false; //contain NAR operators? ie. a=nar OR real, b=nar OR real, a+b=c
    int SIZE = (1 << BITS);
    // int SIZE = (1 << BITS) / 2; //divide by 2 only when ommiting negatives

    

    std::string system = "Posit<" + std::to_string(BITS ) + ", " + std::to_string(EXP) + ">"; // of the form Posit<bits, exp>
    std::string MASTERFILE = "mappings/MasterMappings.txt";
    std::string OUTFILE = "mappings/readable_mappings/" + system + ".txt";
    std::string CSV_OUTFILE = "mappings/csv_mappings/" + system + ".csv";


    std::vector<myPosit> valArray(SIZE); //stores all values in the numSys
    std::vector<std::uint64_t> results(16); // Size 16, 4 entries per operation, +,-,*,/

    systemEvaluator(system, MASTERFILE, OUTFILE, CSV_OUTFILE, valArray, results, CONTAIN_NAR);

    return 0;
}

/**
 * 
 *  Driver fubbction for a system evaluation:
 * 
 *          All files are opened, 
 *          The valArray is initializes with all values in a system
 *          For each arithmeic operation (+,-,*,/) the system is evaluated
 *          A csv and txt file are created for the system (this is handled within buildClosurePlots())
 *          The aggregated results are appended to the master file
 */
template<typename NumberType>
int systemEvaluator(std::string system, std::string masterfile_string, 
                    std::string outfile_string, std::string csv_outfile_string,  std::vector<NumberType>& valArray, std::vector<std::uint64_t>& results, bool contain_nars) {

    std::ofstream masterfile(masterfile_string, std::ios::app);
    if (!masterfile.is_open()) {
        std::cerr << "Error: Could not open master file " << masterfile_string << " for writing.\n";
        return 1;
    }

    std::ofstream outFile(outfile_string, std::ios::out | std::ios::trunc);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open outfile " << outfile_string << " for writing.\n";
        return 1;
    }

    std::ofstream csv_outFile(csv_outfile_string, std::ios::out | std::ios::trunc);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open csv_outfile " << outfile_string << " for writing.\n";
        return 1;
    }

    vectorInitializerAllValues<NumberType>(valArray, contain_nars); // populate the vector w/ wall numbers in a system.

    csv_outFile << std::left 
    << system << ",,,,,\n"
    << "Result" << ","
    << "Value 1" << ","
    << "Operand"  << ","
    << "Value 2"  << ","
    << "Output"  << ","
    << "Float(64) Value"  <<"\n";

    buildClosuePlot<NumberType>(system, valArray, results, outFile, csv_outFile,  0, sw::universal::OperationStruc<NumberType, '+'>{});
    buildClosuePlot<NumberType>(system, valArray, results, outFile, csv_outFile, 5, sw::universal::OperationStruc<NumberType, '-'>{});
    buildClosuePlot<NumberType>(system, valArray, results, outFile, csv_outFile, 10, sw::universal::OperationStruc<NumberType, '*'>{});
    buildClosuePlot<NumberType>(system, valArray, results, outFile, csv_outFile, 15, sw::universal::OperationStruc<NumberType, '/'>{});

    appendResultsToMasterFile(system, masterfile, results); //append the results to the master file
    
    csv_outFile.close();
    outFile.close();
    masterfile.close();
    return 0;
}

/**
 * Build a vector  with all values
 * @param valArray the array of values to be built, of type NumberType
 * @param contain_nars if true, constructs the vector with nars, else ALL nar values are ommited
 * @return 0
 */
template<typename NumberType>
int vectorInitializerAllValues(std::vector<NumberType>& valArray, bool contain_nars){

    constexpr unsigned nbits = NumberType::nbits;
    const unsigned NR_POSITS = (unsigned(1) << nbits); //the total number of representable bit configurations in the number system
    NumberType nar = sw::universal::SpecificValue::nar;

    NumberType val;

    valArray.clear(); // Ensure we start with an empty vector

    for (unsigned i = 0; i < NR_POSITS; i++) {
        
        val.setbits(i);
        if (!contain_nars && val == nar) continue; // Skip NARs if the flag is set
        valArray.push_back(val); // Always use push_back to dynamically store values
    }

    return 0;
}

/**
 * Handles the logic for building a closure plot for a number system
 * @param system the string representation of the system
 * @param values the vector containing all values
 * @param results the vector the contains aggregated results of the operations
 * @param outFile the .txt outfile
 * @param csv_outFile the .csv outfile
 * @param indexOfOutput the index of results to add the results to ***hard coded in the function call
 * 
 * @return 0 
 * 
 */
template<typename NumberType, char Op>
int buildClosuePlot(std::string system, std::vector<NumberType>& values, std::vector<std::uint64_t>& results, 
                    std::ofstream& outFile, std::ofstream& csv_outFile, int indexOfOutput, 
                    const sw::universal::OperationStruc<NumberType, Op>& operation){

    char operationChar = operation.getOperationChar();
    std::string operationString = std::string(operation.getOperationString());

    constexpr u_char setw = 32;
    outFile << "Generate " <<  operationString <<" table\n";
    outFile << std::left 
            << std::setw(setw) << "Result"
            << std::setw(setw) << "Value 1"
            << std::setw(setw) << "Operand" 
            << std::setw(setw) << "Value 2" 
            << std::setw(setw) << "Output" 
            << std::setw(setw) << "Float(64) Value" 
            << "\n";

    csv_outFile << "Generate '" <<  operation.getOperationChar() <<"' table:,,,,,\n";

    long vectorSize = values.size();
    long narCount{0}, correctCount{0}, approximationCount{0}, incorrectNonNarCount{0};
    long totalOperations = vectorSize * vectorSize;
    // long totalOperations = (vectorSize * (vectorSize + 1)) / 2; only use when calculating unique pairs
    

    for (int i = 0; i < vectorSize; ++i) {
        for (int j = 0; j < vectorSize; ++j) { // change to j = i when calculating uniquie pairs
                    
            NumberType nar = sw::universal::SpecificValue::nar;
            NumberType va = values[i]; 
            NumberType vb = values[j];
            NumberType vc = operation.executeOperation(va, vb);

            sw::universal::OperationStruc<double, Op> dblOp; //used only for the .txt file. can be deleted for optimization
            double targetVal = dblOp.executeOperation(static_cast<double>(va), static_cast<double>(vb));


            std::string result = "";

            if (vc == nar) {
                ++narCount;
                result = "NAR";
            } 

            else {
                double vcDouble = static_cast<double>(vc);
                
                if (vcDouble == targetVal) {
                    ++correctCount;
                    result = "Correct";
                }
                else if (std::abs(vcDouble - targetVal) <= 1e-6) {
                    ++approximationCount;
                    result = "Approximation";
                }
                else {
                    ++incorrectNonNarCount;
                    result = "Incorrect Non-NAR";
                }
            }

        outFile << std::left 
        << std::setw(setw) << result
        << std::setw(setw) << va
        << std::setw(setw) << operationChar
        << std::setw(setw) << vb
        << std::setw(setw) << vc 
        << std::setw(setw) << targetVal 
        << "\n";

        csv_outFile << std::left 
        << result << ","
        << va << ","
        << operationChar << ","
        << vb << ","
        << vc  << ","
        << targetVal
        << "\n";

        }
    }
             
    
    results[indexOfOutput] = totalOperations;
    results[indexOfOutput+1] = correctCount;
    results[indexOfOutput+2] = incorrectNonNarCount;
    results[indexOfOutput+3] = approximationCount;
    results[indexOfOutput+4] = narCount;
    
    outFile << "\nTotal " << operationString << "s: " << totalOperations << "\n";
    outFile << "Total correct " << operationString << "s: " << correctCount << "\n";
    outFile << "Total non-nar incorrect " << operationString << "s: " << incorrectNonNarCount << "\n";
    outFile << "Total approximation " << operationString << "s: " << approximationCount << "\n";
    outFile << "Total nar " << operationString << "s: " << narCount << "\n\n\n";

    return 0;         
}

/**
 * Appends the aggregated to the running master file
 * @param numberSystem the string representation of the system
 * @param masterFile the output file of the aggregated data
 * @param results the vector that contains the aggregated data
 * 
 */
int appendResultsToMasterFile(std::string numberSystem, std::ofstream& masterFile, std::vector<std::uint64_t>& results){

    // Define column widths based on longest terms
    const int col1_width = 15; // "Multiplication:" (15 chars)
    const int col2_width = 9;  // "Total Ops" (9 chars)
    const int col3_width = 7;  // "Correct" (7 chars)
    const int col4_width = 17; // "Incorrect Non-NaR" (17 chars)
    const int col5_width = 14; // "Approximations" (14 chars)
    const int col6_width = 3;  // "NaR" (3 chars)
    const std::string spacer = std::string(16, ' ');

    // Header row
    masterFile << std::left  << std::setw(col1_width) << numberSystem << spacer
        << std::right << std::setw(col2_width) << "Total Ops" << spacer
        << std::setw(col3_width) << "Correct" << spacer
        << std::setw(col4_width) << "Incorrect non-nar" << spacer
        << std::setw(col5_width) << "Approximations" << spacer
        << std::setw(col6_width) << " nar" << "\n";

    // Data rows
    masterFile << std::left  << std::setw(col1_width) << "Addition:" << spacer
        << std::right << std::setw(col2_width) << results[0] << spacer
        << std::setw(col3_width) << results[1] << spacer
        << std::setw(col4_width) << results[2] << spacer
        << std::setw(col5_width) << results[3] << spacer // Approximations
        << std::setw(col6_width) << results[4] << "\n"; // NaR

    masterFile << std::left  << std::setw(col1_width) << "Subtraction:" << spacer
    << std::right << std::setw(col2_width) << results[5] << spacer
    << std::setw(col3_width) << results[6] << spacer
    << std::setw(col4_width) << results[7] << spacer
    << std::setw(col5_width) << results[8] << spacer // Approximations
    << std::setw(col6_width) << results[9] << "\n"; // NaR

    masterFile << std::left  << std::setw(col1_width) << "Multiplication:" << spacer
        << std::right << std::setw(col2_width) << results[10] << spacer
        << std::setw(col3_width) << results[11] << spacer
        << std::setw(col4_width) << results[12] << spacer
        << std::setw(col5_width) << results[13] << spacer // Approximations
        << std::setw(col6_width) << results[14] << "\n"; // NaR

    masterFile << std::left  << std::setw(col1_width) << "Division:" << spacer
        << std::right << std::setw(col2_width) << results[15] << spacer
        << std::setw(col3_width) << results[16] << spacer
        << std::setw(col4_width) << results[17] << spacer
        << std::setw(col5_width) << results[18] << spacer // Approximations
        << std::setw(col6_width) << results[19] << "\n\n\n"; // NaR

    return 0;
}
