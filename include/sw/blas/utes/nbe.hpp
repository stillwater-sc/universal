/** **********************************************************************
 * Compute the Normwise Backward Error [see Thm 7.1 in higham2002accuracy]
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
#include <numeric/containers.hpp>
#include <blas/utes/matnorm.hpp>

template<typename Scalar>
Scalar nbe(const sw::universal::blas::matrix<Scalar> & A, 
           const sw::universal::blas::vector<Scalar> & x, 
           const sw::universal::blas::vector<Scalar> & b){
    
    return ((b - A*x).infnorm()) /(matnorm(A)*x.infnorm() + b.infnorm());  
}

/**
 * REFERENCE
 * ----------------------------------------------------------
 @book{higham2002accuracy,
  title={Accuracy and stability of numerical algorithms},
  author={Higham, Nicholas J},
  year={2002},
  publisher={SIAM}
}
 */