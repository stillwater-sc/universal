#pragma once

#include<algorithm>
#include<universal/blas/operators.hpp>
#include<universal/blas/matrix.hpp>
#include<universal/blas/vector.hpp>
#include<universal/blas/solvers.hpp>
#include<universal/blas/generators.hpp>
#include<universal/blas/blas_l1.hpp>

namespace sw::universal::blas {

template<typename Scalar, typename Tolerance = double>
void svd(const matrix<Scalar>& A, matrix<Scalar>& S, matrix<Scalar>& V, matrix<Scalar>& D, Tolerance tol = 10e-10) {
/*
    using value_type = Scalar;
    using size_type  = typename matrix<Scalar>::size_type;
    size_type        ncols = num_cols(A), nrows = num_rows(A);
    value_type       ref, zero = Scalar{ 0 }, one = Scalar{ 1 };
    
    // if (nrows != ncols) std::swap(row, col);

    matrix<Scalar> Q(nrows, nrows), R(nrows, ncols), VT(nrows, ncols), E(nrows, ncols),QT(ncols, ncols), RT(ncols, nrows);
    size_type l = 100 * std::max(nrows, ncols);
    S = one; D = one; 
    double err{ std::numeric_limits<double>::max() };
    for (size_type i = 0; err > tol && i < l; ++i) {
        double e, f;
        std::tie(QT, RT) = qr(V);
        S *= QT;
        VT = RT.transpose();
        std::tie(Q, R) = qr(VT);
        D *= Q;
        E = triu(R, 1);
        V = trans(R);
        f=norm(diag(R), 2);
        e=norm(E);
        if(f==zero) f=1;
        err=e/f;
    }
    {
	    V= 0;  
	    matrix<Scalar>  ins_V(V), ins_S(S);
	                
	    for (size_type i= 0, end= std::min(nrows, ncols); i < end; ++i) {
	        ins_V[i][i] *= std::abs(R[i][i]);
	        if (R[i][i] < zero) 	
		    for (size_type j= 0; j < nrows; ++j) 
		    ins_S[j][i] *= -1; 
	    }
    }
    */
}

template<typename Scalar, typename Tolerance>
std::tuple<matrix<Scalar>, matrix<Scalar>, matrix<Scalar>> svd(const matrix<Scalar>& A, Tolerance tol= 10e-10) {
    using size_type = typename matrix<Scalar>::size_type;
    size_type    ncols = num_cols(A), nrows = num_rows(A);
    //if (nrows != ncols) std::swap(row, col);
    matrix<Scalar> ST(ncols, ncols), V(A), D(nrows, nrows);
    svd(A, ST, V, D, tol);
    return std::make_tuple(ST, V, D);

}   

}  // namespace sw::universal::blas