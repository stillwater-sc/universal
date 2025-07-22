#pragma once
// luir.hpp: dense matrix iterative refinement LU decomposition and backsubstitution to solve systems of equations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <universal/blas/blas.hpp>
// overload the triangular solvers for posits with the fused dot product
#include <universal/blas/ext/solvers/posit_fused_backsub.hpp>
#include <universal/blas/ext/solvers/posit_fused_forwsub.hpp>
#include <universal/blas/utes/nbe.hpp>      // Normwise Backward Error

namespace sw { namespace universal { namespace blas {

/// <summary>
/// Solver Ax = b using Iterative Refinement with low precision LU factorization
/// </summary>
/// <typeparam name="HighPrecision"></typeparam>
/// <typeparam name="WorkingPrecision"></typeparam>
/// <typeparam name="LowPrecision"></typeparam>
/// <param name="Ah">matrix values in high precision</param>
/// <param name="Aw">matrix values in working precision</param>
/// <param name="Al">matrix values in low precision</param>
/// <returns>a pair consisting of the number of iterations of the IR loop and the final error norm of the solution</returns>
template<typename HighPrecision, typename WorkingPrecision, typename LowPrecision>
std::pair<int, double> SolveIRLU(matrix<HighPrecision>& Ah, matrix<WorkingPrecision>& Aw, matrix<LowPrecision>& Al, int maxIterations = 10, bool reportResultVector = false) 
{
    //if (reportResultVector) ReportExperimentConfiguration<HighPrecision, WorkingPrecision, LowPrecision>();

    /**
     * Matrix and Vector Type alias
     * - Mh: High Precision Matrix
     * - Vh: High Precision Vector
     * - Mw: Working Precision Matrix
     * - Vw: Working Precision Vector
     * - Ml: Low Precision Matrix
    */
    //using Mh = sw::universal::blas::matrix<HighPrecision>;
    using Vh = sw::universal::blas::vector<HighPrecision>;
    using Mw = sw::universal::blas::matrix<WorkingPrecision>;
    using Vw = sw::universal::blas::vector<WorkingPrecision>;
    //using Ml = sw::universal::blas::matrix<LowPrecision>;

    unsigned n = static_cast<unsigned>(num_cols(Aw));

    /** ********************************************************************
     *  LU Factorization of Low Precision Matrix (key step)
     *  : A is factored into LU using low precision.
     *  : LU is then stored in working precision (note permuations included)
     *  : A = P*A is computed & stored in high precision for residual calc.
     * *********************************************************************
    */
    vector<size_t> P(n);
    plu(Al, P);
    Mw LU(Al);
	permute(P, Aw);
    Ah = Aw; // update Ah with permuted Aw
    
    // Initializations
    Vh xh(n, 1);    // generate a known solution
    Vh b = Ah * xh; // mu*R*b
    Vw xw(xh);      // y = Sx
    Vw bw(b);       // Note: also try b = P*mu*R*(AX), where A is original matrix
    
    /** Iterative Refinement Steps
      1. Factor A = LU in low precision(see above)
      2. Solve x = (LU)^ { -1 } b
      3. While not coverged
          a).r = b - Ax(high precision calculation)
          b).Solve Ac = r(c = corrector)
          c).Update solution : x = x + c
      4. Goto 3
    */
    auto xn = backsub(LU, forwsub(LU, bw));
    if (normL1(xn).isinf()) {
		std::cerr << "Initial guess is not a valid solution as it contains infinites\n";
		return std::make_pair<int, double>(-1, INFINITY);
	}
    Vh r(n);
    int iteration = 0;
    bool stop = false, diverge = false;
    WorkingPrecision errnorm;
    //WorkingPrecision u_W = std::numeric_limits<WorkingPrecision>::epsilon();
    while (!stop) { 
        ++iteration;
        // std::cout << niters << " : " << xn << '\n';
        xh = xn;
        r = b - Ah * xh;
        Vw rn(r);
        auto c = backsub(LU, forwsub(LU, rn));
        if (normL1(c).isinf() || normL1(c).isnan()) {
            if (normL1(c).isnan()) std::cerr << "correction vector contains NaNs\n";
            if (normL1(c).isinf()) std::cerr << "correction vector contains infinites\n";
            return std::make_pair<int, double>(-1, double(normL1(c)));
        }
        xn += c;
        errnorm = (xw - xn).infnorm(); // nbe(A,xn,bw); 
        // if ((nbe(Aw, xn, bw) < u_W) || (errnorm < u_W) || (iteration >= maxIterations) || diverge) {  //
        // u_W is dependent on the working precision configuration and thus makes it difficult to
        // compare iterates between different precisions. We replace this with a constant error of 1.0e-6
        // to resolve that. 
        if ((nbe(Aw, xn, bw) < 1.0e-6) || (errnorm < 1.0e-6) || (iteration >= maxIterations) || diverge) {  // 
            // Stop Criteria
            // (nbe(A,xn,bw) < n*u_W)
            // (maxnorm < 1e-7)
            // (maxnorm/x.infnorm() < n*u_W)
            // forward error maxnorm/x.infnorm()
            stop = true;
        }

        // Print Results
        //std::cout << std::setw(4) << niters << std::setw(COLWIDTH) << maxnorm << std::setw(COLWIDTH) << nbe(Aw, xn, bw) << '\n';
        if ((errnorm > 1e+2)) { diverge = true; stop = true; iteration = 0; }
    }

    if (reportResultVector) std::cout << xn << " in " << iteration << " iterations, final error = " << errnorm << '\n';
    if (diverge) std::cerr << "Iterative refinement diverged\n";

    return std::make_pair(iteration, double(errnorm));
}


} } }  // namespace sw::universal::blas
