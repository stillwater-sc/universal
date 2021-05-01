#include <universal/blas/solvers/qr.hpp>
#include <universal/blas/matrix.hpp>
using namespace sw::universal::blas;
int main()
{
  matrix<double> A =
                   {
                     {12, -51, 4},
                     {6, 167, -68},
                     {-4, 24, -41},
                     {-1, 1, 0},
                     {2, 0, 3},
                   },
                 Q, R;

  std::tie(Q, R) = sw::universal::blas::solvers::qr(A, Q, R);
  std::cout << A << '\n' << Q << '\n' << R << '\n';
  return 0;
}