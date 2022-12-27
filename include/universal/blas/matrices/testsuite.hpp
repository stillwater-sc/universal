/** **********************************************************************
 * Matrix Test Suite for Iterative Refinement Iterations
 *
 * @author:     James Quinlan
 * @date:       2022-12-13
 * @copyright:  Copyright (c) 2022 Stillwater Supercomputing, Inc.
 * @license:    MIT Open Source license 
 * ***********************************************************************
 */

#pragma once

// Import Matrices
#include <universal/blas/matrices/lambers_well.hpp>  // 2 x 2 well-conditioned matrix
#include <universal/blas/matrices/lambers_ill.hpp>   // 2 x 2 ill-conditioned matrix
#include <universal/blas/matrices/h3.hpp>            // 3 x 3 test matrix
#include <universal/blas/matrices/q3.hpp>            // 3 x 3 test matrix
#include <universal/blas/matrices/int3.hpp>          // 3x3 integer test matrix (low condition number)
#include <universal/blas/matrices/faires74x3.hpp>    // Burden Faires 3x3 Ill-conditioned
#include <universal/blas/matrices/q4.hpp>            // 4 x 4 test matrix
#include <universal/blas/matrices/q5.hpp>            // 4 x 4 test matrix
#include <universal/blas/matrices/lu4.hpp>           // 4 x 4 test matrix
#include <universal/blas/matrices/s4.hpp>            // 4 x 4 test matrix
#include <universal/blas/matrices/rand4.hpp>         // Random 4x4 (low condition) for testing
#include <universal/blas/matrices/west0132.hpp>      //
#include <universal/blas/matrices/west0167.hpp>      //
#include <universal/blas/matrices/steam1.hpp>        //
#include <universal/blas/matrices/steam3.hpp>        //
#include <universal/blas/matrices/fs_183_1.hpp>      //
#include <universal/blas/matrices/fs_183_3.hpp>      // 
#include <universal/blas/matrices/bwm200.hpp>        // Chem. simulation 1e3.
#include <universal/blas/matrices/gre_343.hpp>       // Directed Weighted Graph
#include <universal/blas/matrices/b1_ss.hpp>         // 7x7 Chemical Process Simulation Problem
#include <universal/blas/matrices/cage3.hpp>         // 
#include <universal/blas/matrices/pores_1.hpp>       // 30x30 Computational Fluid Dynamics
#include <universal/blas/matrices/Stranke94.hpp>     // 10 x 10 Undirected Weighted Graph
#include <universal/blas/matrices/Trefethen_20.hpp>  // 20x20 Combinatorial Problem
#include <universal/blas/matrices/bcsstk01.hpp>      // 48x48
#include <universal/blas/matrices/bcsstk03.hpp>      // 112 x 112
#include <universal/blas/matrices/bcsstk04.hpp>      // 132 x 132
#include <universal/blas/matrices/bcsstk05.hpp>      // 153 x 153
#include <universal/blas/matrices/bcsstk22.hpp>      // 138 x 138
#include <universal/blas/matrices/lund_a.hpp>        //  
#include <universal/blas/matrices/nos1.hpp>          //
#include <universal/blas/matrices/arc130.hpp>
#include <universal/blas/matrices/saylr1.hpp>        // 238 x 238 CFD
#include <universal/blas/matrices/tumorAntiAngiogenesis_2.hpp>      // 


