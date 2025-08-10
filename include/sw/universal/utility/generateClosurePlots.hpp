#pragma once
// x_over_one_minus_x.hpp: generic implementation of the function x / (1 - x)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// Author: Colby Wirth
// 
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

 #include <vector>
 #include <iostream>
 #include <string>
 #include <fstream>
 #include <iomanip>
 #include <filesystem> 
 #include <cstdlib> 
 
 #include <universal/utility/error.hpp>
 
 namespace sw { namespace universal {
 
 
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
     NumberSystemStats() : 
         total{ 0 }, 
         nars_and_nans { 0 }, 
         exact{ 0 }, 
         approximate{ 0 }, 
         overflow{ 0 }, 
         underflow{ 0 }, 
         saturate{ 0 },
		 absoluteError{ 0.0 },
		 relativeError{ 0.0 },
		 relativeLogError{ 0.0 }     {}
     std::uint64_t total;
     std::uint64_t nars_and_nans;
     std::uint64_t exact;
     std::uint64_t approximate;
     std::uint64_t overflow;
     std::uint64_t underflow;
     std::uint64_t saturate;
     double absoluteError;
     double relativeError;
     double relativeLogError;
 };
 
 
 /**
  * helper function for systemEvaluators: 
  * 
  *  3 error metrics are calculated:
  *      i. absolute error (absErr)
  *      ii. relative error (relativeErr)
  *      iii. relative log error (relativeLogErr)
  *              - this value is normalized with the logarithmic min max normalization function (MinMaxLogNormalization)
  * 
  *      All of these functions are found in error.hpp
  */
 template<typename NumberType>
 void calculateError(const std::string& result, 
                     double vcDouble, double targetVal, 
                     double& absErr, double& relativeErr, double& relativeLogErr,
                     NumberType maxpos, NumberType minpos){
 
         if (result == "NAR"){
             absErr = std::numeric_limits<double>::infinity();
             relativeErr = std::numeric_limits<double>::infinity();
             relativeLogErr = std::numeric_limits<double>::infinity();
         }
         
         else if (result == "Exact"){
             absErr = 0;
             relativeErr = 0;
             relativeLogErr = 0;
         }
 
         else{
             absErr = std::abs(sw::universal::AbsoluteError(vcDouble, targetVal));
             relativeErr = std::abs(sw::universal::RelativeError(vcDouble, targetVal));
             relativeLogErr = sw::universal::MinMaxLogNormalization(sw::universal::LogRelativeError(vcDouble, targetVal), (double) maxpos, (double) minpos);
         }
 }
 
 template<typename NumberType>
 void configureValue(NumberType v, std::string& vString){
     vString = sw::universal::to_binary(v);
     vString.erase(0,2);
     std::erase_if(vString, [](char c) { return c != '0' && c != '1'; });
 }
 
 
 /**
  * Handles the logic for building a closure plot for a number system
  * @param system the string representation of the system
  * @param results the statistics struct the contains aggregated results of the operations
  * @param outFile the .txt ostream
  * @param csv_outFile the .csv ostream
  * 
  * @return 0 
  * 
  */
 template<typename NumberType, char Op>
 int systemEvaluator(std::string system, NumberSystemStats& stats, 
                     std::ostream& outFile, std::ostream& csvFile, 
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
             << std::setw(setw) << "Value 1 Encoding"
             << std::setw(setw) << "Value 2 Encoding"
             << std::setw(setw) << "Output Encoding"
             << std::setw(setw) << "Absolute Error"
             << std::setw(setw) << "Relative Error"
             << std::setw(setw) << "Normalized Relative Log Error"
             << "\n";
 
     csvFile << "Generate '" <<  operation.getOperationChar() <<"' table:,,,,,,,,,,,\n";
 
     unsigned long nar_and_nan_Count{0}, exactCount{0}, overFlowCount{0}, underFlowCount{0}, saturateCount{0}, approximateCount{0};
     unsigned NR_ENCODINGS = (1u << nbits);
     unsigned long totalOperations = NR_ENCODINGS * NR_ENCODINGS;
 
     // constant values of importance
     NumberType maxpos(sw::universal::SpecificValue::maxpos);
     NumberType minpos(sw::universal::SpecificValue::minpos);
     NumberType nar(sw::universal::SpecificValue::nar);
     double dmaxpos = double(maxpos);
     double dminpos = double(minpos);
     NumberType va{ 0 }, vb{ 0 }, vc{ 0 };
     std::string vaString, vbString, vcString;
 
     for (int i = 0; i <  NR_ENCODINGS; ++i) {
 
         va.setbits(i);
  
         configureValue(va, vaString);
 
         for (int j = 0; j < NR_ENCODINGS; ++j) { // change to j = i when calculating uniquie pairs
             
             vb.setbits(j); 
 
             configureValue(vb, vbString);
  
             vc = operation.executeOperation(va, vb);
            configureValue(vc, vcString);
 
             double vcDouble = double (vc);
 
             
             OperationStruc<double, Op> dblOp;
             double targetVal = dblOp.executeOperation(double (va), double (vb));
 
             std::string result = ""; 
 
             if constexpr (sw::universal::is_posit<NumberType>){
 
                 // double useed = 1U << (1U << NumberType::es);
                 double useed = std::pow(2.0, std::pow(2.0, NumberType::es)); //useed = 2^2^es *not shifting to avoid warning 
                 double regime_max = std::pow(useed, nbits-1);
                 double regime_min = std::pow(useed, -(nbits - 1)); 
 
                 long overflow_min = std::sqrt(dmaxpos * regime_max);
                 long underflow_max = std::sqrt(dminpos * regime_min);
 
                 if (vc == nar ) {
                     ++nar_and_nan_Count;
                     result = "NAR/NAN";
                 }
                 else if(targetVal == vcDouble){
                     ++exactCount;
                     result = "Exact";
                 }
                 else if (targetVal > dmaxpos) {
 
                     if(targetVal > overflow_min){ //TODO: dynamically account for rounding of overflow_min and underflow max
                         ++overFlowCount;
                         result = "Overflow";
                     }
                     else{
                         result = "Saturate";
                         ++saturateCount;
                     }
                 }
                 else if(targetVal < dminpos){
 
                     if(targetVal < underflow_max){ //TODO: dynamically account for rounding of overflow_min and underflow max
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
             }
             else{ //works for cfloats - maybe LNS?
 
                 if(sw::universal::isnan(vc)){
                     result = "NAR/NAN";
                     nar_and_nan_Count++;
                 }
                 else if(targetVal == vcDouble){
                     result = "Exact";
                     exactCount++;
                 }
                 else if(sw::universal::isinf(vc)){
                             result = "Overflow";
                             overFlowCount++;
                 }
                 else if(!sw::universal::isnormal(vc)){
                             result = "Underflow";
                             underFlowCount++;
                 }
                 else{
                     result = "Approximation";
                     ++approximateCount;
                 }
 
             }
 
         double absErr;
         double relativeErr;
         double relativeLogErr;
 
        calculateError(result, vcDouble, targetVal, absErr, relativeErr, relativeLogErr, maxpos, minpos);  
 
         outFile << std::left 
         << std::setw(setw) << result
         << std::setw(setw) << va
         << std::setw(setw) << operationChar
         << std::setw(setw) << vb
         << std::setw(setw) << vc 
         << std::setw(setw) << targetVal
         << std::setw(setw) << vaString
         << std::setw(setw) << vbString
         << std::setw(setw) << vcString
         << std::setw(setw) << absErr
         << std::setw(setw) << relativeErr
         << std::setw(setw) << relativeLogErr
         << "\n";
 
         csvFile << std::left
         << result << ","
         << va << ","
         << operationChar << ","
         << vb << ","
         << vc  << ","
         << targetVal << ","
         << vaString << ","
         << vbString << ","
         << vcString << ","
         << absErr << ","
         << relativeErr << ","
         << relativeLogErr
         << "\n";
 
         }
     }
     
     outFile << "\nTotal " << operationString << "s: " << totalOperations << "\n";
     outFile << "Total correct " << operationString << "s: " << exactCount << "\n";
     outFile << "Total overflow " << operationString << "s: " << overFlowCount << "\n";
     outFile << "Total underflow " << operationString << "s: " << underFlowCount << "\n";
     outFile << "Total saturate " << operationString << "s: " << saturateCount << "\n";
     outFile << "Total approximate " << operationString << "s: " << approximateCount << "\n";
     outFile << "Total nar/nan " << operationString << "s: " << nar_and_nan_Count << "\n\n\n";
 
     // organize statistics
 
     stats.total = totalOperations;
     stats.nars_and_nans = nar_and_nan_Count;
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
 void ReportNumberSystemClosureStats(std::ostream& ostr, std::string numberSystem, std::map<char, NumberSystemStats>& results){
 
     // Define column widths based on longest terms
     const int col1_width = 15; // operation string (15 chars)
     const int col2_width =  9;  // "Total Ops" (9 chars)
     const int col3_width =  5;  // "Exact" (5 chars)
     const int col4_width = 11;  // "Approximate" (11 chars)
     const int col5_width =  8;  // "Overflow" (8 chars)
     const int col6_width =  9;  // "Underflow" (9 chars)
     const int col7_width =  8;  // "Saturate" (3 chars)
     const int col8_width =  3;  // "Nar/Nan" (7 chars)
     const std::string spacer = std::string(3, ' ');
 
     // Header row
     ostr
         << std::left << std::setw(col1_width) << numberSystem << spacer
         << std::right << std::setw(col2_width) << "Total Ops" << spacer
         << std::setw(col3_width) << "Exact" << spacer
         << std::setw(col4_width) << "Approximate" << spacer
         << std::setw(col5_width) << "Overflow" << spacer
         << std::setw(col6_width) << "Underflow" << spacer
         << std::setw(col7_width) << "Saturate" << spacer
         << std::setw(col8_width) << "NAR/NAN" << "\n";
 
     // Data rows
     std::vector<char> ops = { '+', '-', '*', '/' };
     for (auto op : ops) {
         NumberSystemStats stats = results[op];
         ostr 
             << std::left << std::setw(col1_width) << getOperation(op) << " :" << spacer
             << std::right << std::setw(col2_width) << stats.total << spacer
             << std::setw(col3_width) << stats.exact << spacer
             << std::setw(col4_width) << stats.approximate << spacer
             << std::setw(col5_width) << stats.overflow << spacer
             << std::setw(col6_width) << stats.underflow << spacer 
             << std::setw(col7_width) << stats.saturate << spacer
             << std::setw(col8_width) << stats.nars_and_nans << '\n';
     }
 
 }
 

/// <summary> 
///  main function builds a "closure plot" for a templatized number system
/// </summary>
///
/// <param name="system">a string representation of the system used to format the txt and csv file. 
/// examples: cfloat , posit , lns </param>
/// <param name="txtFile"> a txtFile to output the closure plot to - human readable. </param>
///
/// <param name="csvFile"> a csvFile to output the closure plot to - **USED TO CONFIGURE THE VISUAL CLOSURE PLOT** . </param>
///
/// <returns> 0 if executed properly</returns>
 template<typename NumberType>
 int buildClosurePlot(std::string system, std::ostream& txtFile, std::ostream& csvFile) {
 
     std::cout << "\n\nExecuting buildClosurePlot() function for " << system << ":\n\n";
 
     // create a header for the CSV output file
     csvFile << std::left
         << system << ",,,,,,,,,,,\n"
         << "Result,"
         << "Value 1,"
         << "Operand,"
         << "Value 2,"
         << "Output,"
         << "Float(64) Value," 
         << "Value 1 Encoding,"
         << "Value 2 Encoding,"
         << "Output Encoding,"
         << "Absolute Error,"
         << "Relative Error,"
         << "Normalized Relative Log Error"
         << "\n"
         ;
 
     // create a statistics map
     std::map<char, NumberSystemStats> results;
     NumberSystemStats stats;
     systemEvaluator<NumberType>(system, stats, txtFile, csvFile, OperationStruc<NumberType, '+'>{});
     results['+'] = stats;
     systemEvaluator<NumberType>(system, stats, txtFile, csvFile, OperationStruc<NumberType, '-'>{});
     results['-'] = stats;
     systemEvaluator<NumberType>(system, stats, txtFile, csvFile, OperationStruc<NumberType, '*'>{});
     results['*'] = stats;
     systemEvaluator<NumberType>(system, stats, txtFile, csvFile, OperationStruc<NumberType, '/'>{});
     results['/'] = stats;
 
     ReportNumberSystemClosureStats(std::cout, system, results);
 
     return 0;
 }
 
 } } // end namespace sw::universal