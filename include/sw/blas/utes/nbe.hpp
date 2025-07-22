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

namespace sw {
    namespace blas {
		using namespace sw::numeric::containers;

        template<typename Scalar>
        Scalar nbe(const matrix<Scalar>& A,
            const vector<Scalar>& x,
            const vector<Scalar>& b) {

            return ((b - A * x).infnorm()) / (matnorm(A) * x.infnorm() + b.infnorm());
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
    }
}