// Get Matrix
sw::universal::blas::matrix<double> getTestMatrix(const std::string &testMatrix){
    if (testMatrix == "lambers_well"){
        return lambers_well;
    }else if (testMatrix == "lambers_ill"){
        return lambers_ill;
    }else if (testMatrix == "h3"){
        return h3;
    }else if (testMatrix == "q3"){
        return q3;
    }else if (testMatrix == "int3"){
        return int3;
    }else if (testMatrix == "faires74x3"){
        return faires74x3;
    }else if (testMatrix == "q4"){
        return q4;
    }else if (testMatrix == "lu4"){
        return lu4;
    }else if (testMatrix == "s4"){
        return s4;
    }else if (testMatrix == "rand4"){
        return rand4;
    }else if (testMatrix == "q5"){
        return q5;
    }else if (testMatrix == "west0132"){
        return west0132;
    }else if (testMatrix == "west0167"){
        return west0167;
    }else if (testMatrix == "steam1"){
        return steam1;
    }else if (testMatrix == "steam3"){
        return steam3;
    }else if (testMatrix == "fs_183_1"){
        return fs_183_1;
    }else if (testMatrix == "fs_183_3"){
        return fs_183_3;
    }else if (testMatrix == "bwm200"){
        return bwm200;
    }else if (testMatrix == "gre_343"){
        return gre_343;
    }else if (testMatrix == "b1_ss"){
        return b1_ss;
    }else if (testMatrix == "cage3"){
        return cage3;
    }else if (testMatrix == "pores_1"){
        return pores_1;
    }else if (testMatrix == "Stranke94"){
        return Stranke94;
    }else if (testMatrix == "Trefethen_20"){
        return Trefethen_20;
    }else if (testMatrix == "bcsstk01"){
        return bcsstk01;
    }else if (testMatrix == "bcsstk03"){
        return bcsstk03;
    }else if (testMatrix == "bcsstk04"){
        return bcsstk04;
    }else if (testMatrix == "bcsstk05"){
        return bcsstk05;
    }else if (testMatrix == "bcsstk22"){
        return bcsstk22;
    }else if (testMatrix == "lund_a"){
        return lund_a;
    }else if (testMatrix == "nos1"){
        return nos1;
    }else if (testMatrix == "arc130"){
        return arc130;
    }else if (testMatrix == "saylr1"){
        return saylr1;
    }else if (testMatrix == "tumorAntiAngiogenesis_2"){
        return tumorAntiAngiogenesis_2;    
    }else{
        return lu4;
    }
} // end    


// Condition Number
double kappa(const std::string &testMatrix){
    if (testMatrix == "lambers_well"){
        return 10.0;
    }else if (testMatrix == "lambers_ill"){
        return 1.869050824603144e+08;
    }else if (testMatrix == "h3"){
        return 1.8478e+11;
    }else if (testMatrix == "q3"){
        return 1.2857e+06;
    }else if (testMatrix == "int3"){
        return 43.6115;
    }else if (testMatrix == "faires74x3"){
        return 15999;
    }else if (testMatrix == "q4"){
        return 2.35;
    }else if (testMatrix == "lu4"){
        return 11.6810;
    }else if (testMatrix == "s4"){
        return 4.19;
    }else if (testMatrix == "rand4"){
        return 27.81;
    }else if (testMatrix == "q5"){
        return 1.1e+04;
    }else if (testMatrix == "west0132"){
        return 4.2e+11;
    }else if (testMatrix == "west0167"){
        return 2.827e+07;
    }else if (testMatrix == "steam1"){
        return 2.827501e+07;
    }else if (testMatrix == "steam3"){
        return 5.51e+10;
    }else if (testMatrix == "fs_183_1"){
        return 1.5129e+13;
    }else if (testMatrix == "fs_183_3"){
        return 1.5129e+13;
    }else if (testMatrix == "bwm200"){
        return 2.412527e+03;
    }else if (testMatrix == "gre_343"){
        return 1.119763e+02;
    }else if (testMatrix == "b1_ss"){
        return 1.973732e+02;
    }else if (testMatrix == "cage3"){
        return 1.884547e+01;
    }else if (testMatrix == "pores_1"){
        return 1.812616e+06;
    }else if (testMatrix == "Stranke94"){
        return 5.173300e+01;
    }else if (testMatrix == "saylr1"){
        return 7.780581e+08;
    }else if (testMatrix == "Trefethen_20"){
        return 6.308860e+01;
    }else if (testMatrix == "bcsstk01"){
        return 8.8234e+05;
    }else if (testMatrix == "bcsstk03"){
        return 6.791333e+06;
    }else if (testMatrix == "bcsstk04"){
        return 2.292466e+06;
    }else if (testMatrix == "bcsstk05"){
        return 1.428114e+04;
    }else if (testMatrix == "bcsstk22"){
        return 1.107165e+05;
    }else if (testMatrix == "lund_a"){
        return 2.796948e+06;
    }else if (testMatrix == "nos1"){
        return 1.991546e+07;
    }else if (testMatrix == "arc130"){
        return 6.0542e+10;
    }else if (testMatrix == "tumorAntiAngiogenesis_2"){
        return 1.9893e+10;    
    }else{
        return 11.6810;
    }
} // kappa    