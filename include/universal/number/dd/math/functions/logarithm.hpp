#pragma once
// logarithm.hpp: logarithm functions for double-double (dd) floating-point
//
// base algorithm strategy courtesy Scibuilder, Jack Poulson
// 
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>

namespace sw { namespace universal {

	/// <summary>
	/// Natural logarithm (base = e)
	/// </summary>
	/// <param name="a">input</param>
	/// <returns>natural logarithm of a</returns>
	inline dd log(const dd& a) {
		if (a.isnan() || a.isinf()) return a;

		if (a.iszero()) return dd(SpecificValue::infneg);

		if (a.isone()) return 0.0;

		if (a.sign())	{
			std::cerr << "log: non-positive argument\n";
			errno = EDOM;
			return dd(SpecificValue::qnan);
		}

		/* Strategy.  The Taylor series for log converges much more
		   slowly than that of exp, due to the lack of the factorial
		   term in the denominator.  Hence this routine instead tries
		   to determine the root of the function

			   f(x) = exp(x) - a

		   using Newton iteration.  The iteration is given by

			   x' = x - f(x)/f'(x)
				  = x - (1 - a * exp(-x))
				  = x + a * exp(-x) - 1.

		   Only one iteration is needed, since Newton's iteration
		   approximately doubles the number of digits per iteration. 
	   */

		dd x = std::log(a.high());   // Initial approximation
		x = x + a * exp(-x) - 1.0;
		return x;
	}

	/// <summary>
	/// binary logarithm (base = 2)
	/// </summary>
	/// <param name="a">input</param>
	/// <returns>binary logarithm of a</returns>
	inline dd log2(const dd& a) {
		if (a.isnan() || a.isinf()) return a;

		if (a.iszero()) return dd(SpecificValue::infneg);

		if (a.isone()) return 0.0;

		if (a.sign()) {
			std::cerr << "log2: non-positive argument\n";
			errno = EDOM;
			return dd(SpecificValue::qnan);
		}

		return log(a) * dd_lge;
	}

	/// <summary>
	/// decimal logarithm (base = 10)
	/// </summary>
	/// <param name="a">input</param>
	/// <returns>binary logarithm of a</returns>
	inline dd log10(const dd& a) {
		if (a.isnan() || a.isinf()) return a;

		if (a.iszero()) return dd(SpecificValue::infneg);

		if (a.isone()) return 0.0;

		if (a.sign()) {
			std::cerr << "log10: non-positive argument\n";
			errno = EDOM;
			return dd(SpecificValue::qnan);
		}

		return log(a) / dd_log10;
	}

	/// <summary>
	/// Natural logarithm of 1+x
	/// </summary>
	/// <param name="a">input</param>
	/// <returns>binary logarithm of a</returns>
	inline dd log1p(const dd& a) {
		if (a.isnan() || a.isinf()) return a;

		if (a.iszero()) return dd(0.0);

		if (a == -1.0) return dd(SpecificValue::infneg);

		if (a < -1.0) {
			std::cerr << "log1p: non-positive argument\n";
			errno = EDOM;
			return dd(SpecificValue::qnan);
		}

		if ((a >= 2.0) || (a <= -0.5))			//	a >= 2.0 - no loss of significant bits - use log()
			return log(1.0 + a);

		// at this point, -1.0 < a < 2.0
		// return _log1p(a);
		return log(1.0 + a);   // TODO: evaluate loss of precision
	}

}} // namespace sw::universal
