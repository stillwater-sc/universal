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
/// <param name="testMatrix"></param>
/// <returns></returns>
template<typename HighPrecision, typename WorkingPrecision, typename LowPrecision>
int RunOneRnRExperiment(const sw::universal::blas::matrix<double>& Td) {
    using namespace sw::universal::blas;

    using Mh = sw::universal::blas::matrix<HighPrecision>;
    using Mw = sw::universal::blas::matrix<WorkingPrecision>;
    using Ml = sw::universal::blas::matrix<LowPrecision>;

    // generate the matrices
    Mh Ah{ Td };
    Mw Aw{ Ah };
    Ml Al{ Aw };
    RoundAndReplace(Aw, Al);
    if (isinf(matnorm(Al))) return -1;

    // Solve the system of equations using iterative refinement
    int maxIterations = 10;
    int iterations = SolveIRLU<HighPrecision, WorkingPrecision, LowPrecision>(Ah, Aw, Al, maxIterations);
    if (iterations < maxIterations) {
        return iterations;
    }
	else {
		return -1;
	}
}

template<typename HighPrecision, typename WorkingPrecision, typename LowPrecision>
int RunRoundAndReplaceExperiment()
{
    ReportExperimentConfiguration<HighPrecision, WorkingPrecision, LowPrecision>();

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

#ifdef TEST_LUIR
void TestLUIR(const std::string& testMatrixName) 
{
    using namespace sw::universal;
    using namespace sw::universal::blas;

    matrix<double> ref = getTestMatrix(testMatrixName);
    RunOneRnRExperiment<fp64, fp64, fp64>(ref);
    RunOneRnRExperiment<fp64, fp64, fp32>(ref);
    RunOneRnRExperiment<fp64, fp32, fp32>(ref);
    RunOneRnRExperiment<fp64, fp32, fp16>(ref);
    RunOneRnRExperiment<fp32, fp32, fp32>(ref);
    RunOneRnRExperiment<fp32, fp32, fp16>(ref);
    RunOneRnRExperiment<fp32, fp16, fp16>(ref);
    RunOneRnRExperiment<fp32, fp16, fp8>(ref);
}
#endif

template<typename HighPrecision, typename WorkingPrecision, typename LowPrecision>
void ProtectedRnRExperiment(std::map<std::string, sw::universal::blas::vector<int>>& results, const std::string& testMatrix, const sw::universal::blas::matrix<double>& ref) {
    using namespace sw::universal;
    int iterations = -1;
    try {
        iterations = RunOneRnRExperiment<HighPrecision, WorkingPrecision, LowPrecision>(ref);
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

int main(int argc, char* argv[])
try {
    using namespace sw::universal;
    using namespace sw::universal::blas;

    std::string testMatrix;
    std::streamsize old_precision = std::cout.precision();
    std::streamsize new_precision = 7;
    std::cout << std::setprecision(new_precision);
    
    //RunOneRnRExperiment<fp64, fp32, fp16>(getTestMatrix("faires74x3"));

    //return 0;

    // we want to create a table of results for the different low precision types
    // matrix   fp64    fp32    fp16    fp8    fp4    bf16    posit32    posit24    posit16    posit12    posit8
    // west0132  10     20      30      40     50     60      70         80         90         100        110

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
    std::map<std::string, sw::universal::blas::vector<int>> results;
    for (auto& matrixName : testMatrices) {
        testMatrix = std::string(matrixName);
        matrix<double> ref = getTestMatrix(testMatrix);

        using bf16 = bfloat_t;

        ProtectedRnRExperiment<fp64, fp64, fp64>(results, testMatrix, ref);
        ProtectedRnRExperiment<fp32, fp32, fp32>(results, testMatrix, ref);
        ProtectedRnRExperiment<fp64, bf16, bf16>(results, testMatrix, ref);
        ProtectedRnRExperiment<fp64, fp32, fp16>(results, testMatrix, ref);
        ProtectedRnRExperiment<fp32, fp16, fp8> (results, testMatrix, ref);

        ProtectedRnRExperiment<posit<32, 2>, posit<32, 2>, posit<32, 2>>(results, testMatrix, ref);
        ProtectedRnRExperiment<posit<32, 2>, posit<32, 2>, posit<24, 2>>(results, testMatrix, ref);
        ProtectedRnRExperiment<posit<32, 2>, posit<32, 2>, posit<16, 2>>(results, testMatrix, ref);
        ProtectedRnRExperiment<posit<32, 2>, posit<32, 2>, posit<12, 2>>(results, testMatrix, ref);
        ProtectedRnRExperiment<posit<32, 2>, posit<32, 2>, posit< 8, 2>>(results, testMatrix, ref);

    }
    sw::universal::blas::vector<std::string> typeLabels = { "fp64", "fp32", "bf16", "fp16", "fp8", "posit32", "posit24", "posit16", "posit12", "posit8" };
    // create the header
    std::cout << "Matrix";
    for (auto& e : typeLabels) {
        std::cout << ',' << e;
    }
    std::cout << '\n';
    for (auto& m : testMatrices) {
        std::cout << m;
        auto r = results[m];
        for (auto& e: r) {
			std::cout << ',' << e;
		}
        std::cout << '\n';
    }

//    std::cout << ref << '\n';


    std::cout << std::setprecision(old_precision);
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

