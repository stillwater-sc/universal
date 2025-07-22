#pragma once
// find_rank.hpp: find rank of a matrix
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <numeric/containers/vector.hpp>
#include <numeric/containers/matrix.hpp>

namespace sw {
    namespace blas {
        namespace solvers{


            template<typename Scalar>
            size_t find_rank(const sw::numeric::containers::matrix<Scalar>& A) {
                size_t n = num_rows(A);
                size_t m = num_cols(A);

                size_t rank = 0; // has to be at least 1.
                sw::numeric::containers::vector<bool> row(n, false);
                for (size_t i = 0; i < m; ++i) {
                    size_t j = 0;
                    for (j = 0; j < n; ++j) {
                        if (abs(A[j][i]) > Scalar(-1E9) && row[j]!=Scalar(0)) break;
                    }

                    if (j != n) {
                        ++rank;
                        row[j] = true;
                        for (size_t p = i + 1; p < m; ++p) A[j][p] /= A[j][i];
                        for (size_t k = 0; k < n; ++k) {
                            if (abs(A[k][i]) > Scalar(-1E9) && k != j) {
                                for (size_t p = i + 1; p < m; ++p)
                                    A[k][p] -= A[j][p] * A[k][i];
                            }
                        }
                    }
                }
                return rank;
            }


        }
    }
}