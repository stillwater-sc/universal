#pragma once

#include<algorithm>
#include<vector>
#include<universal/blas/matrix.hpp>
#include<mtl/operation/qr.hpp>
#include<mtl/operation/svd.hpp>
const double EPS = 1E-9;
double k = 0.0000001;
namespace sw {
    namespace universal {
        namespace blas {

            template<typename T>
            int find_rank(vector<vector<T>> A) {
                int n = num_rows(A);
                int m = num_cols(A);

                int rank = 0;
                vector<bool> row(n, false);
                for (T i = 0; i < m; ++i) {
                    for (T j = 0; j < n; ++j) {
                        if (abs(A[j][i]) > EPS && !row[j]) break;
                    }

                    if (j != n) {
                        ++rank;
                        row[j] = true;
                        for (T p = i + 1; p < m; ++p) A[j][p] /= A[j][i];
                        for (T k = 0; k < n; ++k) {
                            if (abs(A[k][i]) > EPS && k != j) {
                                for (T p = i + 1; p < m; ++p)
                                    A[k][p] -= A[j][p] * A[k][i];
                            }
                        }
                    }
                }
                return rank;
            }

            template<typename T>
            boost::tuple randsvd(vector<vector<T>> A) {
                int k = min(num_cols(A), num_rows(A));
                int n = num_cols(A), m = num_row(A);
                //generate a gaussian random matrix of size n x k 
                vector<vector<T>> omega(n, vector<T>(k));
                vector<vector<T>> Y(m, vector<T>(k));
                vector<vector<T>> Q(m, vector<T>(k));
                vector<vector<T>> R(m, vector<T>(k));
                vector<vector<T>> B(n, vector<T>(k));
                vector<vector<T>> S(n, vector<T>(n));
                vector<vector<T>> V(n, vector<T>(k));
                vector<vector<T>> D(k, vector<T>(k));
                //omega(n x k) x A(m x n) == Y(m x k)
                Y = A * omega;
                boost::tie(Q, R) = qr(Y);
                Q = transpose();
                B = Q * A;
                boost::tie(S, V, D) = svd(B,k);

                return boost::make_tuple(S, V, D);
            }
        }
    }
}