// condest.hpp: Estimated Condition number of matrix 
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
// @jquinlan
//
// This file is part of the universal numbers project, 
// released under an MIT Open Source license.

// Reference 
/**
Cline, A. K., Moler, C. B., Stewart, G. W., & Wilkinson, J. H. (1979). 
An estimate for the condition number of a matrix. SIAM Journal on 
Numerical Analysis, 16(2), 368-375.
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

    Scalar Na  = matnorm(A,2);    // || A ||
    Scalar Ni  = 1;               // || A^{-1} ||
    sw::universal::blas::vector<Scalar> b(num_cols(A),1);

    auto [P, L, U] = plu(A);
    auto x = solve((L*U).transpose(), b);
    auto z = forwsub(L,x);
    auto y = backsub(U,z);

    Ni = y.infnorm()/x.infnorm();

    return Ni*Na;

} // end function

/**
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