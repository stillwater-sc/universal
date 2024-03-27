/** **********************************************************************
 * backsub.hpp: Backsubstitution to solve Ax = b given A = upper triangular 
 *
 * @author:     James Quinlan
 * @date:       2022-12-17
 * @modified:   2023-10-10
 * @copyright:  Copyright (c) 2022 Stillwater Supercomputing, Inc.
 * @license:    MIT Open Source license 
 * ***********************************************************************
 */

#pragma once
#include <universal/blas/matrix.hpp>
#include <universal/blas/vector.hpp>

namespace sw { namespace universal { namespace blas {

template<typename Matrix, typename Vector>
Vector backsub(const Matrix& A, const Vector& b) {
	using Scalar = typename Matrix::value_type;
	unsigned n = static_cast<unsigned>(size(b));
    
    Vector x(n);
    for (unsigned e = 0; e < n; ++e) {
        unsigned i = n - 1u - e;
        Scalar y = 0.0;
        for (unsigned j = i; j < n; ++j){
            y += A(i,j)*x(j);
        }
        x(i) = (b(i) - y)/A(i,i);
    }
	return x;
}

}}} // namespace sw::universal::blas
