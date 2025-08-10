/** **********************************************************************
 * forwsub.hpp: Forward substitution to solve Ax = b 
 *              Input: Matrix A, Vector b, bool lower
 *              Inplace forward sub. Uses only lower tri.
 *
 * @author:     James Quinlan
 * @date:       2022-12-17
 * @modified:   2023-10-10
 * @copyright:  Copyright (c) 2022 Stillwater Supercomputing, Inc.
 * @license:    MIT Open Source license 
 * ***********************************************************************
 */
#pragma once
#include <numeric/containers.hpp>

namespace sw {
    namespace blas {
        namespace solvers {

            template<typename Matrix, typename Vector>
            Vector forwsub(const Matrix& A, const Vector& b, bool lower = false) {
                using Scalar = typename Matrix::value_type;
                size_t n = size(b);
                Vector x(n);
                Vector d(n, 1);

                if (lower) { d = diag(A); }

                x(0) = b(0) / d(0);
                for (size_t i = 1; i < n; ++i) {
                    Scalar y = 0.0;
                    for (size_t j = 0; j < i; ++j) {
                        y += A(i, j) * x(j);
                    }
                    x(i) = (lower) ? (b(i) - y) / d(i) : (b(i) - y);
                }
                return x;
            }


        }
    }
}
