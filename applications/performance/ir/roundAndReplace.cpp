/** ************************************************************************
* roundAndReplace: A = LU Iterative Refinement approach
* 
*    Addresses the fundamental problem of solving Ax = b efficiently.
*      
* @author:     James Quinlan
* @copyright:  Copyright (c) 2022 James Quinlan
* SPDX-License-Identifier: MIT
* 
* This file is part of the Mixed Precision Iterative Refinement project
* *************************************************************************
*/
#include<universal/utility/directives.hpp>
#include<universal/utility/long_double.hpp>
#include<universal/utility/bit_cast.hpp>

#include <iostream>
#include <fstream>
#include <iomanip>

// Universal Number System Types
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>

// Higher Order Libraries
#include <universal/blas/blas.hpp>
#include <universal/blas/ext/solvers/fused_backsub.hpp>
#include <universal/blas/ext/solvers/fused_forwsub.hpp>
// get the test matrix database API
#include <universal/blas/serialization/test_matrix.hpp>

/// <summary>
/// run one LUIR experiment with Round-and-Replace preconditioning
/// </summary>
/// <typeparam name="LowPrecision"></typeparam>
/// <typeparam name="HighPrecision"></typeparam>
/// <typeparam name="WorkingPrecision"></typeparam>
/// <param name="Td">test matrix in double precision</param>
/// <param name="reportResultVector">if true report the result vector</param>
/// <returns>number of iterations</returns>
template<typename HighPrecision, typename WorkingPrecision, typename LowPrecision>
int RunOneRnRExperiment(const sw::universal::blas::matrix<double>& Td, bool reportResultVector = false) {
    using namespace sw::universal::blas;

    using Mh = sw::universal::blas::matrix<HighPrecision>;
    using Mw = sw::universal::blas::matrix<WorkingPrecision>;
    using Ml = sw::universal::blas::matrix<LowPrecision>;

    // generate the matrices
    Mh Ah{ Td };
    Mw Aw{ Ah };
    Ml Al{ Aw };
    RoundAndReplace(Aw, Al);
    // std::cout << "matrix norm: " << matnorm(Al) << '\n';
    if (isinf(matnorm(Al))) return -1;

    // Solve the system of equations using iterative refinement
    int maxIterations = 10;
    int iterations = SolveIRLU<HighPrecision, WorkingPrecision, LowPrecision>(Ah, Aw, Al, maxIterations, reportResultVector);
    if (iterations < maxIterations) {
        return iterations;
    }
	else {
		return iterations;   // is there a way to communicate this information about the failure to converge?
	}
}

/// <summary>
/// run one LUIR experiment with Round-and-Replace preconditioning with exception handling to catch numerical exceptions
/// </summary>
/// <typeparam name="HighPrecision"></typeparam>
/// <typeparam name="WorkingPrecision"></typeparam>
/// <typeparam name="LowPrecision"></typeparam>
/// <param name="testMatrix"></param>
/// <param name="ref"></param>
/// <param name="results"></param>
/// <param name="reportResultVector"></param>
template<typename HighPrecision, typename WorkingPrecision, typename LowPrecision>
void ProtectedRnRExperiment(const std::string& testMatrix, const sw::universal::blas::matrix<double>& ref, std::map<std::string, sw::universal::blas::vector<int>>& results, bool reportResultVector = false) {
    using namespace sw::universal;
    int iterations = -1;
    try {
        iterations = RunOneRnRExperiment<HighPrecision, WorkingPrecision, LowPrecision>(ref, reportResultVector);
        results[testMatrix].push_back(iterations);
    }
    catch (const sw::universal::universal_arithmetic_exception& err) {
        std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
        results[testMatrix].push_back(-1);
    }
    catch (std::runtime_error& err) {
        std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
        results[testMatrix].push_back(-1);
    }
    catch (...) {
        std::cerr << "Caught unknown exception" << std::endl;
        results[testMatrix].push_back(-1);
    }
}

/// <summary>
/// print the results of a LUIR experiments
/// </summary>
/// <param name="ostr"></param>
/// <param name="testMatrices"></param>
/// <param name="typeLabels"></param>
/// <param name="results"></param>
void PrintExperimentResults(std::ostream& ostr, const std::vector<std::string>& testMatrices, const sw::universal::blas::vector<std::string>& typeLabels, std::map<std::string, sw::universal::blas::vector<int> >& results) {
    // create CSV output
   // create the header
    ostr << "Matrix";
    for (auto& e : typeLabels) {
        ostr << ',' << e;
    }
    ostr << '\n';
    for (auto& m : testMatrices) {
        ostr << m;
        if (auto it = results.find(m); it != results.end()) {
            auto& r = results[m];
            for (auto& e : r) {
                ostr << ',' << e;
            }
        }
        ostr << '\n';
    }
}

// we want to create a table of results for the different low precision types
 // matrix   fp64    fp32    fp16    fp8    fp4    bf16    posit32    posit24    posit16    posit12    posit8
 // west0132  10     20      30      40     50     60      70         80         90         100        110


