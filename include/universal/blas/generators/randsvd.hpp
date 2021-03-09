#pragma once

#include<algorithm>
#include<vector>
//#include<universal/blas/matrix.hpp>
#include<boost/numeric/mtl/operation/qr.hpp>
#include<boost/numeric/mtl/operation/svd.hpp>
#include<boost/numeric/mtl/operation/trans.hpp>
#include<boost/mpl/bool.hpp>
const double EPS = 1E-9;
double k = 0.0000001;
namespace sw {
    namespace universal {
        namespace blas {

            template<typename IntgerType>
            int find_rank(vector<vector<IntgerType>> A) {
                int n = num_rows(A);
                int m = num_cols(A);

                int rank = 0;
                vector<bool> row(n, false);
                for (IntgerType i = 0; i < m; ++i) {
                    for (IntgerType j = 0; j < n; ++j) {
                        if (abs(A[j][i]) > EPS && !row[j]) break;
                    }

                    if (j != n) {
                        ++rank;
                        row[j] = true;
                        for (IntgerType p = i + 1; p < m; ++p) A[j][p] /= A[j][i];
                        for (IntgerType k = 0; k < n; ++k) {
                            if (abs(A[k][i]) > EPS && k != j) {
                                for (IntgerType p = i + 1; p < m; ++p)
                                    A[k][p] -= A[j][p] * A[k][i];
                            }
                        }
                    }
                }
                return rank;
            }

            template<typename Scalar>
            tuple randsvd(vector<vector<Scalar>> A) {
                Scalar k = min(num_cols(A), num_rows(A))-1;
                Scalar n = num_cols(A), m = num_row(A);
                //generate a gaussian random matrix of size n x k which is omega
                vector<vector<Scalar>> omega;// (n, vector<Scalar>(k));
                vector<vector<Scalar>> Y;// (m, vector<Scalar>(k));
                vector<vector<Scalar>> Q;// (m, vector<Scalar>(k));
                vector<vector<Scalar>> R;// (m, vector<Scalar>(k));
                vector<vector<Scalar>> B;// (n, vector<Scalar>(k));
                vector<vector<Scalar>> S;// (n, vector<Scalar>(n));
                vector<vector<Scalar>> V;// (n, vector<Scalar>(k));
                vector<vector<Scalar>> D;//(k, vector<Scalar>(k));
                //omega(n x k) x A(m x n) == Y(m x k)
                Y = A * omega;
                std::tie(Q, R) = qr(Y);
                Q = trans(Q);
                B = Q * A;
                std::tie(S, V, D) = svd(B, k);

                return std::make_tuple(S, V, D);
            
            }
        }
    }
}