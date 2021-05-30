#pragma once
#include <universal/blas/matrix.hpp>
#include <universal/blas/vector.hpp>
#include <universal/blas/blas_l1.hpp>

namespace sw {
    namespace universal {
        namespace blas {
            template <typename Scalar>
            std::tuple<matrix<Scalar>, matrix<Scalar>> qr(const matrix<Scalar>& A,
                matrix<Scalar>& Q,
                matrix<Scalar>& R)
            {
                size_t m = num_cols(A), n = num_rows(A);
                assert(n == m);
                vector<vector<Scalar>>columnExtractor(Scalar(n), vector<int>(Scalar(m)));
                vector<vector<Scalar>>signal(Scalar(n), vector<int>(Scalar(m), Scalar(-1)));
                vector<Scalar> alpha(n), alpha_norm(n);
                for (size_t i = 1; i <= n; ++i) get_column(A, i, columnExtractor);//put ith column of A ith index of columnExtractor
                alpha[1] = columnExtractor[1] / Scalar(norm(columnExtractor[1],2));
                set_column(Q, 1, alpha[1]);
                for (size_t i = 2; i <= n; ++i) {
                    for (size_t j = 1; j <= i - 1; ++j) {
                        if (signal[i][j] == Scalar(-1)) signal[i][j] = calculateSignal(columnExtractor[i], alpha[j]);
                        alpha[i] = columnExtractor[i] - signal[i][j];//we don't have signal here
                        alpha_norm[i] = alpha[i] / Scalar(norm(alpha[i],2));
                        set_column(Q, i, alpha_norm[i]);
                    }
                }
                R = Q.transpose();
                return std::make_tuple(Q, R);
            }
            template<typename Scalar>
            vector<Scalar> calculateSignal(vector<Scalar>& columnExtractor, vector<Scalar>& alpha) {
                return columnExtractor * alpha / norm(columnExtractor) * alpha;
            }
            template <typename Scalar>
            std::tuple<matrix<Scalar>, matrix<Scalar>> qr(const matrix<Scalar>& A)
            {
                using size_type = typename matrix<Scalar>::size_type;
                size_type ncols = num_cols(A), nrows = num_rows(A);
                matrix<Scalar> Q, R;
                qr(A, Q, R);
                return std::make_tuple(Q, R);
            }

        }   // namespace blas
    }   // namespace universal
}   // namespace sw
