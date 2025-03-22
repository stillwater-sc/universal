/**
 * 
 * Version 21 Mar 2025
 * 
 * Function used to build closure plots for a given number system
 * 
 * HOW TO USE - configure the first few lines of main() with your desired nbits, eBits number system, and the bool bHasNAR values
 * 
 * 
 */

#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <filesystem>

#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include "arithmetic_ops.hpp"

#include <string_view>


template<typename NumberType, char Op>
struct OperationStruc {
    static constexpr char getOperationChar() { return Op; }

    static constexpr std::string_view getOperationString() {
        if (Op == '+') return "addition";
        if (Op == '-') return "subtraction";
        if (Op == '*') return "multiplication";
        if (Op == '/') return "division";
        return "unknown";
    }

    static NumberType primary(NumberType a, NumberType b) {
        if (Op == '+') return a + b;
        if (Op == '-') return a - b;
        if (Op == '*') return a * b;
        if (Op == '/') return a / b;
        return NumberType(0);
    }

    static NumberType inverse(NumberType a, NumberType b) {
        if (Op == '+') return a - b;
        if (Op == '-') return a + b;
        if (Op == '*') return a / b;
        if (Op == '/') return a * b;
        return NumberType(0);
    }

    NumberType executeOperation(NumberType a, NumberType b) const {
        return primary(a, b);
    }

    NumberType executeInverseOperation(NumberType a, NumberType b) const {
        return inverse(a, b);
    }
};

