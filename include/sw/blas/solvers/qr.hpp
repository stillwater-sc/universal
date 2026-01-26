/** **********************************************************************
 * QR decompositions
 *  - 
 * 
 * @author:     James Quinlan
 * @date:       2022-12-28
 * @copyright:  Copyright (c) 2022 Stillwater Supercomputing, Inc.
 * SPDX-License-Identifier: MIT
 * 
 * This file is part of the universal numbers project.
 * ***********************************************************************
 */
#pragma once
#include<blas/blas.hpp>

namespace sw {
    namespace blas {
        namespace solvers {
            using namespace sw::numeric::containers;

            template<typename Scalar>
            matrix<Scalar> submat(const matrix<Scalar>& A, size_t i, size_t m, size_t j, size_t n) {
                size_t a = m - i;
                size_t b = n - j;
                matrix<Scalar> B(a, b);
                for (size_t p = 0; p < a; ++p) {
                    for (size_t q = 0; q < b; ++q) {
                        B(p, q) = A(i + p, j + q);
                    }
                }
                return B;
            }


            template<typename Scalar>
            vector<Scalar> subvec(const vector<Scalar>& v, size_t j, size_t n) {
                size_t m = n - j + 1;
                vector<Scalar> x(m);
                for (size_t i = 0; i < m; ++i) {
                    x(i) = v(i + j - 1);
                }
                return x;
            }

            // Modified Gram-Schmidt Orthogonalization Process
            template<typename Scalar>
            void mgs(const matrix<Scalar>& A, matrix<Scalar>& Q, matrix<Scalar>& R) {
                size_t m = num_rows(A);
                size_t n = num_cols(A);
                vector<Scalar> a(m);

                for (size_t j = 0; j < m; ++j) {
                    a(j) = A(j, 0);
                }
                R(0, 0) = normL2(a);
                for (size_t k = 0; k < m; ++k) {
                    Q(k, 0) = a(k) / R(0, 0);
                }

                for (size_t j = 1; j < n; ++j) {
                    // Get jth column
                    for (size_t k = 0; k < m; ++k) {
                        a(k) = A(k, j);
                    }
                    // Compute projections
                    for (size_t i = 0; i < j; ++i) {

                        for (size_t k = 0; k < m; ++k) {
                            R(i, j) += Q(k, i) * a(k);
                        }

                        for (size_t k = 0; k < m; ++k) {
                            a(k) = a(k) - R(i, j) * Q(k, i);
                        }
                    }
                    R(j, j) = norm(a, 2);
                    for (size_t k = 0; k < m; ++k) {
                        Q(k, j) = a(k) / R(j, j);
                    }
                }
            }

            // Updates R matrix
            template<typename Scalar>
            bool householder_update_R(matrix<Scalar>& R, const vector<Scalar>& v, Scalar c, size_t j) {
                size_t m = num_rows(R);
                size_t n = num_cols(R);
                if (j > m - 1 || j > n - 1) { std::cerr << "Index out of bounds\n"; return false; }

                auto XXt = xyt(v, v);
                auto B = c * XXt * submat(R, j, m, j, n);
                for (size_t i = 0; i < num_rows(B); ++i) {
                    for (size_t k = 0; k < num_cols(B); ++k) {
                        R(i + j, k + j) = R(i + j, k + j) - B(i, k);
                    }
                }
                return true;
            }

            // Update Q matrix
            template<typename Scalar>
            bool householder_update_Q(matrix<Scalar>& Q, const vector<Scalar>& v, Scalar c, size_t j) {
                size_t m = num_rows(Q);
                size_t n = m;
                if (j > m - 1) { std::cerr << "Index out of bounds\n"; return false; }
                auto XXt = xyt(v, v);
                auto B = c * submat(Q, 0, m, j, n) * XXt;

                for (size_t i = 0; i < num_rows(B); ++i) {
                    for (size_t k = 0; k < num_cols(B); ++k) {
                        Q(i, k + j) = Q(i, k + j) - B(i, k);
                    }
                }
                return true;
            }

            template<typename Scalar>
            void houseqr(const matrix<Scalar>& A, matrix<Scalar>& Q, matrix<Scalar>& R) {
                size_t m = num_rows(A);
                size_t n = num_cols(A);
                Scalar c;
                vector<Scalar> v(m);
                for (size_t j = 0; j < n; ++j) {

                    // Extracting subvector from R
                    for (size_t k = j; k < m; ++k) {
                        v(k) = R(k, j);
                    }
                    auto w = subvec(v, j + 1, m);
                    int sgn = w(0) < 0 ? -1 : 1;
                    w(0) += sgn * normL2(w);
                    c = 2 / dot(w, w);
                    householder_update_R(R, w, c, j);
                    householder_update_Q(Q, w, c, j);
                }
            }


