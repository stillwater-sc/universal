#pragma once

#include <universal/blas/blas.hpp>
#include <universal/blas/generators/gaussian_random.hpp>

namespace sw {
namespace universal {
namespace blas {

template <typename Scalar>
std::tuple<matrix<Scalar>, matrix<Scalar>, matrix<Scalar>> randsvd(
  const matrix<Scalar>& A)
{
  size_t k = std::min(num_cols(A), num_rows(A));
  size_t n = num_cols(A), m = num_rows(A);
  matrix<Scalar> omega(n, k), Y(m, k), B(k, n);
  double mean   = 1.0;
  double stddev = 0.5;
  gaussian_random(omega, mean, stddev);
  Y = A * omega;
  matrix<Scalar> Q(n, k), R(n, n);
  std::tie(Q, R) = qr(Y);
  Q.transpose();
  B = Q * A;
  matrix<Scalar> S(n, k), V(n, n), D(n, n);
  std::tie(S, V, D) = svd(B, k);
  return std::make_tuple(S, V, D);
}
}   // namespace blas
}   // namespace universal
}   // namespace sw
