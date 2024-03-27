// twoSidedScaleAndRound: A = LU Iterative Refinement approach
//
//   Addresses the fundamental problem of solving Ax = b efficiently.
//
// Copyright (c) 2022 James Quinlan
// SPDX-License-Identifier: MIT
// 
// This file is part of the Mixed Precision Iterative Refinement project.
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
#include "experiment_utils.hpp"

/// <summary>
/// run one LUIR experiment with Scale-and-Round preconditioning
/// </summary>
/// <typeparam name="LowPrecision"></typeparam>
/// <typeparam name="HighPrecision"></typeparam>
/// <typeparam name="WorkingPrecision"></typeparam>
/// <param name="Td">test matrix in double precision</param>
/// <param name="reportResultVector">if true report the result vector</param>
/// <returns>number of iterations</returns>
template<typename HighPrecision, typename WorkingPrecision, typename LowPrecision>
int RunOne2sSnRExperiment(const sw::universal::blas::matrix<double>& Td, bool reportResultVector = false) {
    using namespace sw::universal::blas;

    using Mh = sw::universal::blas::matrix<HighPrecision>;
    using Mw = sw::universal::blas::matrix<WorkingPrecision>;
    using Ml = sw::universal::blas::matrix<LowPrecision>;
    using Vw = sw::universal::blas::vector<WorkingPrecision>;

    // generate the matrices
    Mh Ah{ Td };
    Mw Aw{ Ah };
    Ml Al{ Aw };
    WorkingPrecision t  = 0.1;  // 2949990 Is there an optimal value?  Parameter sweep 0.75 west
    WorkingPrecision mu = 1.0;  // 16 best for posit<x,2>
    TwoSidedScaleAndRound(Aw, Al, t, mu);
    std::cout << "matrix norm: " << matnorm(Al) << '\n';
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
/// run one LUIR experiment with Scale-and-Round preconditioning with exception handling to catch numerical exceptions
/// </summary>
/// <typeparam name="HighPrecision"></typeparam>
/// <typeparam name="WorkingPrecision"></typeparam>
/// <typeparam name="LowPrecision"></typeparam>
/// <param name="testMatrix"></param>
/// <param name="ref"></param>
/// <param name="results"></param>
/// <param name="reportResultVector"></param>
template<typename HighPrecision, typename WorkingPrecision, typename LowPrecision>
void Protected2sSnRExperiment(const std::string& testMatrix, const sw::universal::blas::matrix<double>& ref, std::map<std::string, sw::universal::blas::vector<int>>& results, bool reportResultVector = false) {
    using namespace sw::universal;
    int iterations = -1;
    try {
        iterations = RunOne2sSnRExperiment<HighPrecision, WorkingPrecision, LowPrecision>(ref, reportResultVector);
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
/// run a series of LUIR experiments with Scale-and-Round preconditioning
/// </summary>
/// <param name="ostr"></param>
/// <param name="testMatrices"></param>
void RunScaleAndRoundExperiment(std::ostream& ostr, const std::vector<std::string>& testMatrices)
{
    using namespace sw::universal;
    using namespace sw::universal::blas;

    sw::universal::blas::vector<std::string> typeLabels = { "fp64", "fp32", "bf16", "fp16", "fp8", "posit32", "posit24", "posit16", "posit12", "posit8" };

    std::map<std::string, sw::universal::blas::vector<int>> results;
    for (auto& testMatrix : testMatrices) {
        matrix<double> ref = getTestMatrix(testMatrix);

        using bf16 = bfloat_t;

        Protected2sSnRExperiment<fp64, fp64, fp64>(testMatrix, ref, results);
        Protected2sSnRExperiment<fp32, fp32, fp32>(testMatrix, ref, results);
        Protected2sSnRExperiment<fp64, bf16, bf16>(testMatrix, ref, results);
        Protected2sSnRExperiment<fp64, fp32, fp16>(testMatrix, ref, results);
        Protected2sSnRExperiment<fp32, fp16, fp8> (testMatrix, ref, results);

        Protected2sSnRExperiment<posit<32, 2>, posit<32, 2>, posit<32, 2>>(testMatrix, ref, results);
        Protected2sSnRExperiment<posit<32, 2>, posit<32, 2>, posit<24, 2>>(testMatrix, ref, results);
        Protected2sSnRExperiment<posit<32, 2>, posit<32, 2>, posit<16, 2>>(testMatrix, ref, results);
        Protected2sSnRExperiment<posit<32, 2>, posit<32, 2>, posit<12, 2>>(testMatrix, ref, results);
        Protected2sSnRExperiment<posit<32, 2>, posit<32, 2>, posit< 8, 2>>(testMatrix, ref, results);
    }

    PrintIterativeRefinementExperimentResults(ostr, testMatrices, typeLabels, results);
}


void RunSmallTestMatrixExperiment(const std::string& resultFileName)
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

    std::ofstream ofs;
    ofs.open(resultFileName);
    if (ofs.good()) {
        RunScaleAndRoundExperiment(ofs, testMatrices);
    }
    else {
        std::cerr << "Unable to open file " << resultFileName << '\n';
    }
    ofs.close();
}

void RunTestMatrixExperiment(const std::string& resultFileName)
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

    std::ofstream ofs;
    ofs.open(resultFileName);
    if (ofs.good()) {
        RunScaleAndRoundExperiment(ofs, testMatrices);
    }
    else {
        std::cerr << "Unable to open file " << resultFileName << '\n';
    }
    ofs.close();
}

void RunDebugTest1() 
{
    using namespace sw::universal;
    using namespace sw::universal::blas;
    std::map<std::string, sw::universal::blas::vector<int>> results;
    std::string testMatrix = std::string("q3");
    matrix<double> ref = getTestMatrix(testMatrix);
    vector<std::string> typeLabels = { "fp16", "posit<16, 2>" };
    Protected2sSnRExperiment<fp64, fp32, fp16>(testMatrix, ref, results, true);
    Protected2sSnRExperiment<posit<32, 2>, posit<32, 2>, posit<16, 2>>(testMatrix, ref, results, true);
    std::vector<std::string> testMatrices = { testMatrix };
    PrintIterativeRefinementExperimentResults(std::cout, testMatrices, typeLabels, results);
}

void RunDebugTest2()
{
    using namespace sw::universal;
    using namespace sw::universal::blas;
    std::map<std::string, sw::universal::blas::vector<int>> results;
    std::string testMatrix = std::string("bcsstk01");  // K = 8.8234e+05
    matrix<double> ref = getTestMatrix(testMatrix);
    vector<std::string> typeLabels = { "fp32", "posit<32, 2>", "posit<24, 2>", "posit<16, 2>", "posit<8, 2>"};
    Protected2sSnRExperiment<fp64, fp32, fp32>(testMatrix, ref, results, true);
    Protected2sSnRExperiment<posit<32, 2>, posit<32, 2>, posit<32, 2>>(testMatrix, ref, results, true);
    Protected2sSnRExperiment<posit<32, 2>, posit<32, 2>, posit<24, 2>>(testMatrix, ref, results, true);
    Protected2sSnRExperiment<posit<32, 2>, posit<32, 2>, posit<16, 2>>(testMatrix, ref, results, true);
    Protected2sSnRExperiment<posit<32, 2>, posit<32, 2>, posit< 8, 2>>(testMatrix, ref, results, true);
    std::vector<std::string> testMatrices = { testMatrix };
    PrintIterativeRefinementExperimentResults(std::cout, testMatrices, typeLabels, results);
}

int main(int argc, char* argv[])
try {
    using namespace sw::universal;
    using namespace sw::universal::blas;

    // RunDebugTest1();
    // RunDebugTest2();

    RunSmallTestMatrixExperiment("s2sSnR.csv");

    // RunTestMatrixExperiment("2sSnR.csv");

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

