#pragma once
// ddpoly.hpp: Evaluate a polynomial of degree N at point x as well as its ND derivatives
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
namespace sw { namespace universal {

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

/*
	pd[0] = c0 + c1*x + c2*x^2 + c3*x^3
	pd[1] = c1 + 2*c2*x + 3*c3*x^2
	pd[2] = 2*c2 + 3*2*c3*x
	pd[3] = 3*2*c3
	
	p = c0 + (c1 + c2*x + c3*x^2)*x
	  = c0 + (c1 + (c2 + c3*x)*x)*x
	
	p' = c1 + 2*c2*x + 3*c3*x^2
	   = c1 + (2*c2 + 3*c3*x)*x
	
	p'' = 2*c2 + 3*2*c3*x
	
	p''' = 3*2*c3
*/
// ddpoly evaluate a polynomial of degree N at point x as well as its ND derivatives
template<typename Vector, typename Scalar>
void ddpoly(const Scalar& x, const Vector& c, Vector& pd) {
	int N  = int(c.size())-1;  // c0 + c1*x + c2*x^2, etc., so we have N+1 coefficients for a polynomial of degree N
	int ND = int(pd.size())-1; // pd[0] is the value of the polynomial at x, and pd[1..ND] are the derivatives at x

	for (auto&& v : pd) v = Scalar(0);
	pd[0] = c[N];
	for (int i = N-1; i >= 0; --i) {
		int nnd = (ND < (N - i) ? ND : N - i);
		for (int j = nnd; j >= 1; --j) {
			pd[j] = pd[j] * x + pd[j - 1];
			//std::cout << "pd[" << j << "] = " << pd[j] << std::endl;
		}
		pd[0] = pd[0] * x + c[i];
	}
	// after the first derivative, factorial constants come in
	Scalar cnst(1);
	for (int i = 2; i <= ND; ++i) {
		cnst *= i;
		pd[i] *= cnst;
		//std::cout << "pd[" << i << "] = " << pd[i] << std::endl;
	}
}

}} // namespace sw::universal

