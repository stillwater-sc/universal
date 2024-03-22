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
	int n = static_cast<int>(size(b));
    Vector x(n);
    
	for (int i = n-1; i >=0; --i){
        Scalar y = 0.0;
        for (int j = i; j < n; ++j){
            y += A(i,j)*x(j);
        }
        x(i) = (b(i) - y)/A(i,i);
    }
	return x;
}

template<unsigned nbits, unsigned es, unsigned capacity = 10>
vector<posit<nbits,es>> backsub(const matrix<posit<nbits,es>> & A, const vector<posit<nbits,es>>& b) {
    using Scalar = posit<nbits, es>;
    using Vector = vector<Scalar>;
    using Quire  = quire<nbits,es,capacity>;
	int n = static_cast<int>(size(b));

    Vector x(n);
	for (int i = n-1; i >=0; --i) {
        Quire q{0};
        for (int j = i; j < n; ++j) {
            q += quire_mul(A(i,j), x(j));
        }
        Scalar y;
        convert(q.to_value(), y); 
        x(i) = (b(i) - y)/A(i,i);
    }
	return x;
}

}}} // namespace sw::universal::blas
