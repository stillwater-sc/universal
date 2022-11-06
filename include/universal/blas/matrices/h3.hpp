#pragma once
#include <universal/blas/blas.hpp>

// Size = 3 x 3
// Rank 3; Matrix Norm = 1.4142e+11; Cond. =  1.8478e+11
// symmetric = NO
/// pos.def = NO 
// Matrix ID = Higham 2019 Squeeze Matrix
// NNZ = 8
sw::universal::blas::matrix<double>  h3 = {
{1.0, 1.0, 2.5e6},
{1.0, -1.0, 2.5e6},
{1.0, 1.0, 0.0}
};

// if Amax > 2e12, then 0 (underflow) ==> singular
// subnormals in fp16 if 1e9 < alpha = Amax < 1e12