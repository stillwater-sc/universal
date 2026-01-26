#pragma once
#include <numeric/containers/matrix.hpp>

// Size = 6 x 6 
// Rank 6; 
// Matrix 1-Norm = 36712 ; 
// Cond(est) = 10^16 
// symmetric = No
// pos.def = NO 
// Matrix ID = none
// NNZ = 36
// 

sw::numeric::containers::matrix<double>  rump6x6ill = {
    { 6566, -5202, -4040, -5524,  1420,  6229}, 
		{ 4104,  7449, -2518, -4588, -8841,  4040},
		{ 5266, -4008,  6803, -4702,  1240,  5060},
		{ -9306, 7213,  5723,  7961, -1981, -8834}, 
    { -3782, 3840,  2464, -8389,  9781, -3334},
    { -6903, 5610,  4306,  5548, -1380,  3539} 
};

// Let b = {-551,-354,9659,776,580,10720} for exact solution x = {1,1,1,1,1,1}