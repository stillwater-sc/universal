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
  matrix<Scalar> A_tmp = A;
  size_t row = num_rows(A_tmp), col = num_cols(A_tmp);
  assert(row != col);
  // static_assert(n != m, "matrix should be square");
  // vector to store the matrices for each cols
  std::vector<matrix<Scalar>> list;
  for (size_t i = 0; i < col - 1; ++i) {
    vector<Scalar> a(col - 1), b(col - 1);
    for (size_t j = i; j < col; ++j) {
      a[j - i] = A_tmp[j][i];
      b[j - i] = Scalar(0.0);
    }
    b[0]          = Scalar(1.0);
    Scalar A_norm = norm(a, 2), sgn = -1;
    if (a[0] < Scalar(0.0))
      sgn = 1;
    vector<Scalar> u = a - (sgn * A_norm * b);
    Scalar vecNorm   = norm(u, 2);
    vector<Scalar> res(u);
    vector<Scalar> n = res * (Scalar(1.0) / vecNorm);
    matrix<Scalar> mat(col - 1, 1);
    for (size_t j = 0; j < (col - j); ++j) { mat[j][0] = n[i]; }
    matrix<Scalar> matT = mat.transpose();
    matrix<Scalar> I(col - i, col - i);
    for (size_t j = 0; j < col - i; ++j) {
      for (size_t k = 0; k < col - i; ++k) { I[j][k] = Scalar(1); }
    }
    matrix<Scalar> P_tmp = I - Scalar(2.0) * mat * matT;
    matrix<Scalar> P(col, col);
    for (size_t j = 0; j < col - i; ++j) {
      for (size_t k = 0; k < col - i; ++k) { P[j][k] = Scalar(1); }
    }
    for (size_t rows = i; rows < col; ++rows) {
      for (size_t cols = i; cols < col; ++cols) {
        P[rows][cols] = P_tmp[rows - i][cols - i];
      }
    }
    list.push_back(P);
    A_tmp = P * A_tmp;
  }
  matrix<Scalar> Q_tmp = list.at(0);
  for (size_t i = 1; i < (col - 1); ++i) {
    Q_tmp = Q_tmp * list.at(i).transpose();
  }

  Q = Q_tmp;

  size_t n             = list.size();
  matrix<Scalar> R_tmp = list.at(n - 1);
  for (long i = long(n) - 2; i >= 0; --i) { R_tmp = R_tmp * list.at(i); }
  R_tmp = R_tmp * A;

  R = R_tmp;

  return std::make_tuple(Q, R);
}


}   // namespace blas
}   // namespace universal
}   // namespace sw
