#include <universal/blas/generators/randsvd.hpp>
using namespace sw::universal::blas;
int main()
{
  const matrix<double> A = {
    {12, -51, 4},
    {6, 167, -68},
    {-4, 24, -41},
    {-1, 1, 0},
    {2, 0, 3},
  };
  matrix<double> S, V, D;
  std::tie(S, V, D) = randsvd(A);
  std::cout << A << '\n' << S << '\n' << V << '\n' << D << '\n';
  return 0;
}
