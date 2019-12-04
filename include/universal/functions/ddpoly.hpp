#pragma once
// ddpoly.hpp: Evaluate a polynomial of degree N at point x as well as its ND derivatives
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
namespace sw {
namespace function {

#if defined(__clang__)
	/* Clang/LLVM. ---------------------------------------------- */

#elif defined(__ICC) || defined(__INTEL_COMPILER)
	/* Intel ICC/ICPC. ------------------------------------------ */

#elif defined(__GNUC__) || defined(__GNUG__)
	/* GNU GCC/G++. --------------------------------------------- */
	template<typename Scalar>
	inline size_t size(const std::vector<Scalar>& v) {
		return v.size();
	}

#elif defined(__HP_cc) || defined(__HP_aCC)
	/* Hewlett-Packard C/aC++. ---------------------------------- */

#elif defined(__IBMC__) || defined(__IBMCPP__)
	/* IBM XL C/C++. -------------------------------------------- */

#elif defined(_MSC_VER)
	/* Microsoft Visual Studio. --------------------------------- */

#elif defined(__PGI)
	/* Portland Group PGCC/PGCPP. ------------------------------- */

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
	/* Oracle Solaris Studio. ----------------------------------- */

#endif


// ddpoly evaluate a polynomial of degree N at point x as well as its ND derivatives
template<typename Vector, typename Scalar>
void ddpoly(const Scalar& x, const Vector& c, Vector& pd) {
	int N  = int(size(c))-1;  // c0 + c1*x + c2*x^2, etc., so we have N+1 coefficients for a polynomial of degree N
	int ND = int(size(pd)) - 1;   // pd[0] is the value of the polynomial at x, and pd[1..ND] are the derivatives at x

	for (int i = 0; i < int(size(pd)); ++i) pd[i] = 0;
	for (int i = N; i >= 0; --i) {
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