struct NumberSystemStats {
    NumberSystemStats() : total{ 0 }, nars { 0 }, exact{ 0 }, approximate{ 0 }, overflow{ 0 }, underflow{ 0 }, saturate{ 0 } {}
    unsigned long total;
    unsigned long nars;
    unsigned long exact;
    unsigned long approximate;
    unsigned long overflow;
    unsigned long underflow;
    unsigned long saturate;
};



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
int buildClosurePlot(std::string system, NumberSystemStats& stats, 
                    std::ofstream& outFile, std::ofstream& csv_outFile, 
                    const OperationStruc<NumberType, Op>& operation){
    constexpr unsigned nbits = NumberType::nbits;

    char operationChar = operation.getOperationChar();
    std::string operationString = std::string(operation.getOperationString());

    constexpr unsigned char setw = 32;
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

    unsigned long narCount{0}, exactCount{0}, overFlowCount{0}, underFlowCount{0}, saturateCount{0}, approximateCount{0};
    unsigned NR_ENCODINGS = (1u << nbits);
    unsigned long totalOperations = NR_ENCODINGS * NR_ENCODINGS;

    // constant values of importance
    NumberType maxpos(sw::universal::SpecificValue::maxpos);
    NumberType minpos(sw::universal::SpecificValue::minpos);
    NumberType nar(sw::universal::SpecificValue::nar);
    double dmaxpos = double(maxpos);
    double dminpos = double(minpos);
    NumberType va{ 0 }, vb{ 0 }, vc{ 0 };
    for (int i = 0; i < NR_ENCODINGS; ++i) {

        va.setbits(i);

        for (int j = 0; j < NR_ENCODINGS; ++j) { // change to j = i when calculating uniquie pairs
            
            vb.setbits(j);

            NumberType vc = operation.executeOperation(va, vb);
            double vcDouble = double (vc);

            OperationStruc<double, Op> dblOp;
            double targetVal = dblOp.executeOperation(double (va), double (vb));

            std::string result = "";

            if (vc == nar) {
                ++narCount;
                result = "NAR";
            }
            else if(targetVal == vcDouble){
                ++exactCount;
                result = "Exact";
            }
            else if (targetVal > dmaxpos) {

                if(targetVal > 2* dmaxpos){
                ++overFlowCount;
                result = "Overflow";
                }
                else{
                    result = "Saturate";
                    ++saturateCount;
                }
            }
            else if(targetVal < dminpos){

                if(targetVal < 0.5 * dminpos){
                    ++underFlowCount;
                    result = "Underflow";
                }
                else{
                    result = "Saturate";
                    ++saturateCount;
                }
            }
            else{
                result = "Approximation";
                ++approximateCount;
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
    
    outFile << "\nTotal " << operationString << "s: " << totalOperations << "\n";
    outFile << "Total correct " << operationString << "s: " << exactCount << "\n";
    outFile << "Total overflow " << operationString << "s: " << overFlowCount << "\n";
    outFile << "Total underflow " << operationString << "s: " << underFlowCount << "\n";
    outFile << "Total saturate " << operationString << "s: " << saturateCount << "\n";
    outFile << "Total approximate " << operationString << "s: " << approximateCount << "\n";
    outFile << "Total nar " << operationString << "s: " << narCount << "\n\n\n";

    // organize statistics

    stats.total = totalOperations;
    stats.nars = narCount;
    stats.exact = exactCount;
    stats.approximate = approximateCount;
    stats.overflow = overFlowCount;
    stats.underflow = underFlowCount;
    stats.saturate = saturateCount;

    return 0;
}

std::string getOperation(char op) {
    std::string operation{};
    switch (op) {
    case '+':
        operation = "addition";
        break;
    case '-':
        operation = "subtraction";
        break;
    case '*':
        operation = "multiplication";
        break;
    case '/':
        operation = "division";
        break;
    default:
        operation = "unknown";
        break;
    }
    return operation;
}
/**
 * Appends the aggregated to the running master file
 * @param numberSystem the string representation of the system
 * @param masterFile the output file of the aggregated data
 * @param results the vector that contains the aggregated data
 * 
 */
int appendResultsToMasterFile(std::string numberSystem, std::ofstream& masterFile, std::map<char, NumberSystemStats>& results){

    // Define column widths based on longest terms
    const int col1_width = 15; // operation string (15 chars)
    const int col2_width =  9;  // "Total Ops" (9 chars)
    const int col3_width =  5;  // "Exact" (5 chars)
    const int col4_width = 11;  // "Approximate" (11 chars)
    const int col5_width =  8;  // "Overflow" (8 chars)
    const int col6_width =  9;  // "Underflow" (9 chars)
    const int col7_width =  8;  // "Saturate" (3 chars)
    const int col8_width =  3;  // "NaR" (3 chars)
    const std::string spacer = std::string(16, ' ');

    // Header row
    masterFile 
        << std::left  << std::setw(col1_width) << numberSystem << spacer
        << std::right << std::setw(col2_width) << "Total Ops" << spacer
        << std::setw(col3_width) << "Exact" << spacer
        << std::setw(col4_width) << "Approximate" << spacer
        << std::setw(col5_width) << "Overflow" << spacer
        << std::setw(col6_width) << "Underflow" << spacer
        << std::setw(col7_width) << "Saturate" << spacer
        << std::setw(col8_width) << "NAR" << "\n";

    // Data rows
    std::vector<char> ops = { '+', '-', '*', '/' };
    for (auto op : ops) {
        NumberSystemStats stats = results[op];
        masterFile 
            << std::left << std::setw(col1_width) << getOperation(op) << " :" << spacer
            << std::right << std::setw(col2_width) << stats.total << spacer
            << std::setw(col3_width) << stats.exact << spacer
            << std::setw(col4_width) << stats.approximate << spacer
            << std::setw(col5_width) << stats.overflow << spacer
            << std::setw(col6_width) << stats.underflow << spacer 
            << std::setw(col7_width) << stats.saturate << spacer
            << std::setw(col8_width) << stats.nars << '\n';
    }

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
int systemEvaluator(std::string system, std::string masterFilename, std::string outFilename, std::string csv_outFilename) {

    std::cout << "\nExecuting systemEvaluator function for " << system << "\n\n";

    std::ofstream masterfile(masterFilename, std::ios::app);
    if (!masterfile.is_open()) {
        std::cerr << "Error: Could not open master file " << masterFilename << " for writing.\n";
        return 1;
    }

    std::ofstream outFile(outFilename, std::ios::out | std::ios::trunc);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open output file " << outFilename << " for writing.\n";
        return 1;
    }

    std::ofstream csv_outFile(csv_outFilename, std::ios::out | std::ios::trunc);
    if (!csv_outFile.is_open()) {
        std::cerr << "Error: Could not open CSV output file " << csv_outFilename << " for writing.\n";
        return 1;
    }

    //vectorInitializerAllValues<NumberType>(valArray, contain_nars); // populate the vector w/ wall numbers in a system.

    // create a header for the CSV output file
    csv_outFile << std::left
        << system << ",,,,,\n"
        << "Result" << ","
        << "Value 1" << ","
        << "Operand" << ","
        << "Value 2" << ","
        << "Output" << ","
        << "Float(64) Value" << "\n";

    // create a statistics map

    NumberSystemStats addition;
    buildClosurePlot<NumberType>(system, addition, outFile, csv_outFile, OperationStruc<NumberType, '+'>{});
    NumberSystemStats subtraction;
    buildClosurePlot<NumberType>(system, subtraction, outFile, csv_outFile, OperationStruc<NumberType, '-'>{});
    NumberSystemStats multiplication;
    buildClosurePlot<NumberType>(system, multiplication, outFile, csv_outFile, OperationStruc<NumberType, '*'>{});
    NumberSystemStats division;
    buildClosurePlot<NumberType>(system, division, outFile, csv_outFile, OperationStruc<NumberType, '/'>{});

    std::map<char, NumberSystemStats> results;
    results['+'] = addition;
    results['-'] = subtraction;
    results['*'] = multiplication;
    results['/'] = division;
    appendResultsToMasterFile(system, masterfile, results); // append the results to the master file

    csv_outFile.close();
    outFile.close();
    masterfile.close();
    return 0;
}

/**
 *
 * Main function drives systemEvaluator
 *
 * There are three outputs from the program: All outputs are located in build/mappings
 *  1. A human readable mapping at readable_mappings/NumberSystem<nbits, eBits>.txt
 *  2. A csv file used for data visualization at csv_mappings/NumberSystem<nbits, eBits>.txt
 *  3. Appends MasterMappings.txt with the aggregated points on all operations performed
 *
 */
int main() {
    constexpr unsigned nbits{ 4 };  // size in bits of the encoding
    constexpr unsigned eBits{ 1 };  // number of exponent bits in the encoding
    using Real = sw::universal::posit<nbits, eBits>;

    std::string system = "posit<" + std::to_string(nbits) + "," + std::to_string(eBits) + ">"; // of the form Posit<bits, exp>
    std::string masterFilename = "mappings/MasterMappings.txt";
    std::string outFilename = "mappings/readable_mappings/" + system + ".txt";
    std::string CSV_outFilename = "mappings/csv_mappings/" + system + ".csv";

    systemEvaluator<Real>(system, masterFilename, outFilename, CSV_outFilename);

    return 0;
}