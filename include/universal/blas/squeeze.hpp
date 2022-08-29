// Squeeze.hpp: Squeeze elements of a matrix
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
// Authors: James Quinlan
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#pragma once
#include <universal/blas/blas.hpp>


#define ALGO 2

// Matrix Squeeze
template<typename Scalar>
void Squeeze(const sw::universal::blas::matrix<double>& A,
    sw::universal::blas::matrix<Scalar>& B,
    const Scalar &maxneg,
    const Scalar &maxpos) {

    size_t m = num_rows(A);
    double dmaxpos = double(maxpos);
    double dmaxneg = double(maxneg);

#if ALGO == 1 // Algo 2.1

    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < m; ++j) {
            double element = A(i, j);
            if (element < 0) {

                if (element < dmaxneg) {
                    B(i, j) = maxneg;
                }
                else {
                    B(i, j) = element;
                }

            }
            else { // positive
                if (element > dmaxpos) {
                    B(i, j) = maxpos;
                }
                else {
                    B(i, j) = element;
                }
            }
            if (A(i, j) != 0 && B(i, j) == 0) {
                // std::cout << "A(i,j) = " << A(i,j) << "      B(i,j) = " << B(i,j) << '\n';
            }
        }
    }
 
#elif ALGO == 2  // Algo. 2.2
    // Maximum Element of A
    Scalar a_max = 0;
    Scalar mu = 1.0;
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < m; ++j) {
            Scalar element = A(i, j);
            if (a_max <= abs(element)) {  // A(i,j) > maxpos = semantic bug
                a_max = abs(element);
            }
        }
    }

    if (maxpos < a_max) {
        mu = maxpos / a_max;
    }
    //else {
    //    Scalar mu = 1.0;
    //} // end if

    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < m; ++j) {
            Scalar element = A(i, j);
            B(i, j) = mu * element;
        }
    } // end for


#elif ALGO == 3
    sw::universal::blas::matrix<double> R(m, m);
    sw::universal::blas::matrix<double> S(m, m);
    sw::universal::blas::matrix<double> RA(m, m);
    sw::universal::blas::matrix<double> RAS(m, m);

    double MAX = 0;
    //for (size_t i = 0; i < m; ++i) {
      //  R(i, i) = maxrow(A, i, m);
    //} //end for

    // MAX ROW
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < m; ++j) {
            double element = A(i, j);
            if (MAX <= abs(element)) {
                MAX = abs(element);
            }
        }
        R(i, i) = MAX;
    }

    RA = R * A;


    // MAX COLUMN
    for (size_t j = 0; j < m; ++j) {
        for (size_t i = 0; i < m; ++i) {
            double element = A(i, j);
            if (MAX <= abs(element)) {
                MAX = abs(element);
            }
        }
        S(j, j) = MAX;
    }



    //for (size_t j = 0; j < m; ++j) {
    //    S(j, j) = maxcol(RA, j, m);
    //} // end for


    RAS = RA * S;

    // Maximum Element of A
    Scalar beta = 0;
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < m; ++j) {
            Scalar element = RAS(i, j);
            if (beta <= abs(element)) {
                beta = abs(element);
            }
        }
    } // end for = max element of RAS

    double mu = 1.0;
    if (dmaxpos < beta) {
        mu = double(dmaxpos / beta);
    }



    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < m; ++j) {
            double element = A(i, j);
            B(i,j) = mu * element;
        }
    }


 

#endif

} // end function


// Vector Squeeze
template<typename Scalar>
void Squeeze(const sw::universal::blas::vector<double> & x, sw::universal::blas::vector<Scalar> & y, 
const Scalar & maxneg, const Scalar & maxpos){

    double dmaxpos = double(maxpos);
    double dmaxneg = double(maxneg);

    size_t n = x.size();
    for(size_t i = 0; i < n; ++i){
        double element = x(i);  
        if (element > dmaxpos){y(i) = maxpos;}else if(element < dmaxneg){y(i) = maxneg;}else{y(i) = element;}
    }
}



// Vector Expand
template<typename Scalar>
void Expand(const sw::universal::blas::vector<Scalar> & x, sw::universal::blas::vector<double> & y){
    // Opposite of Squeeze: put back to double for comparisons (e.g., posit to double)
    size_t n = x.size();
    for(size_t i = 0; i < n; ++i){
        y(i) = double(x(i));   
    }
}



// Max Row
template<typename Scalar>
Scalar maxrow(const sw::universal::blas::matrix<double> & A, size_t & i, size_t &n){
    // i = row number
    // size_t n = num_cols(A);
    Scalar M = 0;
    for (size_t j=0; j < n; ++j){
            double element = A(i,j);
			if (M <= abs(element)){   
				M = abs(element);
			}
	}
    return M;
}



// Max Col
template<typename Scalar>
Scalar maxcol(const sw::universal::blas::matrix<double> & A, size_t & j, size_t &m){
    // i = col. number
    // size_t m = num_rows(A);
    Scalar M = 0;
    for (size_t i=0; i < m; ++i){
            double element = A(i,j);
			if (M <= abs(element)){   
				M = abs(element);
			}
	}
    return M;
}




// norm(A, inf)
/*
template<typename Scalar>
Scalar Linf(sw::universal::blas::vector<Scalar> &x){
    size_t n = x.size();
    Scalar M = 0;
    for (size_t i=0; i < n; ++i){
			if (M <= abs(x(i))){   
				M = abs(x(i));
			}
	}
    return M;
}
*/