            // Householder method with column pivoting
            template<typename Scalar>
            void houseqrpivot(const matrix<Scalar>& A, matrix<Scalar>& Q, matrix<Scalar>& R, matrix<Scalar>& P) {
                // See https://netlib.org/lapack/lug/node42.html
                size_t m = num_rows(A);
                size_t n = num_cols(A);

                // Compute Norms of A
                vector<Scalar> x(n);
                vector<Scalar> r(n);
                Scalar c;

                for (size_t k = 0; k < n; ++k) {
                    x(k) = normL2(getCol(k, A));
                    x(k) *= x(k); // Norm squared
                }

                vector<Scalar> v(m);
                for (size_t j = 0; j < n; ++j) {

                    // Which is largest?
                    size_t argmax = j;
                    for (size_t k = j + 1; k < n; ++k) {
                        if (x(j) < x(k)) {
                            argmax = k;
                        }
                    }

                    // swap 
                    if (argmax != j) {
                        std::cout << "Switch, (j, argmax) = " << j << ", " << argmax << std::endl;
                        Scalar tmp;
                        for (size_t k = 0; k < m; ++k) {
                            tmp = R(k, j);
                            R(k, j) = R(k, argmax);
                            R(k, argmax) = tmp;
                        }

                        // Record swap
                        P(j, 1) = argmax;
                        P(argmax, 1) = j;

                        // swap ||a_i|| and ||a_k||
                        tmp = x(j);
                        x(j) = x(argmax);
                        x(argmax) = tmp;
                    }


                    // Extracting subvector from R
                    for (size_t k = j; k < m; ++k) {
                        v(k) = R(k, j);
                    }
                    auto w = subvec(v, j + 1, m);
                    int sgn = w(0) < 0 ? -1 : 1;
                    w(0) += sgn * normL2(w);
                    c = 2 / dot(w, w);
                    householder_update_R(R, w, c, j);
                    householder_update_Q(Q, w, c, j);

                    // Update norms
                    for (size_t k = j + 1; k < n; ++k) {
                        r(k) = R(0, k) * R(0, k);
                        x(k) = abs(x(k) - r(k));
                        // std::cout << "x(1,k) - r(1,k) = " << x(k) << std::endl;
                    }

                }
            }


            // Given rotation setup (entries of rotation matrix)
            template<typename Scalar>
            vector<Scalar> givens(const Scalar a, const Scalar b) {
                vector<Scalar> x(2);
                if (abs(a) >= abs(b)) {
                    Scalar t = b / a;
                    int sgn = a < 0 ? -1 : 1;
                    x(0) = sgn / sqrt(1 + t * t);
                    x(1) = x(0) * t;
                }
                else {
                    Scalar t = a / b;
                    int sgn = b < 0 ? -1 : 1;
                    x(1) = sgn / sqrt(1 + t * t);
                    x(0) = x(1) * t;
                }
                return x;
            }

            // Perform Given's QR method
            template<typename Scalar>
            void givensqr(const matrix<Scalar>& A, matrix<Scalar>& Q, matrix<Scalar>& R) {
                size_t m = num_rows(A);
                size_t n = num_cols(A);

                for (size_t j = 0; j < n; ++j) {
                    for (size_t i = m - 1; i > j; --i) {
                        matrix<Scalar> G(m, m);
                        G = 1;
                        vector<Scalar> x = givens(R(i - 1, j), R(i, j));
                        G(i, i) = x(0);
                        G(i - 1, i - 1) = x(0);
                        G(i, i - 1) = x(1);
                        G(i - 1, i) = -x(1);

                        Q = Q * G;
                        R = G.transpose() * R;
                    }
                }
            }

            // MAIN QR method (calls specific method within)
            template<typename Scalar>
            std::pair<matrix<Scalar>, matrix<Scalar>> qr(const matrix<Scalar>& A, size_t which = 1) {
                // R is the upper triangular matrix
                // Q is the orthogonal matrix
                matrix<Scalar> Q(num_rows(A), num_rows(A)), R(num_rows(A), num_cols(A));

                switch (which) {
                case 1:
                {
                    Q = 1;
                    R = A;
                    houseqr(A, Q, R);
                }
                break;
                case 2:
                    mgs(A, Q, R);
                    break;
                case 3:
                    // Given's
                    Q = 1;
                    R = A;
                    givensqr(A, Q, R);
                    for (size_t i = 0; i < num_rows(R); ++i) {
                        for (size_t j = 0; j < num_cols(R); ++j) {
                            R(i, j) = (std::abs(R(i, j)) < 1.0e-18 ? 0 : R(i, j));
                        }
                    }
                    break;
                case 4:
                {
                    Q = 1;
                    R = A;
                    matrix<Scalar> P(num_rows(A), 2);
                    for (size_t i = 0; i < num_rows(P); ++i) {
                        P(i, 0) = i;
                        P(i, 1) = i;
                    }
                    houseqrpivot(A, Q, R, P);
                }
                break;
                default:
                {
                    Q = 1;
                    R = A;
                    houseqr(A, Q, R);
                }
                break;
                }
                return std::make_pair(Q, R);
            }

        }
    }
} // namespace sw::blas::solvers

/*
EXAMPLES:

A = {
        {0.75126707, 0.75126707, 0.75126706, 0.75126706},
        {0.25509512, 0.25509512, 0.25509512, 0.25509512},
        {0.50595705, 0.50595706, 0.50595705, 0.50595706},
        {0.69907672, 0.69907673, 0.69907673, 0.69907673},
        {0.89090326, 0.89090326, 0.89090326, 0.89090326}
    };

A = {
        { 1,  -2 , -1 }, 
        { 2,   0,   1 }, 
        { 2,  -4,   2 }, 
        { 4,   0,   0 }
    };

A = {
        { 0.8147,   0.0975 , 0.1576 }, 
        { 0.9058,   0.2785,  0.9706 }, 
        { 0.1270,   0.5469,  0.9572 }, 
        { 0.9134,   0.9575,  0.4854 },
        { 0.6324,   0.9649,  0.8003 }
    };
*/
