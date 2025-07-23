#pragma once
// **********************************************************************
// plu.hpp: dense matrix PLU decomposition (PA = LU)
//          via DooLittle Method (in place)
//
// @author:     James Quinlan
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
// 
// This file is part of the Universal Number Library project.
#include <numeric/containers.hpp>

namespace sw {
    namespace blas {
        namespace solvers {
            using namespace sw::numeric::containers;

            /// <summary>
            ///  dense matrix LU with partial pivoting (PA = LU) decomposition via DooLittle Method (in place)
            /// </summary>
            /// <typeparam name="Scalar"></typeparam>
            /// <param name="A">dense matrix to factor</param>
            /// <param name="P">associated permutation matrix</param>
            template<typename Scalar>
            void plu(matrix<Scalar>& A, vector<size_t>& P) {
                Scalar x;
                size_t n = num_rows(A);
                for (size_t i = 0; i < n; ++i) { // i-th row
                    P(i) = i;

                    Scalar absmax = abs(A(i, i));
                    size_t argmax = i;

                    // Select k >= i to maximize |U(k,i)| 
                    for (size_t k = i + 1; k < n; ++k) {
                        if (abs(A(k, i)) > absmax) {
                            absmax = abs(A(k, i));
                            argmax = k;
                        }
                    }

                    // Check for necessary swaps
                    if (argmax != i) {
                        P(i) = argmax;
                        for (size_t j = 0; j < n; ++j) {  // j = i originally
                            x = A(i, j);
                            A(i, j) = A(argmax, j);
                            A(argmax, j) = x;
                        }
                    }

                    // Continue with row reduction
                    for (size_t k = i + 1; k < n; ++k) {  // objective row
                        A(k, i) = A(k, i) / A(i, i);
                        for (size_t j = i + 1; j < n; ++j) {
                            A(k, j) = A(k, j) - A(k, i) * A(i, j);
                        }
                    } // update L
                }
            }


            /// <summary>
            /// Given a permutation vector P, permute the rows of A
            /// </summary>
            /// <typeparam name="Scalar"></typeparam>
            /// <param name="P">permutation vector</param>
            /// <param name="A">matrix to permute</param>
            template<typename Scalar>
            void permute(vector<size_t>& P, matrix<Scalar>& A) {
                unsigned n = static_cast<unsigned>(num_cols(A));
                for (unsigned i = 0; i < n; ++i) {
                    if (i != P(i)) {
                        for (unsigned j = 0; j < n; ++j) {
                            std::swap(A(i, j), A(P(i), j));
                        }
                    }
                }
            }

        }
    }
} // namespace sw::blas::solvers
