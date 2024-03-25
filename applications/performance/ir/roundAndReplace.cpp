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

int main(int argc, char* argv[])
try {
    using namespace sw::universal;
    using namespace sw::universal::blas;

    std::string testMatrix;
    std::streamsize old_precision = std::cout.precision();
    std::streamsize new_precision = 7;
    std::cout << std::setprecision(new_precision);
    
    for (auto & matrixName : TestMatrixList) {
		std::cout << matrixName << '\n';
        testMatrix = std::string(matrixName);
        matrix<double> ref = getTestMatrix(testMatrix);
        std::cout << "Size: (" << ref.rows() << ", " << ref.cols() << ")\n";
        std::cout << "Condition Number = " << kappa(testMatrix) << '\n';
        //    std::cout << "Condition estimate: " << condest(ref) << '\n';
	}


    // we want to create a table of results for the different low precision types
    // matrix   fp64    fp32    fp16    fp8    fp4    bf16    posit32    posit24    posit16    posit12    posit8
    // west0132  10     20      30      40     50     60      70         80         90         100        110


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

