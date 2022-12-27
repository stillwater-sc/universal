/** **********************************************************************
 * Estimated Condition number of matrix 
 *
 * @author:     James Quinlan
 * @date:       2022-12-13
 * @copyright:  Copyright (c) 2022 Stillwater Supercomputing, Inc.
 * @license:    MIT Open Source license 
 * 
 * This file is part of the universal numbers project.
 * ***********************************************************************
 */
#pragma once
#include <universal/blas/matrix.hpp>
#include <universal/blas/vector.hpp>
#include <universal/blas/blas.hpp>
#include <universal/blas/solvers/plu.hpp>
#include <universal/blas/solvers/lu.hpp>
#include <universal/blas/solvers/backsub.hpp>
#include <universal/blas/solvers/forwsub.hpp>
#include <universal/blas/utes/matnorm.hpp>

template<typename Scalar>
Scalar condest(const sw::universal::blas::matrix<Scalar> & A){
/**
 * After changing from [P,L,U] = plu(A) to inplace version of plu,
 * condest has stopped working.  Need to fix, however, as long as
 * showCondest = false; LUIR.cpp will run.   
 */


    Scalar Na  = matnorm(A,2);    // || A ||
    Scalar Ni  = 1;               // || A^{-1} ||
    size_t n = num_cols(A);
    sw::universal::blas::matrix<Scalar> LU(A);
    sw::universal::blas::vector<Scalar> b(n,1);
    sw::universal::blas::matrix<size_t> P(n-1,2);

    plu(LU,P);
    /*  
    for (size_t ii = 0; ii < n; ++ii){
        if(P(ii,0) != P(ii,1)){
            for (size_t jj = 0; jj < n; ++jj){
                auto aij = LU(P(ii,0),jj);
                LU(P(ii,0),jj) = LU(P(ii,1),jj);
                LU(P(ii,1),jj) = aij;
            }
        }
    }
    */
    
    // auto x = solve((LU).transpose(), b);  // x = (LU')^(-1)*b
    auto x = backsub(LU.transpose(),forwsub(LU.transpose(),b));
    auto z = forwsub(LU.transpose(),x);
    auto y = backsub(LU.transpose(),z);

    Ni = y.infnorm()/x.infnorm();

    return Ni*Na;

} // end function

// Reference 
/**
Cline, A. K., Moler, C. B., Stewart, G. W., & Wilkinson, J. H. (1979). 
An estimate for the condition number of a matrix. SIAM Journal on 
Numerical Analysis, 16(2), 368-375.

@article{cline1979estimate,
  title={An estimate for the condition number of a matrix},
  author={Cline, Alan K and Moler, Cleve B and Stewart, George W and Wilkinson, James H},
  journal={SIAM Journal on Numerical Analysis},
  volume={16},
  number={2},
  pages={368--375},
  year={1979},
  publisher={SIAM}
}
*/