/// <summary>
/// run a series of LUIR experiments with Round-and-Replace preconditioning
/// </summary>
/// <param name="ostr"></param>
/// <param name="testMatrices"></param>
void RunRoundAndReplaceExperiment(std::ostream& ostr, const std::vector<std::string>& testMatrices)
{
    using namespace sw::universal;
    using namespace sw::universal::blas;

    sw::universal::blas::vector<std::string> typeLabels = { "fp64", "fp32", "bf16", "fp16", "fp8", "posit32", "posit24", "posit16", "posit12", "posit8" };

    std::map<std::string, sw::universal::blas::vector<int>> results;
    for (auto& testMatrix : testMatrices) {
        matrix<double> ref = getTestMatrix(testMatrix);

        using bf16 = bfloat_t;

        ProtectedRnRExperiment<fp64, fp64, fp64>(testMatrix, ref, results);
        ProtectedRnRExperiment<fp32, fp32, fp32>(testMatrix, ref, results);
        ProtectedRnRExperiment<fp64, bf16, bf16>(testMatrix, ref, results);
        ProtectedRnRExperiment<fp64, fp32, fp16>(testMatrix, ref, results);
        ProtectedRnRExperiment<fp32, fp16, fp8> (testMatrix, ref, results);

        ProtectedRnRExperiment<posit<32, 2>, posit<32, 2>, posit<32, 2>>(testMatrix, ref, results);
        ProtectedRnRExperiment<posit<32, 2>, posit<32, 2>, posit<24, 2>>(testMatrix, ref, results);
        ProtectedRnRExperiment<posit<32, 2>, posit<32, 2>, posit<16, 2>>(testMatrix, ref, results);
        ProtectedRnRExperiment<posit<32, 2>, posit<32, 2>, posit<12, 2>>(testMatrix, ref, results);
        ProtectedRnRExperiment<posit<32, 2>, posit<32, 2>, posit< 8, 2>>(testMatrix, ref, results);
    }

	// print the results
    PrintExperimentResults(ostr, testMatrices, typeLabels, results);
}

void ReportKappaValuesForTestMatrices() {
    using namespace sw::universal::blas;
    for (auto& matrixName : TestMatrixList) {
        std::cout << matrixName << '\n';
        matrix<double> ref = getTestMatrix(matrixName);
        std::cout << "Size: (" << ref.rows() << ", " << ref.cols() << ")\n";
        std::cout << "Condition Number = " << kappa(matrixName) << '\n';
        //    std::cout << "Condition estimate: " << condest(ref) << '\n';
    }
}

void RunSmallTestMatrixExperiment()
{
    std::vector<std::string> testMatrices = {
                "lambers_well",
                "lambers_ill",
                "h3",
                "q3",
                "int3",
                "faires74x3",
                "q4",
                "lu4",
                "s4",
                "rand4",
                "q5"
    };

    std::string resultFileName{ "smallMatricesRnR.csv" };
    std::ofstream ofs;
    ofs.open(resultFileName);
    if (ofs.good()) {
        RunRoundAndReplaceExperiment(ofs, testMatrices);
    }
    else {
        std::cerr << "Unable to open file " << resultFileName << '\n';
    }
    ofs.close();
}

void RunTestMatrixExperiment()
{
    std::vector<std::string> testMatrices = {
                "west0132",
                "west0167",
                "steam1",
                "steam3",
                "fs_183_1",
                "fs_183_3",
                "bwm200",
                "gre_343",
                "b1_ss",
                "cage3",
                "pores_1",
                "Stranke94",
                "saylr1",
                "Trefethen_20",
                "bcsstk01",
                "bcsstk03",
                "bcsstk04",
                "bcsstk05",
                "bcsstk22",
                "lund_a",
                "nos1",
                "arc130",
                "tumorAntiAngiogenesis_2"
    };

    std::string resultFileName{ "testMatricesRnR.csv" };
    std::ofstream ofs;
    ofs.open(resultFileName);
    if (ofs.good()) {
        RunRoundAndReplaceExperiment(ofs, testMatrices);
    }
    else {
        std::cerr << "Unable to open file " << resultFileName << '\n';
    }
    ofs.close();
}

void RunDebugTest() 
{
    using namespace sw::universal;
    using namespace sw::universal::blas;
    std::map<std::string, sw::universal::blas::vector<int>> results;
    std::string testMatrix = std::string("q3");
    matrix<double> ref = getTestMatrix(testMatrix);
    vector<std::string> typeLabels = { "fp16", "posit<32, 2>" };
    ProtectedRnRExperiment<fp64, fp32, fp16>(testMatrix, ref, results, true);
    ProtectedRnRExperiment<posit<32, 2>, posit<32, 2>, posit<16, 2>>(testMatrix, ref, results, true);
    std::vector<std::string> testMatrices = { testMatrix };
    PrintExperimentResults(std::cout, testMatrices, typeLabels, results);
}

int main(int argc, char* argv[])
try {
    using namespace sw::universal;
    using namespace sw::universal::blas;

    RunSmallTestMatrixExperiment();

    RunTestMatrixExperiment();

    return EXIT_SUCCESS;
}
catch (char const* msg) {
    std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
    return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
    std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
    std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
    std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
    return EXIT_FAILURE;
}
catch (...) {
    std::cerr << "Caught unknown exception" << std::endl;
    return EXIT_FAILURE;
}

