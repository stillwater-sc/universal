#pragma once
// ddpoly.hpp: Evaluate a polynomial of degree N at point x as well as its ND derivatives
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
namespace sw {
namespace function {

// ddpoly evaluate a polynomial of degree N at point x as well as its ND derivatives
template<typename Vector, typename Scalar>
void ddpoly(const Scalar& x, const Vector& c, Vector& pd) {
	int N  = int(size(c))-1;  // c0 + c1*x + c2*x^2, etc., so we have N+1 coefficients for a polynomial of degree N
	int ND = int(size(pd)) - 1;   // pd[0] is the value of the polynomial at x, and pd[1..ND] are the derivatives at x

	for (int i = 0; i < int(size(pd)); ++i) pd[i] = 0;
	for (int i = N-1; i >= 0; --i) {
		int nnd = (ND < (N - i) ? ND : N - i);
		for (int j = nnd; j >= i; --j) {
			pd[j] = pd[j] * x + pd[j - 1];
		}
		pd[0] = pd[0] * x + c[i];
	}
	// after the first derivative, factorial constants come in
	Scalar cnst(1);
	for (int i = 2; i < ND; ++i) {
		cnst *= i;
		pd[i] *= cnst;
	}
}

}  // namespace function
}  // namespace sw

