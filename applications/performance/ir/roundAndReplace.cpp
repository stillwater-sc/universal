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
#include <universal/blas/matrices/testsuite.hpp>

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

    // set up the set of test matrices
    std::vector<std::string> allMatrices = {
        "lambers_well",  //   2 x   2 well-conditioned matrix, K = 10.0
        "lambers_ill",   //   2 x   2 ill-conditioned matrix, K = 1.869050824603144e+08
        "h3",            //   3 x   3 test matrix, K = 1.8478e+11
        "int3",          //   3 x   3 integer test matrix (low condition number), K = 43.6115
        "faires74x3",    //   3 x   3 Burden Faires Ill-conditioned, K = 15999
        "q3",            //   3 x   3 Variable test matrix (edit entries), K = 1.2857e+06
        "q4",            //   4 x   4 test matrix, K = 2.35
        "q5",            //   4 x   4 test matrix, K = 1.1e+04
        "lu4",           //   4 x   4 test matrix, K = 11.6810
        "s4",            //   4 x   4 test matrix, K = 4.19
        "rand4",         //   4 x   4 random (low condition), K = 27.81
        "west0132",      // 132 x 132 Chem. Simulation Process, K = 4.2e+11 
        "west0167",      // 167 x 167 Chemical Simulation Process, K = 2.827e+07
        "steam1",        // 240 x 240 Computational Fluid Dynamics, K = 2.827501e+07
        "steam3",        //  83 x  83 Computational Fluid Dynamics, K = 5.51e+10
        "fs_183_1",      // 183 x 183 2D/3D Problem Sequence, K = 1.5129e+13
        "fs_183_3",      // 183 x 183 2D/3D Problem Sequence, K = 1.5129e+13
        "bwm200",        // 200 x 200 Chemical simulation, K = 2.412527e+03
        "gre_343",       // 343 x 343 Directed Weighted Graph, K = 1.119763e+02
        "b1_ss",         //   7 x   7 Chemical Process Simulation Problem, K = 1.973732e+02
        "cage3",         //   5 x   5 Directed Weighted Graph, K = 1.884547e+01
        "pores_1",       //  30 x  30 Computational Fluid Dynamics, K = 1.812616e+06
        "Stranke94",     //  10 x  10 Undirected Weighted Graph, K = 5.173300e+01
        "Trefethen_20",  //  20 x  20 Combinatorial Problem, K = 6.308860e+01
        "bcsstk01",      //  48 x  48 Structural Engineering, K = 8.8234e+05
        "bcsstk03",      // 112 x 112 Structural Engineering, K = 6.791333e+06
        "bcsstk04",      // 132 x 132 Structural Engineering, K = 2.292466e+06
        "bcsstk05",      // 153 x 153 Structural Engineering, K = 1.428114e+04
        "bcsstk22",      // 138 x 138 Structural Engineering, K = 1.107165e+05
        "lund_a",        // 147 x 147 Structural Engineering, K = 2.796948e+06
        "nos1",          // 237 x 237 Structural Engineering K = 1.991546e+07
        "arc130",        // 130 x 130    K = 6.0542e+10
        "saylr1",        // 238 x 238 Computational Fluid Dynamics, K = 7.780581e+08
        "tumorAntiAngiogenesis_2" // , K 1.9893e+10
    };



    std::vector<std::string > bigMatrices = {
        "west0132",      // 132 x 132 Chem. Simulation Process, K = 
        "west0167",      // 167 x 167 Chemical Simulation Process, K =
        "west0479",      // 479 x 479 Chemical Simulation Process, K =
        "steam1",        // 240 x 240 Computational Fluid Dynamics, K =
        "steam3",        //  83 x 83  Computational Fluid Dynamics, K =   
        "fs_183_1",      // 183 x 183 2D/3D Problem Sequence, K =   
        "fs_183_3",      // 183 x 183 2D/3D Problem Sequence, K =    
        "bwm200",        // 200 x 200 Chem. simulation K = 1e3.
        "gre_343",       // 343 x 343 Directed Weighted Graph, K = 
        "cage3",         // 5 x 5 Directed Weighted Graph, K =   
        "pores_1",       // 30 x 30 Computational Fluid Dynamics, K = 
        "Stranke94",     // 10 x 10 Undirected Weighted Graph, K = 
        "Trefethen_20",  // 20 x 20 Combinatorial Problem, K = 
        "bcsstk01",      // 48 x 48 Structural Engineering, K = 
        "bcsstk03",      // 112 x 112 Structural Engineering, K = 
        "bcsstk04",      // 132 x 132 Structural Engineering, K = 
        "bcsstk05",      // 153 x 153 Structural Engineering, K = 
        "bcsstk22",      // 138 x 138 Structural Engineering, K = 
        "lund_a",        // 147 x 147 Structural Engineering, K =   
        "nos1",          // 237 x 237 Structural Engineering K = 1e7  
        "arc130",        // 130 x 130
        "saylr1",        // 238 x 238 Computational Fluid Dynamics, K = 
    };

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

    std::string testMatrix = std::string("west0132");
    std::streamsize old_precision = std::cout.precision();
    std::streamsize new_precision = 7;
    std::cout << std::setprecision(new_precision);
    
    testMatrix = std::string("lu4");
    matrix<double> ref = getTestMatrix(testMatrix);
    std::cout << "Size: (" << ref.rows() << ", " << ref.cols() << ")\n";
    std::cout << "Condition Number = " << kappa(testMatrix) << '\n';
    //    std::cout << "Condition estimate: " << condest(ref) << '\n';

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

