#pragma once
#include <universal/blas/blas.hpp>

// Size = 4 x 4 
// Rank 4; Matrix Norm = 2.28; Cond. =  27.81
// symmetric = NO
// pos.def = NO 
// Matrix ID = na
// NNZ = 16

sw::universal::blas::matrix<double> rand4 = {
    { 0.826415369653239,   0.031110084193812,   0.248544297835642,   0.235211994298505},
    { 0.788229822846903,   0.951601007043557,   0.973568982776650,   0.748376707115318},
    { 0.485783122158754,   0.721556248348824,   0.442137211841357,   0.017378387081418},
    { 0.757266788300041,   0.406235059668287,   0.517811744943243,   0.156216057520737}
};

// Let 
/* 
b = 
    1.341281745981198
    3.461776519782428
    1.666854969430352
    1.837529650432309
*/
//} for exact solution x = {1,1,1,1}.

