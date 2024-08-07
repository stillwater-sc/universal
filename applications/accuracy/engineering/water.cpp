// water.cpp: example program showing water chemical equilibrium calculation sensitivity
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Author: Allan Leal       : https://geg.ethz.ch/allan-leal/
//         Theodore Omtzigt : port to Universal
//
// stdlib includes
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
// universal BLAS
#include <universal/number/posit/posit.hpp>
#include <universal/blas/blas.hpp>

using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::setw;

using Real = double;
using Vec = sw::universal::blas::vector<Real>;
using Mat = sw::universal::blas::matrix<Real>;

// The number of species.
const auto N = 5; // H2O, H+, OH-, O2, H2

// The number of elements.
const auto E = 3; // H, O, Z(electric charge)

// The names of the species in the chemical equilibrium problem of water
const auto species = vector<string>{ "H2O", "H+", "OH-", "O2", "H2" };

// The standard chemical potentials of the species (in J/mol)
const auto u0 = Vec{{ -237182.0, 0.0000, -157297.0, 16543.5, 17723.4 }};

// The universal gas constant (in J/(mol*K))
const auto R = 8.314;

// The temperature in the calculation (in K)
const auto T = 298.15;

// The pressure in the calculation (in Pa)
//const auto P = 1e5; // = 1bar

// The formula matrix of the species (Aij is the number of atoms of element j in species i)
const auto A = Mat{{
   // H2O H+  OH- O2  H2
    { 2,  1,  1,  0,  2 }, // H
    { 1,  0,  1,  2,  0 }, // O
    { 0,  1, -1,  0,  0 }, // Z (electric charge as element)
}};

// The vector with the amounts of each chemical element
const auto b = Vec{ 110.0, 55.0, 0.0 };           // <-- This converges!
// const auto b = Vec{ 111.0, 55.5, 0.0 };           // <-- This does not converge because of round-off errors!

auto computeF(const Vec& n, const Vec& y)
{ 
    // Compute the activities of the species
    Vec a = 55.508*n/n[0]; // a[solute] = molality[solute]
    a[0] = n[0]/n.sum();   // a[water]  = mole-fractio[water]
    
    // Compute the normalized chemical potentials of the species u/RT = u0/RT + ln(a)
    const Vec ubar = u0/(R*T) + a.array().log().matrix();
    
    Vec vec(N + E);
//    vec.head(N) = ubar + A.transpose()*y;
//    vec.tail(E) = A*n - b;
    
    return vec;
}

auto computeJ(const Vec& n, const Vec& y)
{ 
    // Compute the activities of the species
    Vec a = 55.508*n/n[0]; // a[solute] = molality[solute]
    a[0] = n[0]/n.sum();   // a[water]  = mole-fractio[water]
    
    // Compute the normalized chemical potentials of the species u/RT = u0/RT + ln(a)
    const Vec ubar = u0/(R*T) + a.array().log().matrix();
    
    // Compute the Hessian matrix (diagonal approx)
    Vec H(n);
    H[0] = 1.0/n[0] - 1.0/n.sum();
//    H.tail(N - 1) = 1.0/n.tail(N - 1).array();
    
	Mat mat(N + E, N + E);
//    Mat mat = Mat::Zero(N + E, N + E);
//    mat.topLeftCorner(N, E) = A.transpose();
//    mat.topRightCorner(N, N).diagonal() = H;
//    mat.bottomRightCorner(E, N) = A;
        
    return mat;
}

auto equilibrate(Vec& n, Vec& y)
{
    cout << std::left  << setw(15) << "i";
    cout << std::right << setw(15) << "n[H2O]";
    cout << std::right << setw(15) << "n[H+ ]";
    cout << std::right << setw(15) << "n[OH-]";
    cout << std::right << setw(15) << "n[O2 ]";
    cout << std::right << setw(15) << "n[H2 ]";
    cout << std::right << setw(15) << "y[H]";
    cout << std::right << setw(15) << "y[O]";
    cout << std::right << setw(15) << "y[Z]";
    cout << endl;
    
    const auto maxiters = 100;
    for(auto i = 0; i < maxiters; ++i)
    {
        auto F = computeF(n, y);
        
        if(F.norm() < 1e-10)
            break;
        
        auto J = computeJ(n, y);
        
//        lu.compute(J);
//               
//        Vec dydn = lu.solve(-F);
        
        Vec dydn = solve(J, -F);

        const auto dy = dydn.head(E);
        const auto dn = dydn.tail(N);
        
        n += dn;
        y += dy;
        
        // Ensure an lower bound is imposed for n, since it cannot be negative nor zero
        for(auto k = 0; k < N; ++k)
            n[k] = std::max(n[k], 1e-40);
        
        cout << std::left << setw(15) << i;
        cout << std::right << setw(15) << n[0];
        cout << std::right << setw(15) << n[1];
        cout << std::right << setw(15) << n[2];
        cout << std::right << setw(15) << n[3];
        cout << std::right << setw(15) << n[4];
        cout << std::right << setw(15) << y[0];
        cout << std::right << setw(15) << y[1];
        cout << std::right << setw(15) << y[2];
        cout << endl;
    }
}

int main(int argc, char const *argv[])
{
    // The initial guess for the amounts of H2O, H+, OH-, O2, H2
    auto n = Vec{ 55.0, 1e-6, 1e-6, 1e-20, 1e-20 }; 
    
    // The initial guess for Lagrange multipliers of H, O, Z
    auto y = Vec{ 0.0, 0.0, 0.0 }; 
    
    equilibrate(n, y);
    
    return 0;
}
